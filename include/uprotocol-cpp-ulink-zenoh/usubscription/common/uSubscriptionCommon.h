#ifndef _H_USUBSCRIPTION_COMMON_H_
#define _H_USUBSCRIPTION_COMMON_H_

#include <unordered_map>
#include <uri.pb.h>
#include <uprotocol-cpp/uri/serializer/LongUriSerializer.h>

using namespace uprotocol::v1;

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
    {"uprotocol.core.usubscription.v3.ResetRequest",                Request::RESET_REQUEST}
};

    /**
     * URI for sending usubscription updates.
     */
    static UUri uSubUpdatesUri = ::uprotocol::uri::LongUriSerializer::deserialize("/core.usubscription/3/subscriptions#Update");

}

//core.usubscription.subscribe
static UUri uSubRequestsUri;  //= uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(),
                                //                                   uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                  //                                 uprotocol::uri::UResource::forRpcRequest("subscribe"));
//uSub->streamer - rpc regex
static UUri uSubRemoteRequestsUri; //= uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(),
                                    //                                     uprotocol::uri::UEntity::longFormat("core.streamer"),
                                      //                                   uprotocol::uri::UResource::forRpcRequest("subscribe")); /* TBD */

#endif /* _USUBSCRIPTION_COMMON_H_ */