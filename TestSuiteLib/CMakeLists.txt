cmake_minimum_required(VERSION 3.16)

project(TestSuiteLib LANGUAGES CXX)

include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Xml Test LinguistTools)

set(TARGET TestSuiteLib)

qt_add_library(${TARGET} STATIC
	AdsInputController.cpp
	AdsInputController.h
	Control.cpp
	Control.h
	RunControl.cpp
	RunControl.h
	TestControl.cpp
	TestControl.h
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
	TestObserver.cpp
	TestObserver.h
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

target_precompile_headers(${TARGET} PRIVATE
	<chrono>

	<QApplication>
	<QBuffer>
	<QByteArray>
	<QDir>
	<QFile>
	<QJSEngine>
	<QJSValueIterator>
	<QReadLocker>
	<QRegularExpression>
	<QSettings>
	<QTimer>
	<QVariant>
	<QWriteLocker>
	<QXmlStreamWriter>

	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"
	#"../CommonLib/AfbParamValue.h"
	
	#"../Proto/ProtoSerialization.h"
	"../Proto/Network.pb.h"	
	#"../Proto/AppSignal.pb.h"
	
	#"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
	#"../AppSignalLib/TuningValue.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Qml Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib)

target_compile_definitions(${TARGET} PRIVATE TEST_SUITE_LIBRARY)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
