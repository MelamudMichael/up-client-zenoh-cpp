/*
 * Copyright (c) 2024 General Motors GTO LLC
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
 * SPDX-FileCopyrightText: 2024 General Motors GTO LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZENOH_RPC_CLIENT_H_
#define _ZENOH_RPC_CLIENT_H_

#include <up-cpp/rpc/RpcClient.h>
#include <up-cpp/utils/ThreadPool.h>
#include <up-core-api/ustatus.pb.h>
#include <up-core-api/uri.pb.h>
#include <zenoh.h>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::utils;
using namespace uprotocol::v1;

class ZenohRpcClient : public RpcClient {

    public:
        ZenohRpcClient(const ZenohRpcClient&) = delete;
        ZenohRpcClient& operator=(const ZenohRpcClient&) = delete;

        /**
        * Support for RPC method invocation.
        * @param topic topic of the method to be invoked (i.e. the name of the API we are calling).
        * @param payload The request message to be sent to the server.
        * @param attributes Metadata for the method invocation (i.e. priority, timeout, etc.)
        * @return Returns the CompletableFuture with the result or exception.
        */
        std::future<UPayload> invokeMethod(const UUri &uri, 
                                           const UPayload &payload, 
                                           const UAttributes &attributes) noexcept;
    protected:
        /* Initialization success/failure status */
        UStatus rpcSuccess_;

        ZenohRpcClient() noexcept;
        ~ZenohRpcClient() noexcept;

    private:
        static UPayload handleReply(z_owned_reply_channel_t *channel);

        /* zenoh session handle*/
        z_owned_session_t session_;
        
        std::shared_ptr<ThreadPool> threadPool_;

        static constexpr auto requestTimeoutMs_ = 5000;
        static constexpr auto queueSize_ = size_t(20);
        static constexpr auto maxNumOfCuncurrentRequests_ = size_t(2);

};

#endif /*_ZENOH_RPC_CLIENT_H_*/
