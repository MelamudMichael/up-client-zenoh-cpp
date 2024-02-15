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

        uint8_t counter;
        memcpy(&counter, payload.data(), payload.size());
        std::string topic = ::uprotocol::uri::LongUriSerializer::serialize(uri);
        spdlog::info("Receive Message - {} on Topic: {} \n", counter, topic);

        status.set_code(UCode::OK);

        return status;
    }
};

class Client {

public:

    Client(const std::string name, const std::string publisher, const std::string subscriber) :
        name_(name),
        publisherUri_(LongUriSerializer::deserialize(publisher)),
        subscriberUri_(LongUriSerializer::deserialize(subscriber)) {}

    Client(const Client &) = delete;

    virtual ~Client() = default;

    virtual void test() = 0;

protected:

    int init() {
        if (UCode::OK != ZenohUTransport::instance().init().code()) {
            spdlog::error("ZenohUTransport::instance().init failed");
            return -1;
        }

        if (UCode::OK != uSubscriptionClient::instance().init()) {
            spdlog::error("uSubscriptionClient::instance().init() failed");
            return -1;
        }
        return 0;
    }

    int term() {
        if (UCode::OK != uSubscriptionClient::instance().term()) {
            spdlog::error("uSubscriptionClient::instance().term() failed");
            return -1;
        }

        if (UCode::OK != ZenohUTransport::instance().term().code()) {
            spdlog::error("ZenohUTransport::instance().term() failed");
            return -1;
        }
        return 0;
    }

    void createTopic() {
        CreateTopicRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        UCode code = uSubscriptionClient::instance().createTopic(request);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void deprecateTopic() {
        DeprecateTopicRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        UCode code = uSubscriptionClient::instance().deprecateTopic(request);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void subscribe() {
        SubscriptionRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        request.mutable_subscriber()->mutable_uri()->CopyFrom(subscriberUri_);
        auto response = uSubscriptionClient::instance().subscribe(request);
        UCode code = response.has_value() ? response.value().status().code() : UCode::UNKNOWN;
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void unsubscribe() {
        UnsubscribeRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        request.mutable_subscriber()->mutable_uri()->CopyFrom(subscriberUri_);
        UCode code = uSubscriptionClient::instance().unSubscribe(request);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void registerNotifications() {
        NotificationsRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        UCode code = uSubscriptionClient::instance().registerNotifications(request, receiveSusbcriptionChange);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void unregisterNotifications() {
        NotificationsRequest request;
        request.mutable_topic()->CopyFrom(publisherUri_);
        UCode code = uSubscriptionClient::instance().unregisterNotifications(request);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void publish() {
        UAttributesBuilder builder(Uuidv8Factory::create(), UMessageType::PUBLISH, UPriority::STANDARD);
        UAttributes attributes = builder.build();
        uint8_t *message = getCounter();
        UPayload payload(message, sizeof(uint8_t), UPayloadType::VALUE);
        UCode code = ZenohUTransport::instance().send(publisherUri_, payload, attributes).code();
        spdlog::info("{}: {} Message - {} \n", name_, __func__, *message);
    }

    void registerListener() {
        UStatus status = ZenohUTransport::instance().registerListener(publisherUri_, listener);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    void unregisterListener() {
        UStatus status = ZenohUTransport::instance().unregisterListener(publisherUri_, listener);
        spdlog::info("{}: {} \n", name_, __func__);
    }

    static void receiveSusbcriptionChange(const SubscriptionStatus& status) {
        spdlog::info("Received Subscription Change Update - {} \n", SubscriptionStatus_State_Name(status.state()));
    }

    static uint8_t* getCounter() {
        static uint8_t counter = 0;
        ++counter;
        return &counter;
    }

private:

    const std::string name_;
    const UUri publisherUri_;
    const UUri subscriberUri_;
    TimeListener listener;
};

class PublisherTest : public Client {

public:
    PublisherTest(std::string publisher) : Client("PUBLISHER", publisher, "") {
        spdlog::info("PUBLISHER: Topic - {} \n", publisher);
    }

    ~PublisherTest() = default;

    void test() {
        init();

        createTopic();
        registerNotifications();

        while (!gTerminate) {
            publish();

            spdlog::info("Press 'Enter' to Publish Message, 'X' to Stop execution: ");
            std::string userInput;
            std::getline(std::cin, userInput);
            if ((userInput == "X") || (userInput == "x")) {
                break;
            }
        }

        unregisterNotifications();
        deprecateTopic();

        term();
    }

};

class SubscriberTest : public Client {

public:
    SubscriberTest(std::string subscriber, std::string subscription) : Client("SUBSCRIBER", subscription, subscriber) {
        spdlog::info("SUBSCRIBER: Topic - {} subscribing to {} \n", subscriber, subscription);
    }

    void test () {
        init();

        subscribe();
        registerListener();

        while (!gTerminate) {
            spdlog::info("Press 'S' to Subscribe, 'U' to Unsubscribe, 'X' to Stop execution: ");
            std::string userInput;
            std::getline(std::cin, userInput);

            if ((userInput == "S") || (userInput == "s")) {

                subscribe();
                registerListener();

            } else if ((userInput == "U") || (userInput == "u")) {

                unregisterListener();
                unsubscribe();

            } else if ((userInput == "X") || (userInput == "x")) {
                break;
            }
        }

        unregisterListener();
        unsubscribe();

        term();
    }

};

int main(int argc, char **argv) {
    const std::string publishUri = "/publisher.app/1/counter";
    const std::string subscribeUri = "/subscriber.app/1/monitoring";

    signal(SIGINT, signalHandler);

    if (1 < argc) {
        if (0 == strcmp("-d", argv[1])) {
            spdlog::set_level(spdlog::level::debug);
        }
    }

    // Select Client Type: Publisher or Subscriber
    spdlog::info("Input 'P' for Publisher or 'S' for Subscriber: ");
    std::string userInput;
    std::getline(std::cin, userInput);
    std::string name;

    std::unique_ptr<Client> client;
    if ((userInput == "P") || (userInput == "p")) {
        client = make_unique<PublisherTest>(publishUri);
    } else if ((userInput == "S") || (userInput == "s")) {
        client = make_unique<SubscriberTest>(subscribeUri, publishUri);
    } else {
        spdlog::error("Invalid Input: {}", userInput);
        return -1;
    }

    // Execute Client Test
    client->test();

    return 0;
}
