#include <uprotocol-cpp-ulink-zenoh/transport/zenohUTransport.h>
#include <uprotocol-cpp/uuid/serializer/UuidSerializer.h>
#include <uprotocol-cpp/uri/serializer/LongUriSerializer.h>
#include <uprotocol-cpp/uri/datamodel/UResource.h>
#include <uprotocol-cpp/transport/datamodel/UAttributes.h>
#include <uprotocol-cpp/transport/datamodel/UPayload.h>
#include <src/main/proto/ustatus.pb.h>
#include <uprotocol-cpp-ulink-zenoh/session/zenohSessionManager.h>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;

// initialize test UURIs used in test
UAuthority g_uAuthorityLocal = UAuthority::local();
UAuthority g_uAuthorityRemote = UAuthority::longRemote("VCU", "MY_VIN");
UEntity g_uEntity = UEntity::longFormat("example.service", 1);
UResource g_uResource = UResource::longFormat("example_instance", "example_name", "");
UResource g_uResourceRPC = UResource::forRpcRequest("Example Method");
auto const g_testUUri = UUri(g_uAuthorityLocal, g_uEntity, g_uResource);
auto const g_testUUri2 = UUri(g_uAuthorityRemote, g_uEntity, g_uResource);
auto const g_testRpcUUri= UUri(g_uAuthorityRemote, g_uEntity, g_uResourceRPC);
auto const g_testRpcUUri2= UUri(g_uAuthorityLocal, g_uEntity, g_uResourceRPC);

// Initialize test UListener
class TestListener : public UListener {
    UStatus onReceive(const uprotocol::uri::UUri &uri, const UPayload &payload, const UAttributes &attributes) const {
        UStatus status;
        status.set_code(UCode::OK);
        return status;
    }
};

TestListener testListener;

// Test case to verify that the initialization of ZenohUTransport fails and returns UCode::UNAVAILABLE.
TEST(RegisterListenerTest, initFails) {
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testUUri,testListener).code(), UCode::UNAVAILABLE);
}

// Test case to verify the successful registration of a listener using ZenohUTransport.
TEST(RegisterListenerTest, registerListenerHappyPath) {
    ZenohUTransport::instance().init();
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testUUri,testListener).code(), UCode::OK);
    ZenohUTransport::instance().unregisterListener(g_testUUri,testListener);
}

// Test case to verify the successful registration of multiple listeners using ZenohUTransport.
TEST(RegisterListenerTest, multipleListeners) {
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testUUri,testListener).code(), UCode::OK);
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testUUri2,testListener).code(), UCode::OK);
    ZenohUTransport::instance().unregisterListener(g_testUUri,testListener);
    ZenohUTransport::instance().unregisterListener(g_testUUri2,testListener);
}

// Test case to verify the successful registration of a listener for an RPC method using ZenohUTransport.
TEST(RegisterListenerTest, registerListenerRpcMethod) {
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testRpcUUri,testListener).code(), UCode::OK);
    ZenohUTransport::instance().unregisterListener(g_testRpcUUri,testListener);
}

// Test case to verify that registering the same listener twice for the same URI returns UCode::INVALID_ARGUMENT.
TEST(RegisterListenerTest, sameListenerTwice) {
    ZenohUTransport::instance().registerListener(g_testUUri,testListener);
    EXPECT_EQ(ZenohUTransport::instance().registerListener(g_testUUri,testListener).code(), UCode::INVALID_ARGUMENT);
    ZenohUTransport::instance().unregisterListener(g_testUUri,testListener);
}

void threadFunc(const UUri &uri, const UListener &listener, UCode& res) {
    res = ZenohUTransport::instance().registerListener(uri,listener).code();
}

// Test case to verify the registration of listeners in a multithreaded environment using ZenohUTransport.
TEST(RegisterListenerTest, multithreaded) {
    UCode res; 
    UCode res2;
    std::thread t1(threadFunc, g_testUUri, testListener, std::ref(res));
    std::thread t2(threadFunc, g_testUUri, testListener, std::ref(res2));
    t1.join();
    EXPECT_EQ(res, UCode::OK);
    t2.join();
    EXPECT_EQ(res2, UCode::OK);
}

// Test case to verify the successful unregistering of a listener using ZenohUTransport.
TEST(RegisterListenerTest, unregisterListenerHappyPath) {
    ZenohUTransport::instance().registerListener(g_testUUri,testListener);
    EXPECT_EQ(ZenohUTransport::instance().unregisterListener(g_testUUri,testListener).code(), UCode::OK);
}

// Test case to verify the successful unregistering of multiple listeners using ZenohUTransport.
TEST(RegisterListenerTest, unregisterMultipleListeners) {
    ZenohUTransport::instance().registerListener(g_testUUri,testListener);
    ZenohUTransport::instance().registerListener(g_testUUri2,testListener);
    EXPECT_EQ(ZenohUTransport::instance().unregisterListener(g_testUUri,testListener).code(), UCode::OK);
    EXPECT_EQ(ZenohUTransport::instance().unregisterListener(g_testUUri2,testListener).code(), UCode::OK);
}

// Description: Test case to verify that unregistering a listener for a URI that doesn't exist returns UCode::INVALID_ARGUMENT.
TEST(RegisterListenerTest, listenerDoesntExistForUri) {
    EXPECT_EQ(ZenohUTransport::instance().unregisterListener(g_testRpcUUri2,testListener).code(), UCode::INVALID_ARGUMENT);
}
