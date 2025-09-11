cmake_minimum_required(VERSION 3.16)

project(UtilsTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets)

set(TARGET UtilsTests)

add_executable(${TARGET}
	main.cpp
	SimpleThreadTests.cpp
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

	"../../UtilsLib/SimpleThread.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET} PRIVATE UtilsLib)

add_test(NAME ${TARGET} COMMAND UtilsTests "--gtest_filter=-SimpleThreadTests.*")

add_code_coverage_options(${TARGET} CODE_COVERAGE)
