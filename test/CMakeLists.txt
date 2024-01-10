#Add the new file name under "add_executable" to include any new test files.
cmake_minimum_required(VERSION 3.20.1)
add_definitions(-DNDEBUG)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../uprotocol-cpp/)
    #Add Dependencies for test. These are the actual test files.
    add_executable(unit_tests
    unit_test/zenohUTransport_test.cpp
    unit_test/zenohUTransportSendReceive_test.cpp
    ../src/zenohUTransport.cpp
    )
    #Add gtest/gmock files
    set(gtest_libs gtest_main)
    find_library(gtest_main libgtest_main.so)
    add_library(gtest_main SHARED IMPORTED)
    set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${gtest_main})
    set(gmock_libs gmock_main)
    find_library(gmock_main libgmock_main.so)
    add_library(gmock_main SHARED IMPORTED)
    set_property(TARGET gmock_main PROPERTY IMPORTED_LOCATION ${gmock_main})
    message("${this_lib}/CMakeLists.txt : Linking google test libraries...")
    #To include another cmake for dependencies: 
    #    Include in other cmake: set(this_lib variablename). Ex: set(this_lib ustreamer)
    #    Include in this cmake: variablename. Ex: ustreamer
    target_link_libraries(unit_tests PUBLIC
        gtest
        gmock
        ${gtest_libs}
        ${gmock_libs}  
        uprotocol-cpp-ulink-zenoh 
        uprotocol-cpp
        /usr/local/lib64/libzenohc.so    
        fmt
    )
 
    #Directories for any required files.
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/shmem
        ${CMAKE_CURRENT_SOURCE_DIR}/../demo/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/shmem
    )
    #Set for Test Filtering
    #Format: "Test Suite"."TestName"
    #Example: StreamerTest.AssertInitSuccess , StreamerTest.* (Runs all tests in this suite)
    set (TEST_FILTERING false)
