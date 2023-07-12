cmake_minimum_required(VERSION 3.16)

project(TestSuiteLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Xml Test)

set(TARGET TestSuiteLib)

qt_add_library(${TARGET} STATIC
	AdsInputController.cpp
	AdsInputController.h
	Control.cpp
	Control.h
	IInputController.h
	IOutputController.h
	ScriptRunner.cpp
	ScriptRunner.h
	ScriptTestLog.cpp
	ScriptTestLog.h
	TestController.cpp
	TestController.h
	TestLog.cpp
	TestLog.h
	TestReport.cpp
	TestReport.h
	TestScriptsStorage.cpp
	TestScriptsStorage.h
	TestSuite.cpp
	TestSuite.h
	TestSuiteConfigController.cpp
	TestSuiteConfigController.h
	TestSuiteSettings.cpp
	TestSuiteSettings.h
	TunsOutputController.cpp
	TunsOutputController.h
)

target_precompile_headers(${TARGET} PUBLIC
	<QtQml>
	<QtXml>
	<QtTest>
)


target_link_libraries(${TARGET} PUBLIC Qt6::Qml Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib)

target_compile_definitions(${TARGET} PRIVATE TEST_SUITE_LIBRARY)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
