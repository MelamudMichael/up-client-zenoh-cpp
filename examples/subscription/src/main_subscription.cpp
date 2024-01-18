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

#include <csignal>
#include <uprotocol-cpp-ulink-zenoh/transport/zenohUTransport.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/uSubscriptionClient.h>
#include <uprotocol-cpp/uuid/factory/Uuidv8Factory.h>
#include <uprotocol-cpp/uri/serializer/LongUriSerializer.h>
#include <src/main/proto/ustatus.pb.h>

using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;
using namespace uprotocol::uSubscription;

bool gTerminate = false; 

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true; 
    }
}

class TimeListener : public UListener {
    
    UStatus onReceive(const UUri &uri, 
                      const UPayload &payload, 
                      const UAttributes &attributes) const {
        UStatus status;

        status.set_code(UCode::OK);

        return status;
    }
};

CreateTopicRequest buildCreateTopicRequest(UUri uri) {

    CreateTopicRequest request;

    request.set_allocated_topic(&uri);

    return request;
}

DeprecateTopicRequest buildDeprecateTopicRequest(UUri uri) {

    DeprecateTopicRequest request;

    request.set_allocated_topic(&uri);
    
    return request;
}

UnsubscribeRequest buildUnsubscribeRequest(UUri uri) {

    UnsubscribeRequest request;

    request.set_allocated_topic(&uri);

    return request;
}

SubscriptionRequest buildSubscriptionRequest(UUri uri) {

    SubscriptionRequest request;

    request.set_allocated_topic(&uri);

    return request;
}

int main(int argc, char **argv) {

    TimeListener listener;
    std::string userInput;

    signal(SIGINT, signalHandler);

    if (1 < argc) {
        if (0 == strcmp("-d", argv[1])) {
            spdlog::set_level(spdlog::level::debug);
        }
    }

    ZenohUTransport *transport = &ZenohUTransport::instance();
   
    if (UCode::OK != transport->init().code()) {
        spdlog::error("ZenohUTransport::instance().init failed");
        return -1;
    }

    if (UCode::OK != uSubscriptionClient::instance().init()) {
        spdlog::error("uSubscriptionClient::instance().init() failed");
        return -1;
    }

    auto realUri = LongUriSerializer::deserialize("/real.app/1/milliseconds");
    auto demoUri = LongUriSerializer::deserialize("/demo.app/1/milliseconds");

    auto req1 = buildCreateTopicRequest(realUri);
    auto req2 = buildCreateTopicRequest(demoUri);
    auto req3 = buildSubscriptionRequest(realUri);
    auto req4 = buildUnsubscribeRequest(realUri);

    UAttributesBuilder builder(Uuidv8Factory::create(), UMessageType::PUBLISH, UPriority::STANDARD);

    UAttributes attributes = builder.build();

    UPayload payload(nullptr, 0, UPayloadType::VALUE);

    std::getline(std::cin, userInput);    

    spdlog::info("########## SCENARIO #1 Start - Try to send without calling CreateTopic ##########");
    auto retVal = transport->send(realUri, payload, attributes).code();
    spdlog::info("########## SCENARIO #1 End - Send without calling CreateTopic (return value == {}) ##########", retVal);

    spdlog::info("########## Sending CreateTopic Requests to uSubscription ##########");

    auto resp = uSubscriptionClient::instance().createTopic(req1);
    spdlog::info("\t########## response received real.app = {} ", resp);
    resp = uSubscriptionClient::instance().createTopic(req2);
    spdlog::info("\t########## response received demo.app = {} ", resp);

    std::getline(std::cin, userInput);    

    spdlog::info("########## SCENARIO #2 Start - Try to send without authorization ##########");
    retVal = transport->send(demoUri, payload, attributes).code();
    spdlog::info("########## SCENARIO #2 End - Send without authorization (return value == {}) ##########", retVal);  
   
    std::getline(std::cin, userInput);    

    uint8_t buf[1];
    UPayload validPayload(buf, 1, UPayloadType::VALUE);

    spdlog::info("########## SCENARIO #3 Start - Try to send with authorizaion ##########");
    retVal = transport->send(realUri, validPayload, attributes).code();
    spdlog::info("########## SCENARIO #3 End - Send with authorizatdion (return value == {}) ##########", retVal); 

    std::getline(std::cin, userInput);      
    spdlog::info("########## SCENARIO #4 Start - Try to RegisterListener without subscribe ##########");
    retVal = transport->registerListener(realUri, listener).code();
    spdlog::info("########## SCENARIO #4 End - registerListener without subscribe (return value == {}) ##########", retVal);

    std::getline(std::cin, userInput);     
    
    auto subResp = uSubscriptionClient::instance().subscribe(req3);

    spdlog::info("\t########## subscribe response received real.app = {} ", subResp.value().mutable_status()->state());

    spdlog::info("########## SCENARIO #5 Start - Try to RegisterListener after subscribe ##########");
    transport->registerListener(realUri, listener);
    spdlog::info("########## SCENARIO #5 End - registerListener after subscribe ##########");

    spdlog::info("########## SCENARIO #6 Start - unsubscribe ##########");
    uSubscriptionClient::instance().unSubscribe(req4);
    spdlog::info("########## SCENARIO #6 End - unsubscribe ##########");

    while (!gTerminate) {

         sleep(1);
    }
    
    if (UCode::OK != uSubscriptionClient::instance().term()) {
        spdlog::error("uSubscriptionClient::instance().term() failed");
        return -1;
    }

    if (UCode::OK != transport->term().code()) {
        spdlog::error("ZenohUTransport::instance().term() failed");
        return -1;
    }

    return 0;
}
