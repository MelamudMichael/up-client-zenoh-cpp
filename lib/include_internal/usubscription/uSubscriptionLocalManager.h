
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
 
#ifndef _H_USUBSCRIPTION_LOCAL_MANAGER_
#define _H_USUBSCRIPTION_LOCAL_MANAGER_

#include <unordered_map>

#include <ustatus.pb.h>
#include <up-client-zenoh-cpp/transport/zenohUTransport.h>
#include <core/usubscription/v3/usubscription.pb.h>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::core::usubscription::v3;
using namespace uprotocol::v1;

class SubscriptionLocalManager : public UListener {

    public:

        SubscriptionLocalManager(const SubscriptionLocalManager&) = delete;
         
        SubscriptionLocalManager& operator=(const SubscriptionLocalManager&) = delete;

        static SubscriptionLocalManager& instance() noexcept;
               
        UStatus init();

        UStatus term();
       
        template<typename T>
        UStatus setStatus(UUri uri, 
                          T status);

        SubscriptionStatus_State getSubscriptionStatus(const UUri &uri);

        UCode getPublisherStatus(const UUri &uri);
        
    private:
    
        SubscriptionLocalManager() {};

        UStatus onReceive(const UUri& uri,
                          const UPayload& payload,
                          const UAttributes& attributes) const override;

        unordered_map<std::string, UCode> pubStatusMap_;
        unordered_map<std::string, SubscriptionStatus_State> subStatusMap_;
};

#endif //_H_USUBSCRIPTION_LOCAL_MANAGER_