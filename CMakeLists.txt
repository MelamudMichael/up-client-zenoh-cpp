project(uprotocol-cpp-ulink-zenoh LANGUAGES CXX)

include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../uprotocol-cpp/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../uprotocol-core-api/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../uprotocol-core-api/src/main/proto)

# Set the directory to start the recursive search
set(SearchDirectory "${CMAKE_CURRENT_SOURCE_DIR}/../")

# Perform a recursive file search
file(GLOB_RECURSE FOUND_FILES
    LIST_DIRECTORIES false  # Do not include directories in the results
    "${SearchDirectory}/uSubscriptionClientExt.*"  # Pattern to search for, change as needed
)

list(LENGTH FOUND_FILES numFiles)
if(numFiles EQUAL 1) 
    add_definitions(-DUSUBSCRIPTION_CLIENT_EXT)
    list(GET FOUND_FILES 0 USUBSCRIPTION_EXTENSION_FILE)
    get_filename_component(USUBSCRIPTION_EXTENSION_PATH "${USUBSCRIPTION_EXTENSION_FILE}" DIRECTORY)
    include_directories(${USUBSCRIPTION_EXTENSION_PATH})
else()
    message(WARNING "Expected one file, found ${numFiles} files.")
endif()

find_library(ZENOH_LIB zenohc)

find_package(spdlog REQUIRED)

add_definitions(-DSPDLOG_FMT_EXTERNAL)

add_subdirectory(examples)

add_library(
    ${PROJECT_NAME} SHARED 
    src/zenohUTransport.cpp
    src/zenohRpcClient.cpp
    src/zenohSessionManager.cpp
    src/messageBuilder.cpp
    src/messageParser.cpp
    src/uSubscriptionClient.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../uprotocol-core-api/src/main/proto/core/usubscription/v3/usubscription.pb.cc
    /usr/local/lib64/libzenohc.so)

