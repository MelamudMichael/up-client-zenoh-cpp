#include <uprotocol-cpp-ulink-zenoh/rpc/zenohRpcClient.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/uSubscriptionClient.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/common/uSubscriptionCommon.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/internal/uSubscriptionClientDb.h>
#include <uprotocol-cpp/uri/datamodel/UUri.h>
#include <uprotocol-cpp/uuid/serializer/UuidSerializer.h>
#include <uprotocol-cpp/uuid/factory/Uuidv8Factory.h>
#include <google/protobuf/message.h>
//#include <core/usubscription/v3/usubscription.pb.h>
#include <ustatus.pb.h>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
//using namespace uprotocol::core::usubscription::v3;
using namespace uprotocol::uSubscription;

uSubscriptionClient& uSubscriptionClient::instance() noexcept {

    static uSubscriptionClient client;

    return client;
}

UCode uSubscriptionClient::init() {

    if (UCode::OK != USubscriptionClientDb::instance().init()) {

        spdlog::error("USubscriptionClientDb::instance().init() failed");
        return UCode::UNKNOWN;
    }

    if (UCode::OK != ZenohRpcClient::instance().init().code()) {

        spdlog::error("ZenohRpcClient::instance().init failed");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}

UCode uSubscriptionClient::term() {

     if (UCode::OK != USubscriptionClientDb::instance().term()) {

        spdlog::error("USubscriptionClientDb::instance().term() failed");
        return UCode::UNKNOWN;
    }

    if (UCode::OK != ZenohRpcClient::instance().term().code()) {

        spdlog::error("ZenohRpcClient::instance().term failed");
        return UCode::UNKNOWN;
    }

    return UCode::OK;
}

UCode uSubscriptionClient::createTopic(CreateTopicRequest &request) {

    USubscriptionClientDb::instance().setStatus(request.topic(), UCode::UNKNOWN);

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

    if (res.code() == UCode::ALREADY_EXISTS) {
        USubscriptionClientDb::instance().setStatus(request.topic(), UCode::OK);
        return UCode::OK;
    } else {
        USubscriptionClientDb::instance().setStatus(request.topic(), res.code());
        return res.code();
    }

    return res.code();
}

UCode uSubscriptionClient::registerNotifications(NotificationsRequest &request,
                                                 const notifyFunc func) {

    USubscriptionClientDb::instance().registerForNotifications(request.topic(), func);

    return UCode::OK;
}

UCode uSubscriptionClient::unregisterNotifications(const NotificationsRequest &request) {

    USubscriptionClientDb::instance().unregisterForNotifications(request.topic());

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
        spdlog::error("{}: payload size is 0", __func__);
        return std::nullopt;
    }

    SubscriptionResponse response;

    if (false == response.ParseFromArray(payload.data(), payload.size())) {
        spdlog::error("{}: Failed to ParseFromArray", __func__);
        return std::nullopt;
    }

    if (UCode::OK == response.status().code()) {
        USubscriptionClientDb::instance().setStatus(request.topic(), response.status().state());
    } else {
        spdlog::error("{}: Failed - {}", __func__, UCode_Name(response.status().code()));
    }

    return response;
}

UCode uSubscriptionClient::unSubscribe(const UnsubscribeRequest &request) {
    UPayload payload = sendRequest(request);

    if (0 == payload.size()) {
        spdlog::error("{}: payload size is 0", __func__);
        return UCode::UNAVAILABLE;
    }

    UStatus status;

    if (false == status.ParseFromArray(payload.data(), payload.size())) {
        spdlog::error("{}: Failed to ParseFromArray", __func__);
        return UCode::INTERNAL;
    }

    UCode code = status.code();
    if (UCode::OK == code) {
        USubscriptionClientDb::instance().setStatus(request.topic(), SubscriptionStatus_State_UNSUBSCRIBED);
    } else {
        spdlog::error("{}: Failed - {}", __func__, UCode_Name(status.code()));
    }

    return code;
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

        UAttributesBuilder builder(uuid, UMessageType::REQUEST, UPriority::STANDARD);

        while (retPayload.size() == 0) {
            auto future = ZenohRpcClient::instance().invokeMethod(uSubRequestsUri, payload, builder.build());
            if (false == future.valid()) {
                spdlog::error("result is not valid");
                break;
            }

            switch (std::future_status status = future.wait_for(responseTimeout_); status) {
                case std::future_status::timeout: {
                    spdlog::error("timeout received while waiting for response");
                    return retPayload;
                }
                break;
                case std::future_status::ready: {
                    retPayload = future.get();
                }
                break;
            }

            sleep(1);
        }

    } while(0);

    return retPayload;
}
