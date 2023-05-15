cmake_minimum_required(VERSION 3.16)

project(ClientTests LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets)

set(TARGET_NAME ClientTests)

add_executable(${TARGET_NAME}
	AdsConnectionTests.cpp
	AdsSourceStateConnectionTests.cpp
	AppSignalManagerTests.cpp
	ConfigControllerTests.cpp
	ConnectionPorts.cpp
	ConnectionPorts.h
	main.cpp
	TuningConnectionTests.cpp
	TuningSignalManagerTests.cpp
	TuninsUserManagerTests.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_precompile_headers(${TARGET_NAME} PRIVATE
	<algorithm>
	<chrono>
	<bit>
	<vector>
	<list>
	<ranges>
	<mutex>
	<tuple>
	<condition_variable>

	<gtest/gtest.h>
	<gmock/gmock.h>
	<gmock/gmock-matchers.h>

	<QtGlobal>
	<QDebug>
	<QSignalSpy>
)

target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Test Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET_NAME} PRIVATE ClientLib Simulator)

