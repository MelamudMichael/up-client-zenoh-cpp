#include <uprotocol-cpp-ulink-zenoh/transport/zenohUTransport.h>
#include <uprotocol-cpp/uuid/serializer/UuidSerializer.h>
#include <uprotocol-cpp/uri/datamodel/UResource.h>
#include <uprotocol-cpp/transport/datamodel/UAttributes.h>
#include <uprotocol-cpp/transport/datamodel/UPayload.h>
#include <src/main/proto/ustatus.pb.h>
#include <src/main/proto/uuid.pb.h>
#include <uprotocol-cpp-ulink-zenoh/session/zenohSessionManager.h>
#include <uprotocol-cpp/uuid/factory/Uuidv8Factory.h>
#include <uprotocol-cpp-ulink-zenoh/message/messageBuilder.h>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;

/**
 * Initialize test UURIs.
 */
UAuthority g_sendUAuthorityLocal = UAuthority::local();
UAuthority g_sendUAuthorityRemote = UAuthority::longRemote("VCU", "MY_VIN");
UEntity g_sendUEntity = UEntity::longFormat("example.service", 1);
UResource g_sendUResource = UResource::longFormat("example_instance", "example_name", "");
UResource g_sendUResourceRPC = UResource::forRpcRequest("Example Method");
auto const g_sendTestUURI = UUri(g_sendUAuthorityRemote, g_sendUEntity, g_sendUResource);
auto const g_sendTestRPCUURI= UUri(g_sendUAuthorityRemote, g_sendUEntity, g_sendUResourceRPC);

/**
 * Initializing test attributes and payload.
 */
auto g_uuid = Uuidv8Factory::create(); 
auto g_priority = UPriority::STANDARD;
auto g_publishType = UMessageType::PUBLISH;
auto g_responseType = UMessageType::RESPONSE;
UAttributesBuilder g_builder(g_uuid, g_publishType, g_priority);
UAttributesBuilder g_builder2(g_uuid, g_responseType, g_priority);
UAttributes g_attributes = g_builder.build();
UAttributes g_attributes2 = g_builder2.build();
uint8_t g_buffer[1];
UPayload g_payload(g_buffer, sizeof(g_buffer), UPayloadType::VALUE);

/**
 * Config object to initialize zenohUTransport.
 */
ZenohSessionManagerConfig g_config;

/**
 * Default setup and teardown for zenohUTransportSendReceive_test.
 */
class zenohUTransportSendReceiveTests : public ::testing::Test {
    void SetUp() override {
        ZenohUTransport::instance().init();
    }

    void TearDown() override {
        ZenohUTransport::instance().term();
    }
};

/**
 * Happy path run of send functionality.
 */
TEST_F(zenohUTransportSendReceiveTests, sendHappyPath) {
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes).code(), UCode::OK);
}

/**
 * Test send with an RPC method and an incorrect message type. UCode should be set to UNAVAILABLE.
 */
TEST_F(zenohUTransportSendReceiveTests, sendRPCBadMessageType) {
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestRPCUURI, g_payload, g_attributes).code(),
              UCode::INVALID_ARGUMENT);
}

/**
 * Test send without RPC method and an incorrect message type. UCode should be UNAVAILABLE.
 */
TEST_F(zenohUTransportSendReceiveTests, sendNonRPCBadMessageType) {
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes2).code(), UCode::INVALID_ARGUMENT);
}

/**
 * Test send while ZenohUTransport is marked for termination. Expect UCode OK prior to termination,
 * and UNAVAILABLE after flag. 
 * refCount_ is incremented after every 'init'.
 * So 'term()' may be called twice in order to decrement 'refCount_' correctly.
 */
TEST_F(zenohUTransportSendReceiveTests, sendTerminationUNAVAILABLE) {
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes).code(), UCode::OK);
    EXPECT_EQ(ZenohUTransport::instance().term().code(), UCode::OK);
    if (ZenohUTransport::instance().term().code() == UCode::OK) {
        EXPECT_EQ(ZenohUTransport::instance().term().code(), UCode::OK);
    }
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes).code(), UCode::UNAVAILABLE);
}

/**
 * Test to ensure send is not successful while ZenohUTransport is flagged for termination.
 * Expect UCode to NOT be OK.
 */
TEST_F(zenohUTransportSendReceiveTests, sendTerminationOK) {
    EXPECT_EQ(ZenohUTransport::instance().term().code(), UCode::OK);
    EXPECT_NE(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes).code(), UCode::OK);
}

/**
 * Test send when ZenohUTransport is not initialized. Expect UCode UNAVAILABLE.
 * This test bypasses the SetUp function in order to keep ZenohUTransport uninitialized.
 */
TEST(SendTest, sendinitFails) {
    EXPECT_EQ(ZenohUTransport::instance().send(g_sendTestUURI, g_payload, g_attributes).code(), UCode::UNAVAILABLE);
}

/**
 * Happy path run of receive functionality. Receive is not implemented currently. Will always return UCode UNAVAILABLE.
 */
TEST_F(zenohUTransportSendReceiveTests, ReceiveHappyPath) {
    EXPECT_EQ(ZenohUTransport::instance().receive(g_sendTestUURI, g_payload, g_attributes).code(), UCode::UNAVAILABLE);
}
