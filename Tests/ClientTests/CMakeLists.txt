cmake_minimum_required(VERSION 3.16)

project(ClientTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets)

set(TARGET ClientTests)

add_executable(${TARGET}
	AdsConnectionTests.cpp
	AdsSourceStateConnectionTests.cpp
	AppSignalManagerTests.cpp
	ConfigControllerTests.cpp
	ConnectionPorts.cpp
	ConnectionPorts.h
	main.cpp
	RtDataProviderTests.cpp
	TuningConnectionTests.cpp
	TuningSignalManagerTests.cpp
	TuninsUserManagerTests.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<bit>
	<deque>
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

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	
	<Network.pb.h>
	
	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET} PRIVATE ClientLib SimulatorLib)

add_test(NAME ${TARGET} COMMAND ClientTests "--gtest_filter=-AdsConnectionTests.*:ConfigControllerTests.*:AdsSourceStateConnectionTests.*:TuningConnectionTests.*:RtDataProviderTests.*:TuningUserManagerTests.loginTests:TuningUserManagerTests.checkPasswordsTest")

add_code_coverage_options(${TARGET} CODE_COVERAGE)
