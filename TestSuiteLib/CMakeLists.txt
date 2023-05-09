cmake_minimum_required(VERSION 3.16)

project(TestSuiteLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Xml Test)

add_library(TestSuiteLib STATIC
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

target_precompile_headers(TestSuiteLib PRIVATE
	<QtQml>
	<QtXml>
	<QtTest>
)


target_link_libraries(TestSuiteLib PUBLIC Qt6::Qml Qt6::Xml Qt6::Test)
target_link_libraries(TestSuiteLib PUBLIC CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib)

target_compile_definitions(TestSuiteLib PRIVATE TEST_SUITE_LIBRARY)
target_include_directories(TestSuiteLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
