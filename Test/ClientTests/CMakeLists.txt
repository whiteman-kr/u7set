cmake_minimum_required(VERSION 3.16)

project(ClientTests LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets)

add_executable(ClientTests
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

target_precompile_headers(ClientTests PRIVATE
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

target_link_libraries(ClientTests PRIVATE Qt6::Test Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(ClientTests PRIVATE ClientLib Simulator)

