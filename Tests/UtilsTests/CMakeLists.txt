cmake_minimum_required(VERSION 3.16)

project(UtilsTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets Sql)

set(TARGET UtilsTests)

add_executable(${TARGET}

	AppDataReceiverTests.cpp
	Common.cpp
	Common.h
	DiscretesLogTests.cpp
	GrpcAppDataSrvTests.cpp
	GrpcAdsClientTests.cpp
	GrpcCfgLoaderTests.cpp
	GrpcFileSrvTests.cpp
	main.cpp
	RupFrameProducer.h
	SimpleThreadTests.cpp
	SpinLockTests.cpp

	../../AppDataService/AppDataReceiver.cpp
	../../AppDataService/AppDataReceiver.h
	../../AppDataService/AppDataSource.cpp
	../../AppDataService/AppDataSource.h
	../../AppDataService/AppDataSrvTools.cpp
	../../AppDataService/AppDataSrvTools.h
	../../AppDataService/DiscretesLog.cpp
	../../AppDataService/DiscretesLog.h
	../../AppDataService/DynamicAppSignalState.cpp
	../../AppDataService/DynamicAppSignalState.h
	../../AppDataService/GrpcAppDataSrv.cpp
	../../AppDataService/GrpcAppDataSrv.h
	../../AppDataService/RtTrendsServer.cpp
	../../AppDataService/RtTrendsServer.h
	../../AppDataService/SignalStatesProcessingThread.cpp
	../../AppDataService/SignalStatesProcessingThread.h
	../../ConfigurationService/GrpcCfgServer.cpp
	../../ConfigurationService/GrpcCfgServer.h
	../../Metrology/GrpcAdsClient.cpp
	../../Metrology/GrpcAdsClient.h
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<bit>
	<deque>
	<vector>
	<list>
	<ranges>
	<mutex>
	<random>
	<tuple>
	<condition_variable>
	<chrono>
	<type_traits>

	<asio/io_context.hpp>
	<asio/ip/udp.hpp>
	<asio/steady_timer.hpp>

	<gtest/gtest.h>
	<gmock/gmock.h>
	<gmock/gmock-matchers.h>

	<QtGlobal>
	<QDebug>
	<QSignalSpy>
	<QCoreApplication>
	<QFileInfo>
	<QVariant>
	<QThread>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/HostAddressPort.h>

	"../../OnlineLib/SoftwareInfo.h"
	"../../OnlineLib/CircularLogger.h"
	"../../UtilsLib/SimpleThread.h"
	"../../UtilsLib/SpinLock.h"
)

target_include_directories(${TARGET} PRIVATE
	${CMAKE_BINARY_DIR}/libs/GrpcLib
)

target_compile_definitions(${TARGET} PRIVATE UNDER_GTEST=1)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Threads::Threads GTest::gtest GTest::gmock Qt6::Sql)
target_link_libraries(${TARGET} PRIVATE UtilsLib ClientLib CommonLib OnlineLib AppSignalLib GrpcLib)

#add_test(NAME ${TARGET} COMMAND UtilsTests "--gtest_filter=-SimpleThreadTests.*")

add_code_coverage_options(${TARGET} CODE_COVERAGE)
