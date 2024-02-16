/*
 * Copyright (c) 2023 General Motors GTO LLC
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: 2023 General Motors GTO LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#include <uprotocol-cpp-ulink-zenoh/rpc/zenohRpcClient.h>
#include <uprotocol-cpp-ulink-zenoh/session/zenohSessionManager.h>
#include <uprotocol-cpp/uuid/serializer/UuidSerializer.h>
#include <uprotocol-cpp/uri/serializer/LongUriSerializer.h>
#include <uprotocol-cpp/transport/datamodel/UPayload.h>
#include <uprotocol-cpp/transport/datamodel/UAttributesBuilder.h>
#include <src/main/proto/ustatus.pb.h>
#include <spdlog/spdlog.h>
#include <zenoh.h>
#include <uuid/uuid.h>
#include <src/main/proto/umessage.pb.h>
#include <src/main/proto/uattributes.pb.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uuid;
using namespace uprotocol::uri;
using namespace uprotocol::v1;

ZenohRpcClient& ZenohRpcClient::instance(void) noexcept {

    static ZenohRpcClient rpcClient;

    return rpcClient;
}

UStatus ZenohRpcClient::init() noexcept {

    UStatus status;

    if (0 == refCount_) {

        std::lock_guard<std::mutex> lock(mutex_);

        if (0 == refCount_) {
            /* by default initialized to empty strings */
            ZenohSessionManagerConfig config;

            if (UCode::OK != ZenohSessionManager::instance().init(config)) {
                spdlog::error("zenohSessionManager::instance().init() failed");
                status.set_code(UCode::UNAVAILABLE);
                return status;
            }

            if (ZenohSessionManager::instance().getSession().has_value()) {
                session_ = ZenohSessionManager::instance().getSession().value();
            } else {
                status.set_code(UCode::UNAVAILABLE);
                return status;
            }

            threadPool_ = make_shared<ThreadPool>(threadPoolSize_);
            if (nullptr == threadPool_) {
                spdlog::error("failed to create thread pool");
                status.set_code(UCode::UNAVAILABLE);
                return status;
            }

            threadPool_->init();

        }
        refCount_.fetch_add(1);

    } else {
        refCount_.fetch_add(1);
    }

    status.set_code(UCode::OK);

    return status;
}

UStatus ZenohRpcClient::term() noexcept {

    UStatus status;
    
    std::lock_guard<std::mutex> lock(mutex_);

    refCount_.fetch_sub(1);

    if (0 == refCount_) {

        threadPool_->term();

        if (UCode::OK != ZenohSessionManager::instance().term()) {
            spdlog::error("zenohSessionManager::instance().term() failed");
            status.set_code(UCode::UNAVAILABLE);
            return status;
        }
    }

    status.set_code(UCode::OK);

    return status;
}

std::future<UPayload> ZenohRpcClient::invokeMethod(const UUri &uri, 
                                                   const UPayload &requestPayload, 
                                                   const UAttributes &attributes) noexcept {
    if (0 == refCount_) {
        spdlog::error("ZenohRpcClient is not initialized");
        return std::future<UPayload>();
    }

    if (UMessageType::REQUEST != attributes.type()) {
        spdlog::error("Wrong message type = {}", UMessageTypeToString(attributes.type()).value());
        return std::future<UPayload>();
    }

    uprotocol::v1::UMessage msg;
    *msg.mutable_attributes() = attributes;
    msg.mutable_payload()->set_value(requestPayload.data(), requestPayload.size());

    std::vector<uint8_t> serializedMessage(msg.ByteSizeLong());
    if (!msg.SerializeToArray(serializedMessage.data(), serializedMessage.size())) {
        spdlog::error("SerializeToArray failed");
        return std::future<UPayload>();
    }

    auto uriStr = uri.toString();
    z_owned_reply_channel_t *channel = new z_owned_reply_channel_t(zc_reply_fifo_new(16));

    z_get_options_t opts = z_get_options_default();
    opts.timeout_ms = requestTimeoutMs_;

    if (0 != z_get(z_loan(session_), uriStr.c_str(), serializedMessage.data(), serializedMessage.size(), z_move(channel->send), &opts)) {
        spdlog::error("z_get failure");
        delete channel;
        return std::future<UPayload>();
    }

    auto future = threadPool_->submit([channel]() -> UPayload {
        return ZenohRpcClient::handleReply(channel);
    });

    if (!future.valid()) {
        spdlog::error("failed to invoke method");
    }

    return future;
}


UPayload ZenohRpcClient::handleReply(z_owned_reply_channel_t *channel) {
    z_owned_reply_t reply = z_reply_null();
    UPayload response(nullptr, 0, UPayloadType::VALUE);

    for (z_call(channel->recv, &reply); z_check(reply); z_call(channel->recv, &reply)) {
        if (z_reply_is_ok(&reply)) {
            z_sample_t sample = z_reply_ok(&reply);

            if (sample.payload.len == 0 || sample.payload.start == nullptr) {
                spdlog::error("Payload is empty");
                continue;
            }

            uprotocol::v1::UMessage msg;
            if (!msg.ParseFromArray(sample.payload.start, sample.payload.len)) {
                spdlog::error("ParseFromArray failed");
                continue;
            }

            const auto& payload = msg.payload();

            response = UPayload(payload.value().data(), payload.value().size(), UPayloadType::VALUE);

        } else {
            spdlog::error("error received");
            break;
        }

        z_drop(z_move(reply));
    }

    z_drop(z_move(reply));
    z_drop((channel));
    delete channel;

    return response;
}