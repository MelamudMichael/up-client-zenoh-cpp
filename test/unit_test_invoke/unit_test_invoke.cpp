#include <uprotocol-cpp-ulink-zenoh/transport/zenohUTransport.h>
#include <uprotocol-cpp-ulink-zenoh/rpc/zenohRpcClient.h>
#include <uprotocol-cpp/uuid/factory/Uuidv8Factory.h>
#include <gtest/gtest.h>
#include <chrono>
#include <csignal>

using namespace std;
using namespace uprotocol::utransport;
using namespace uprotocol::uri;
using namespace uprotocol::uuid;
using namespace uprotocol::v1;

int preConditions(){
    	if (UCode::OK != ZenohRpcClient::instance().init().code())
    	{
        	spdlog::error("ZenohRpcClient::instance().init failed");
        	return -1;
    	}
	return 0;
}

/*
 * @brief sendRPC method which calls the invokeMethod
 */

UPayload sendRPC(UUri &uri, UMessageType type,UPriority priority,const int bufferSize)
{
    auto uuid = Uuidv8Factory::create();         
    UAttributesBuilder builder(uuid, type,priority);
    UAttributes attributes = builder.build();
    uint8_t buffer[bufferSize];
    UPayload payload(buffer,
                     sizeof(buffer),
                     UPayloadType::VALUE);
    std::future<UPayload> result = ZenohRpcClient::instance().invokeMethod(uri,payload,attributes);
    
    if (false == result.valid()) {
        spdlog::error("future is invalid");
        return UPayload(nullptr, 0, UPayloadType::UNDEFINED);   
    }

    result.wait();
    return result.get();
}
/*
 * @brief test for happy path 
 */
int testInvokeHappyPath(){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,1);
        if (nullptr != response.data()){
            return 0;
        }
        sleep(1);
    }
    return -1;	
}
/*
 * @brief test for time out, this requires the remote server app not running
 */
int testInvokeTimeOut(){
    return(testInvokeHappyPath());
}
/*
 * @brief test for invalid uri
 */
int testInvokeInvalidUri(){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcInvalidUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("get")); // invalid URI
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcInvalidUri,type,priority,1);
        if (nullptr != response.data()){
            return 0;
        }
        sleep(1);
    }
    return -1;	
}
/*
 * @brief test for invalid Message Type
 */

int testInvokeInvalidMsgType(UMessageType type){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,1);
        if (nullptr != response.data()){
            return 0;
        }
        sleep(1);
    }
    return -1;	
}
/*
 * @brief test for invalid priority
 */
int testInvokeInvalidPriority(){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcInvalidUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::UNDEFINED; //Invalid priority
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcInvalidUri,type,priority,1);
        if (nullptr != response.data()){
            return 0;
        }
        sleep(1);
    }
    return -1;	
}
/*
 * @brief test for large buffer
 */
int testInvokeLargeBuffer(){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,100000); //large buffer size
        if (nullptr != response.data()){
            return 0;
        }
        sleep(1);
    }
    return -1;	
}
/*
 * @brief test for correct response everytime
 */
int testInvokecorrectResponseAlways(){
    size_t constexpr maxIterations = 1;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,1);
        if (nullptr == response.data()){
        	return -1;
        }
        sleep(1);
    }
    return 0;	
}
/*
 * @brief test for chekcing what is the minimum response count which acceptable. 
 */
int testInvokeMinResponse(){
    size_t constexpr maxIterations = 10, minResponseCnt = 5;
    int responseCnt=0;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,1);
        if (nullptr == response.data()){
        }else{
		responseCnt++;
        }
        sleep(1);
    }
    if(responseCnt>=minResponseCnt){
    	return 0;
    }
    return -1;
}
/*
 * @brief test for different use format
 */
int testInvokeInvalidUseFormat(){
    size_t constexpr maxIterations = 10;
    auto use = UEntity::longFormat("test.app");//non matching use format
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        auto response = sendRPC(rpcUri,type,priority,1);
        if (nullptr != response.data()){
        	return 0;
        }
        sleep(1);
    }
   return -1;
}
/*
 * @brief test for maximum number of threads , which does not result in crash or any error.
 */
int testInvokeMaxThreads(){
    size_t constexpr maxIterations = 50;
    int responseCnt=0;
    auto use = UEntity::longFormat("test_rpc.app");
    auto rpcUri = UUri(
        UAuthority::local(), use,
        UResource::forRpcRequest("getTime"));
    auto type = UMessageType::REQUEST;
    auto priority = UPriority::STANDARD;
    for (size_t i = 0 ; i < maxIterations ; ++i)
    {
        sendRPC(rpcUri,type,priority,1);
        sleep(1);
    }
   return 0;
}
// @brief Test the invoke method for happy path.
TEST(ZenohRPC, sendHappyPath) {
    
    EXPECT_EQ(testInvokeHappyPath(), 0);
}
// @brief Test the invoke method for time out.
TEST(ZenohRPC, timeOut) {
    
   EXPECT_EQ(testInvokeTimeOut(), -1);
}
// @brief Test the invoke method for invalid URI
TEST(ZenohRPC, invalidUri) {
    
   EXPECT_EQ(testInvokeInvalidUri(), -1);
}
// @brief Test the invoke method for Minimum response
TEST(ZenohRPC,	minResponse) {
    
   EXPECT_EQ(testInvokeMinResponse(), 0);
}
// @brief Test the invoke method for invalid Msg Type
TEST(ZenohRPC, invalidMsgTypePUBLISH) {
    
   EXPECT_EQ(testInvokeInvalidMsgType(UMessageType::PUBLISH), -1);
}
// @brief Test the invoke method for invalid priority Type
TEST(ZenohRPC, invalidPriority) {
    
   EXPECT_EQ(testInvokeInvalidPriority(), -1);
}
// @brief Test the invoke method for large buffer
TEST(ZenohRPC,	largeBuffer) {
    
   EXPECT_EQ(testInvokeLargeBuffer(), -1);
}
// @brief Test the invoke method for correct response always.
TEST(ZenohRPC,	correctResponseAlways) {
    
   EXPECT_EQ(testInvokecorrectResponseAlways(), 0);
}
// @brief Test the invoke method for Maximum threads
TEST(ZenohRPC,	InvalidUseFormat) {
    
   EXPECT_EQ(testInvokeInvalidUseFormat(), -1);
}
// @brief Test the invoke method for Maximum threads
TEST(ZenohRPC,	maxThreads) {
    
   EXPECT_EQ(testInvokeMaxThreads(), 0);
}
// @brief Test the invoke method for invalid Msg Type
TEST(ZenohRPC, invalidMsgTypeUNDEFINED) {
    
   EXPECT_EQ(testInvokeInvalidMsgType(UMessageType::UNDEFINED), -1);
}
int main(int argc, char** argv) 
{
    if(preConditions()!=0){
    	return -1;
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

