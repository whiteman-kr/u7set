cmake_minimum_required(VERSION 3.25)

project(TestSuiteLibUnitTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})

find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent Gui Widgets)

set(TARGET TestSuiteLibUnitTests)

add_executable(${TARGET}
	./src/main.cpp
	./src/MockTestObserver.h
	./src/MockTestObserver.h
	./src/TestLogTests.cpp
	./src/TestSuiteTests.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<gtest/gtest.h>
	<gmock/gmock.h>
	<gmock/gmock-matchers.h>

	<QtGlobal>
	<QDebug>
	<QSignalSpy>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	#"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	#"../../AppSignalLib/TuningValue.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Threads::Threads GTest::gtest_main GTest::gmock)
target_link_libraries(${TARGET} PRIVATE TestSuiteLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)