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

            if (UCode::OK != ZenohUTransport::instance().registerListener(uSubUriUpdates_, USubscriptionClientDb::instance()).code()) {

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

            if (UCode::OK != ZenohUTransport::instance().unregisterListener(uSubUriUpdates_, USubscriptionClientDb::instance()).code()) {

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

        }
        
        UCode registerForNotifications(UUri uri, 
                                       const notifyFunc &func ) {
            // std::string serUri;

            // if (!uri.SerializeToString(&serUri)) {
            //     return UCode::INTERNAL;
            // }

        //    notifyMap_[serUri] = func;

            return UCode::OK;
        }

        UStatus onReceive(const UUri &uri, 
                          const UPayload &payload, 
                          const UAttributes &attributes) const {

            Update res;
         
            if (false == res.ParseFromArray(payload.data(), payload.size())) {
                spdlog::error("ParseFromArray failed");
                //return UCode::UNKNOWN;
            }
            
            //check notifyMap_
            //check pubStatusMap_
            //check subStatusMap_

            UStatus status;

            status.set_code(UCode::OK);

            return status;        
        }

        UUri uSubUri_; // = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), 
                                               //              uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                   //          uprotocol::uri::UResource::forRpcRequest("subscribe"));

        UUri uSubUriUpdates_ ;//= uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), 
                                                         //           uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                       //             uprotocol::uri::UResource::forRpcRequest("subscriptions#Update"));
    private:

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
