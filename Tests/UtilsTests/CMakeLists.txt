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

	Common.h
	DiscretesLogTests.cpp
	main.cpp
	SimpleThreadTests.cpp
	SpinLockTests.cpp

	../../AppDataService/DiscretesLog.cpp
	../../AppDataService/DiscretesLog.h
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

target_link_libraries(${TARGET} PRIVATE Qt6::Test Threads::Threads GTest::gtest GTest::gmock Qt6::Sql)
target_link_libraries(${TARGET} PRIVATE UtilsLib ClientLib CommonLib OnlineLib)

#add_test(NAME ${TARGET} COMMAND UtilsTests "--gtest_filter=-SimpleThreadTests.*")

add_code_coverage_options(${TARGET} CODE_COVERAGE)
