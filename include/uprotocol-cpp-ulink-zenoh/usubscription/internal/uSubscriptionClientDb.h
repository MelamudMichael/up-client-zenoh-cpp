#include <uprotocol-cpp/transport/datamodel/UListener.h>
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

class USubscriptionClientDb {

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

            return uSubscriptionListener::instance().init();
        };

         /**
         * terminates the uSubClient
         * returns UCode_OK on success and ERROR on failure
         */
        UCode term() {

            return uSubscriptionListener::instance().term();
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
         * Get subscription state for the Uri.
         *
         * @param uri - Subscription URI.
         *
         * @return subscription state.
         */
        SubscriptionStatus_State getSubscriptionStatus(const UUri &uri) {

            std::string topic;
            if (!uri.SerializeToString(&topic)) {
                spdlog::error("Failed to SerializeToString");
                return SubscriptionStatus_State_UNSUBSCRIBED;
            }

            if (subStatusMap_.find(topic) == subStatusMap_.end()) {
                spdlog::error("Uri is not SUBSCRIBED");
                return SubscriptionStatus_State_UNSUBSCRIBED;
            }

            return subStatusMap_[topic];
        }

        UCode getPublisherStatus(const UUri &uri) {

            std::string topic;
            if (!uri.SerializeToString(&topic)) {
                spdlog::error("Failed to SerializeToString");
                return UCode::INTERNAL;
            }

            if (pubStatusMap_.find(topic) == pubStatusMap_.end()) {
                spdlog::error("Uri is not set as publisher");
                return UCode::UNAVAILABLE;
            }

            return pubStatusMap_[topic];
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
                spdlog::error("Failed to serialize topic");
                return UCode::INTERNAL;
            }

            if (notifyMap_.find(topic) != notifyMap_.end()) {
                spdlog::error("Topic already registered for notifications");
                return UCode::ALREADY_EXISTS;
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
                spdlog::error("Failed to serialize topic");
                return UCode::INTERNAL;
            }

            if (notifyMap_.find(topic) == notifyMap_.end()) {
                spdlog::error("Topic not registered for notifications");
                return UCode::NOT_FOUND;
            }
            notifyMap_.erase(topic);

            return UCode::OK;
         }


        UUri uSubUri_; // = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(),
                                               //              uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                   //          uprotocol::uri::UResource::forRpcRequest("subscribe"));


        /**
         * Notify and update the Subscription Change.
         *
         * @param payload - Payload containg the subscription status.
         *
         * @return UStatus of the processing.
         */
        UCode notifyUpdate(const UPayload &payload) {
            Update update;
            if (!update.ParseFromArray(payload.data(), payload.size())) {
                spdlog::error("ParseFromArray failed");
                return UCode::INTERNAL;
            }

            std::string subscribedTopic;
            if (!update.topic().SerializeToString(&subscribedTopic)) {
                spdlog::error("Failed to serialize topic");
                return UCode::INTERNAL;
            }

            subStatusMap_[subscribedTopic] = update.status().state();

            auto it = notifyMap_.find(subscribedTopic);
            if (notifyMap_.end() != it) {
                notifyFunc notify = it->second;
                if (nullptr != notify) {
                    notify(update.status());
                }
            }

            return UCode::OK;
        }

    private:

        /**
         * Map the created topic and callback for the notifications.
         */
        unordered_map<std::string, notifyFunc> notifyMap_;
        /**
         * Map the publish topic and status.
         */
        unordered_map<std::string, UCode> pubStatusMap_;
        /**
         * Map the subscribed topic and Subscription Status.
         */
        unordered_map<std::string, SubscriptionStatus_State> subStatusMap_;


    class uSubscriptionListener : public UListener {

    public:

        /**
         * Get the instance of the listener.
         *
         * @return Return the instance of the listener.
         */
        static uSubscriptionListener& instance() noexcept {

            static uSubscriptionListener listener;
            return listener;
        }

        /**
         * Initialize the listener.
         *
         * @return Retrun UCode_OK on success and ERROR on failure.
         */
        UCode init() {
            if (UCode::OK != ZenohUTransport::instance().init().code()) {
                spdlog::error("ZenohUTransport::instance().init failed");
                return UCode::INTERNAL;
            }

            if (UCode::OK != ZenohUTransport::instance().registerListener(uSubUpdatesUri, uSubscriptionListener::instance()).code()){
                spdlog::error("ZenohUTransport::instance().registerListener failed");
                return UCode::INTERNAL;
            }

            return UCode::OK;
        }

        /**
         * Terminate the listener.
         *
         * @return Retrun UCode_OK on success and ERROR on failure.
         */
        UCode term() {
            if (UCode::OK != ZenohUTransport::instance().unregisterListener(uSubUpdatesUri, uSubscriptionListener::instance()).code()){
                spdlog::error("ZenohUTransport::instance().unregisterListener failed");
                return UCode::INTERNAL;
            }

            if (UCode::OK != ZenohUTransport::instance().term().code()) {
                spdlog::error("ZenohUTransport::instance().term failed");
                return UCode::INTERNAL;
            }

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
                          const UAttributes &attributes) const override {
            UStatus status;
            status.set_code(UCode::OK);
            std::string str = uprotocol::uri::LongUriSerializer::serialize(uri);
            std::string subUri = uprotocol::uri::LongUriSerializer::serialize(uSubUpdatesUri);
            if (str == subUri) {
                UCode code = USubscriptionClientDb::instance().notifyUpdate(payload);
                status.set_code(code);
            }

            return status;
        }
    };

};

