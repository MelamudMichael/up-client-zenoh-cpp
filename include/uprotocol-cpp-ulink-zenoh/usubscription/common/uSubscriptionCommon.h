#ifndef _H_USUBSCRIPTION_COMMON_H_
#define _H_USUBSCRIPTION_COMMON_H_

#include <unordered_map>
#include <uprotocol-cpp/uri/datamodel/UUri.h>

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
static uprotocol::uri::UUri uSubRequestsUri = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), 
                                                                   uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                                   uprotocol::uri::UResource::forRpcRequest("subscribe"));
//uSub->streamer - rpc regex 
static uprotocol::uri::UUri uSubRemoteRequestsUri = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), 
                                                                         uprotocol::uri::UEntity::longFormat("core.streamer"),
                                                                         uprotocol::uri::UResource::forRpcRequest("subscribe")); /* TBD */

/* URI for sending usubscription updates */ 
static uprotocol::uri::UUri uSubUpdatesUri = uprotocol::uri::UUri(uprotocol::uri::UAuthority::local(), 
                                                                  uprotocol::uri::UEntity::longFormat("core.usubscription"),
                                                                  uprotocol::uri::UResource::forRpcRequest("subscriptions#Update"));
#endif /* _USUBSCRIPTION_COMMON_H_ */