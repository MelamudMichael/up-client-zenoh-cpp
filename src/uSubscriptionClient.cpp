#include <uprotocol-cpp-ulink-zenoh/rpc/zenohRpcClient.h>
#ifdef USUBSCRIPTION_CLIENT_EXT
#include <uSubscriptionClientExt.h>
#else 
#include <uprotocol-cpp-ulink-zenoh/usubscription/uSubscriptionClientExtStub.h>
#endif
#include <uprotocol-cpp-ulink-zenoh/usubscription/common/uSubscriptionCommon.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/uSubscriptionClient.h>
#include <uprotocol-cpp/uri/datamodel/UUri.h>
#include <uprotocol-cpp/uuid/serializer/UuidSerializer.h>
#include <uprotocol-cpp/uuid/factory/Uuidv8Factory.h>
#include <google/protobuf/message.h>
#include <core/usubscription/v3/usubscription.pb.h>
#include <ustatus.pb.h>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::core::usubscription::v3;
using namespace uprotocol::uSubscription;

uSubscriptionClient& uSubscriptionClient::instance() noexcept {
    
    static uSubscriptionClient client;

    return client;
}

UCode uSubscriptionClient::init() {
   
    if (UCode::OK != USubscriptionClientExt::instance().init()) {

        spdlog::error("USubscriptionClientExt::instance().init() failed");
        return UCode::UNKNOWN;
    }
    
    if (UCode::OK != ZenohRpcClient::instance().init().code()) {

        spdlog::error("ZenohRpcClient::instance().init failed");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}

UCode uSubscriptionClient::term() {

     if (UCode::OK != USubscriptionClientExt::instance().term()) {

        spdlog::error("USubscriptionClientExt::instance().term() failed");
        return UCode::UNKNOWN;
    }

    if (UCode::OK != ZenohRpcClient::instance().term().code()) {
        
        spdlog::error("ZenohRpcClient::instance().term failed");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}


UCode uSubscriptionClient::createTopic(CreateTopicRequest &request) {

    USubscriptionClientExt::instance().setStatus(request.topic(), UCode::UNKNOWN);

    UPayload payload = sendRequest(request);
    if (0 == payload.size()) {
        spdlog::error("payload size is 0");
        return UCode::UNKNOWN;
    }

    UStatus res;
           
    if (false == res.ParseFromArray(payload.data(), payload.size())) {
        spdlog::error("ParseFromArray failed");
        return UCode::UNKNOWN;
    }

    NotificationsRequest notifyReq;

    auto topic = request.topic();

    ::uprotocol::v1::UUri *uri = notifyReq.mutable_topic();
    
    ::uprotocol::core::usubscription::v3::SubscriberInfo* info = notifyReq.mutable_subscriber();

    ::google::protobuf::RepeatedPtrField<::google::protobuf::Any>* details_field = info->mutable_details();
        
    notifyReq.mutable_topic()->CopyFrom(topic);

    return USubscriptionClientExt::instance().setStatus(request.topic(), res.code());
}

UCode uSubscriptionClient::registerNotifications(NotificationsRequest &request,
                                                 const notifyFunc func) {

    ::uprotocol::core::usubscription::v3::SubscriberInfo* info = request.mutable_subscriber();
    ::google::protobuf::RepeatedPtrField<::google::protobuf::Any>* details_field = info->mutable_details();
        
    pid_t pid = getpid();

    std::string spid = std::to_string(pid);

    details_field->Add()->set_value(const_cast<char*>(spid.c_str()));

    USubscriptionClientExt::instance().registerForNotifications(request.topic(), func);

    auto payload = sendRequest(request);
    if (0 == payload.size()) {
        spdlog::error("payload size is 0");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}

UCode uSubscriptionClient::deprecateTopic(const DeprecateTopicRequest &request) {
    
    UPayload payload = sendRequest(request);

    if (0 == payload.size()) {
        spdlog::error("payload size is 0");
        return UCode::UNKNOWN;
    }

    UStatus res;
           
    if (false == res.ParseFromArray(payload.data(), payload.size())) {
        spdlog::error("ParseFromArray failed");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}

std::optional<SubscriptionResponse> uSubscriptionClient::subscribe(const SubscriptionRequest &request,
                                                                   std::optional<notifyFunc> func) {
    UPayload payload = sendRequest(request);

    if (0 == payload.size()) {
        spdlog::error("payload size is 0");
        return std::nullopt;
    }

    SubscriptionResponse resp;

    if (false == resp.ParseFromArray(payload.data(), payload.size())) {
        spdlog::error("ParseFromArray failed");
        return std::nullopt;
    }
    
    USubscriptionClientExt::instance().setStatus(request.topic(), resp.status());
    
    return resp;
}

template <typename T>
UPayload uSubscriptionClient::sendRequest(const T &request) noexcept {

    UUID uuid = Uuidv8Factory::create();

    uint8_t buffer[request.ByteSizeLong() + sizeof(uint8_t)];
    size_t size = request.ByteSizeLong() ;
    
    UPayload retPayload(nullptr, 0, UPayloadType::REFERENCE);

    do {

        if (false == request.SerializeToArray(buffer + 1, size)) {
            spdlog::error("SerializeToArray failure");
            break;
        }

        const google::protobuf::Descriptor* descriptor = request.GetDescriptor();

        buffer[0] = static_cast<uint8_t>(requestStrToNum[descriptor->full_name()]);

        UPayload payload(buffer, sizeof(buffer), UPayloadType::REFERENCE);
       
        auto type = UMessageType::REQUEST;
        auto priority = UPriority::STANDARD;

        UAttributesBuilder builder(uuid, type, priority);

        auto future = ZenohRpcClient::instance().invokeMethod(USubscriptionClientExt::instance().uSubUri_, payload, builder.build());
        if (false == future.valid()) {
            spdlog::error("result is not valid");
            break;
        }

        std::future_status status;
    
        switch (status = future.wait_for(responseTimeout_); status) {
            case std::future_status::timeout: {
                spdlog::error("timeout received while waiting for response");
            } 
            break;
            case std::future_status::ready: {
                retPayload= future.get();
            }
            break;
        }

    } while(0);

    return retPayload;
}

