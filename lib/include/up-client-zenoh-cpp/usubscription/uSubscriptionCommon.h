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
 
#ifndef _H_USUBSCRIPTION_COMMON_H_
#define _H_USUBSCRIPTION_COMMON_H_

#include <unordered_map>
#include <up-cpp/uri/serializer/LongUriSerializer.h>
#include <uri.pb.h>

using namespace uprotocol::v1;
using namespace uprotocol::uri;

namespace uprotocol::uSubscription {

/* request enumberations */
enum class Request {
    SUBSCRIPTION_REQUEST = 1,
    UNSUBSCRIBE_REQUEST,
    FETCH_SUBSCRIPTION_REQUEST,
    CREATE_TOPIC_REQUEST,
    DEPRECATE_TOPIC_REQUEST,
    NOTIFICATION_REQUEST,
    FETCH_SUBSCRIBERS_REQUEST,
    RESET_REQUEST
};

static std::unordered_map<std::string, Request> requestStrToNum = {
    {"uprotocol.core.usubscription.v3.SubscriptionRequest",         Request::SUBSCRIPTION_REQUEST},
    {"uprotocol.core.usubscription.v3.UnsubscribeRequest",          Request::UNSUBSCRIBE_REQUEST},
    {"uprotocol.core.usubscription.v3.FetchSubscriptionsRequest",   Request::FETCH_SUBSCRIPTION_REQUEST},
    {"uprotocol.core.usubscription.v3.CreateTopicRequest",          Request::CREATE_TOPIC_REQUEST},
    {"uprotocol.core.usubscription.v3.DeprecateTopicRequest",       Request::DEPRECATE_TOPIC_REQUEST},
    {"uprotocol.core.usubscription.v3.NotificationsRequest",        Request::NOTIFICATION_REQUEST},
    {"uprotocol.core.usubscription.v3.FetchSubscribersRequest",     Request::FETCH_SUBSCRIBERS_REQUEST},
    {"uprotocol.core.usubscription.v3.ResetRequest",                Request::RESET_REQUEST}};
}

//core.usubscription.subscribe
static UUri uSubRequestsUri = LongUriSerializer::deserialize("/core.usubscription/3/rpc.subscribe"); 

static UUri uSubForwareRemoteRequestsUri = LongUriSerializer::deserialize("/core.ustreamer/3/rpc.remote.subscribe"); 

static UUri uSubUpdateUri = LongUriSerializer::deserialize("/core.usubscription/3/subscriptions#Update");

#endif /* _USUBSCRIPTION_COMMON_H_ */