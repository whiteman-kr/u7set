cmake_minimum_required(VERSION 3.28)

project(TestSuiteLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Xml Test LinguistTools)

set(TARGET TestSuiteLib)

qt_add_library(${TARGET} STATIC
	./include/TestSuiteLib/Control.h
	./include/TestSuiteLib/ControlParams.h
	./include/TestSuiteLib/ControlStatus.h
	./include/TestSuiteLib/IInputController.h
	./include/TestSuiteLib/IOutputController.h
	./include/TestSuiteLib/IScriptProvider.h
	./include/TestSuiteLib/MatsTestSuite.h
	./include/TestSuiteLib/ScriptInfo.h
	./include/TestSuiteLib/ScriptRunner.h
	./include/TestSuiteLib/TestControl.h
	./include/TestSuiteLib/TestController.h
	./include/TestSuiteLib/TestLog.h
	./include/TestSuiteLib/TestReport.h
	./include/TestSuiteLib/TestScript.h
	./include/TestSuiteLib/TestScriptSelection.h
	./include/TestSuiteLib/TestScriptsStorage.h
	./include/TestSuiteLib/TestSuite.h
	./include/TestSuiteLib/TestSuiteConfigController.h
	./include/TestSuiteLib/TestSuiteSettings.h
	./src/AdsInputController.cpp
	./src/AdsInputController.h
	./src/Control.cpp
	./src/MatsTestSuite.cpp
	./src/RunControl.cpp
	./src/RunControl.h
	./src/ScriptInfo.cpp
	./src/ScriptRunner.cpp
	./src/ScriptRunnerImpl.cpp
	./src/ScriptRunnerImpl.h
	./src/ScriptTestLog.cpp
	./src/ScriptTestLog.h
	./src/ScriptTestSuiteApplication.cpp
	./src/ScriptTestSuiteApplication.h
	./src/TestControl.cpp
	./src/TestController.cpp
	./src/TestLog.cpp
	./src/TestObserver.cpp
	./src/TestObserver.h
	./src/TestReport.cpp
	./src/TestScript.cpp
	./src/TestScriptSelection.cpp
	./src/TestScriptsStorage.cpp
	./src/TestSuite.cpp
	./src/TestSuiteConfigController.cpp
	./src/TestSuiteSettings.cpp
	./src/TunsOutputController.cpp
	./src/TunsOutputController.h
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	
	#<ProtoSerialization.h>
	#<AppSignal.pb.h>
	<Network.pb.h>
	
	#"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	#"../../AppSignalLib/TuningValue.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Qml Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib AppSignalLib OnlineLib ClientLib VFrame30 ReportLib)

target_compile_definitions(${TARGET} PRIVATE TEST_SUITE_LIBRARY)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
