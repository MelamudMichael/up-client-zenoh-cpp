#include <uprotocol-cpp-ulink-zenoh/transport/zenohUTransport.h>
#include <uprotocol-cpp/uri/serializer/LongUriSerializer.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/common/uSubscriptionCommon.h>
#include <uprotocol-cpp-ulink-zenoh/usubscription/uSubscriptionClient.h>
#include <core/usubscription/v3/usubscription.pb.h>
#include <google/protobuf/message.h>
#include <ustatus.pb.h>
#include <uri.pb.h>
#include <unordered_map>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::uSubscription;

class USubscriptionClientDb : public UListener {

    public:

        static USubscriptionClientDb& instance() noexcept {

            static USubscriptionClientDb clientInternal;

            return clientInternal;
        }

         /**
         * initialized the uSubClient
         * returns UCode_OK on success and ERROR on failure
         */
        UCode init() {

            if (UCode::OK != ZenohUTransport::instance().init().code()) {

                spdlog::error("ZenohUTransport::instance().init failed");
                return UCode::UNKNOWN;
            }

            if (UCode::OK != ZenohUTransport::instance().registerListener(uSubUpdatesUri, USubscriptionClientDb::instance()).code()) {

                spdlog::error("ZenohUTransport::instance().init failed");
                return UCode::UNKNOWN;
            }

            return UCode::OK;
        };

         /**
         * terminates the uSubClient
         * returns UCode_OK on success and ERROR on failure
         */
        UCode term() {

            pid_t pid = getpid();

            if (UCode::OK != ZenohUTransport::instance().unregisterListener(uSubUpdatesUri, USubscriptionClientDb::instance()).code()) {

                spdlog::error("ZenohUTransport::instance().init failed");
                return UCode::UNKNOWN;
            }

            if (UCode::OK != ZenohUTransport::instance().term().code()) {

                spdlog::error("ZenohUTransport::instance().term failed");
                return UCode::UNKNOWN;
            }

            return UCode::OK;
        };

        /**
         * set subscription status (no RPC call)
         * @param uri the URI
         * @param status subscription status
         * @return returns UCode_OK on success and ERROR on failure
         */
        template <typename T>
        UCode setStatus(UUri uri,
                        T status) {

            std::string serUri;
            if (!uri.SerializeToString(&serUri)) {
                spdlog::error("SerializeToString failed");
                return UCode::INTERNAL;
            }

            if constexpr (std::is_same_v<T, SubscriptionStatus_State>) {
                subStatusMap_[serUri] = status;
            } else if constexpr (std::is_same_v<T, UCode>){
                pubStatusMap_[serUri] = status;
            } else {
            ///    valueSize = sizeof(T);
            }

            return UCode::OK;
        }

         /**
         * get subscription status (no RPC call)
         * @param uri the URI
         * @param status subscription status
         * @return returns UCode_OK on success and ERROR on failure
         */
        SubscriptionStatus_State getSubscriptionStatus(const UUri &uri) {

            // auto u = Temp::buildTopic(uri);

            // std::string serUri;
            // if (!u.SerializeToString(&serUri)) {
            //     spdlog::error("SerializeToString failed");
            //     return SubscriptionStatus_State_UNSUBSCRIBED;
            // }

            // if (subStatusMap_.find(serUri) != subStatusMap_.end()) {
            //     return subStatusMap_[serUri];
            // } else {
            //     return SubscriptionStatus_State_UNSUBSCRIBED;
            // }

//            return subStatusMap_[serUri];
            return SubscriptionStatus_State_UNSUBSCRIBED;
        }

        UCode getPublisherStatus(const UUri &uri) {

            // auto u = Temp::buildTopic(uri);

            // std::string serUri;
            // if (!u.SerializeToString(&serUri)) {
            //     return UCode::INTERNAL;
            // }

            // if (pubStatusMap_.find(serUri) != pubStatusMap_.end()) {
            //     return pubStatusMap_[serUri];
            // } else {
            //     return UCode::UNAVAILABLE;
            // }
            return UCode::OK;
        }

        /**
         * Register for notifications on a topic.
         *
         * @param topicUri - Topic to be registered.
         * @param func - Callback function to notify.
         *
         * @return Return OK if success.
         */
        UCode registerForNotifications(const UUri &topicUri,
                                       const notifyFunc &func ) {
            std::string topic;
            if (!topicUri.SerializeToString(&topic)) {
                return UCode::INTERNAL;
            }

            notifyMap_[topic] = func;

            return UCode::OK;
        }

        /**
         * Unregister the notifications of a topic.
         *
         * @param topicUri - Topic to be unregistered.
         *
         * @return Return OK if success.
         */
        UCode unregisterForNotifications(const UUri &topicUri) {
            std::string topic;
            if (!topicUri.SerializeToString(&topic)) {
                return UCode::INTERNAL;
            }

            notifyMap_.erase(topic);

            return UCode::OK;
         }

        /**
         * Handle the received events.
         *
         * @param uri - Uri for the event.
         * @param payload - Payload of the message.
         * @param attributes - Attributes for the message.
         *
         * @return Return UStatus.
         */
        UStatus onReceive(const UUri &uri,
                          const UPayload &payload,
                          const UAttributes &attributes) const {
            UStatus status;
            status.set_code(UCode::OK);

            if (uri == uSubUpdatesUri) {
                UCode code = notifyUpdate(payload);
                status.set_code(code);
            }

            return status;
        }

        UUri uSubUri_; // = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(),
                                               //              uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                   //          uprotocol::uri::UResource::forRpcRequest("subscribe"));

    private:

        /**
         * Notify the Subscription Change Update.
         *
         * @param payload - Payload containg the subscription status.
         *
         * @return UStatus of the processing.
         */
        UCode notifyUpdate(const UPayload &payload) const {
            Update update;
            if (!update.ParseFromArray(payload.data(), payload.size())) {
                spdlog::error("ParseFromArray failed");
                return UCode::INTERNAL;
            }

            std::string subscribedTopic;
            if (!update.topic().SerializeToString(&subscribedTopic)) {
                return UCode::INTERNAL;
            }

            auto it = notifyMap_.find(subscribedTopic);
            if (notifyMap_.end() != it) {
                notifyFunc notify = it->second;
                if (nullptr != notify) {
                    notify(update.status());
                }
            }

            return UCode::OK;
        }

        /**
         * Map to store the callbacks for the notifications.
         */
        unordered_map<std::string, notifyFunc> notifyMap_;

        unordered_map<std::string, UCode> pubStatusMap_;

        unordered_map<std::string, SubscriptionStatus_State> subStatusMap_;

};

UCode getPublisherStatus(const UUri &uri) {
   return USubscriptionClientDb::instance().getPublisherStatus(uri);
}

SubscriptionStatus_State getSubscriberStatus(const UUri &uri) {
    return USubscriptionClientDb::instance().getSubscriptionStatus(uri);
}
