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
    
    UStatus onReceive(const uprotocol::uri::UUri &uri, 
                      const UPayload &payload, 
                      const UAttributes &attributes) const {
        UStatus status;

        status.set_code(UCode::OK);

        return status;
    }
};

CreateTopicRequest buildCreateTopicRequest(uprotocol::uri::UUri uri) {

     CreateTopicRequest request;

    ::uprotocol::v1::UAuthority* mutableAuthority = request.mutable_topic()->mutable_authority();
    if (mutableAuthority != nullptr) {        

        if (true == uri.getUAuthority().isRemote()) {
            mutableAuthority->set_name(uri.getUAuthority().getDevice());
        }
    }

    ::uprotocol::v1::UEntity* mutableEntity = request.mutable_topic()->mutable_entity();
    if (mutableEntity != nullptr) {        
        uprotocol::uri::UEntity entity = uri.getUEntity();

        mutableEntity->set_name(entity.getName());
        
        if (true == entity.getId().has_value()) {
            mutableEntity->set_id(entity.getId().value());
        }

        if (true == entity.getVersion().has_value()) {
            mutableEntity->set_version_major(entity.getVersion().value());
        }
    }

    ::uprotocol::v1::UResource* mutableResource = request.mutable_topic()->mutable_resource();
    if (mutableResource != nullptr) {        

        uprotocol::uri::UResource resource = uri.getUResource();

        mutableResource->set_name(resource.getName());
        mutableResource->set_instance(resource.getInstance());
        mutableResource->set_message(resource.getMessage());

        if (true == resource.getId().has_value()) {
            mutableResource->set_id(resource.getId().value());
        }
    }

    return request;
}

SubscriptionRequest buildSubscriptionRequest(uprotocol::uri::UUri uri) {

    SubscriptionRequest request;

    ::uprotocol::v1::UAuthority* mutableAuthority = request.mutable_topic()->mutable_authority();
    if (mutableAuthority != nullptr) {        

        if (true == uri.getUAuthority().isRemote()) {
            mutableAuthority->set_name(uri.getUAuthority().getDevice());
        }
    }

    ::uprotocol::v1::UEntity* mutableEntity = request.mutable_topic()->mutable_entity();
    if (mutableEntity != nullptr) {        
        uprotocol::uri::UEntity entity = uri.getUEntity();

        mutableEntity->set_name(entity.getName());
        
        if (true == entity.getId().has_value()) {
            mutableEntity->set_id(entity.getId().value());
        }

        if (true == entity.getVersion().has_value()) {
            mutableEntity->set_version_major(entity.getVersion().value());
        }
    }

    ::uprotocol::v1::UResource* mutableResource = request.mutable_topic()->mutable_resource();
    if (mutableResource != nullptr) {        

        uprotocol::uri::UResource resource = uri.getUResource();

        mutableResource->set_name(resource.getName());
        mutableResource->set_instance(resource.getInstance());
        mutableResource->set_message(resource.getMessage());

        if (true == resource.getId().has_value()) {
            mutableResource->set_id(resource.getId().value());
        }
    }

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

    auto realUri = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), uprotocol::uri::UEntity::longFormat("real.app"), uprotocol::uri::UResource::longFormat("milliseconds"));
    auto demoUri = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), uprotocol::uri::UEntity::longFormat("demo.app"), uprotocol::uri::UResource::longFormat("milliseconds"));


    auto req1 = buildCreateTopicRequest(realUri);
    auto req2 = buildCreateTopicRequest(demoUri);
    auto req3 = buildSubscriptionRequest(realUri);

    UAttributesBuilder builder(Uuidv8Factory::create(), UMessageType::PUBLISH, UPriority::STANDARD);

    UAttributes attributes = builder.build();

    UPayload payload(nullptr, 0, UPayloadType::VALUE);

    std::getline(std::cin, userInput);    

    spdlog::warn("#SCENARIO #1 Start - Send without CreateTopic");
    auto retVal = transport->send(realUri, payload, attributes).code();
    spdlog::info("ret value = {} ", retVal);
    spdlog::warn("#SCENARIO #1 End - Send without CreateTopic");

    auto resp = uSubscriptionClient::instance().createTopic(req1);

    spdlog::info("response received real.app = {} ", resp);

    resp = uSubscriptionClient::instance().createTopic(req2);

    spdlog::info("response received demo.app = {} ", resp);

    std::getline(std::cin, userInput);    

    spdlog::warn("#SCENARIO #2 Start - Send without auth");
    retVal = transport->send(demoUri, payload, attributes).code();
    spdlog::info("ret value = {} ", retVal);
    spdlog::warn("#SCENARIO #2 End - Send without auth");  
   
    std::getline(std::cin, userInput);    

    uint8_t buf[1];
    UPayload validPayload(buf, 1, UPayloadType::VALUE);

    spdlog::warn("#SCENARIO #3 Start - Send with auth");
    retVal = transport->send(realUri, validPayload, attributes).code();
    spdlog::info("ret value = {} ", retVal);
    spdlog::warn("#SCENARIO #3 End - Send with auth"); 

    std::getline(std::cin, userInput);      
    spdlog::warn("#SCENARIO #4 Start - registerListener without subscribe");
    transport->registerListener(realUri, listener);
    spdlog::warn("#SCENARIO #4 End - registerListener without subscribe");


    std::getline(std::cin, userInput);     

    
    auto subResp = uSubscriptionClient::instance().subscribe(req3);

    spdlog::info("subscribe response received real.app = {} ", subResp.value().mutable_status()->state());

    spdlog::warn("#SCENARIO #5 Start - registerListener after subscribe");
    transport->registerListener(realUri, listener);
    spdlog::warn("#SCENARIO #5 End - registerListener after subscribe");

    while (!gTerminate) {

         sleep(1);
    }
    
    if (UCode::OK != transport->term().code()) {
        spdlog::error("ZenohUTransport::instance().term() failed");
        return -1;
    }

    return 0;
}
