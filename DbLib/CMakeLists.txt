cmake_minimum_required(VERSION 3.16)

project(DbLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Sql Concurrent Network Qml)

set(TARGET DbLib)

qt_add_library(${TARGET} STATIC
	DbController.cpp
	DbController.h
	DbControllerTools.cpp
	DbControllerTools.h
	DbObjectStorage.h
	DbProgress.cpp
	DbProgress.h
	DbProgressDialog.cpp
	DbProgressDialog.h
	DbStruct.cpp
	DbStruct.h
	DbWorker.cpp
	DbWorker.h
)

target_precompile_headers(${TARGET} PRIVATE
	<QCoreApplication>
	<QGuiApplication>
	<QJSEngine>
	<QJSValueIterator>
	<QMessageBox>
	<QThread>
	<QVariant>

	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"
	#"../CommonLib/AfbParamValue.h"
	
	"../Proto/ProtoSerialization.h"
	#"../Proto/Network.pb.h"	
	"../Proto/AppSignal.pb.h"
	
	"../AppSignalLib/AppSignal.h"
	#"../AppSignalLib/AppSignalParam.h"
	#"../AppSignalLib/AppSignalState.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib HardwareLib AppSignalLib)

target_compile_definitions(${TARGET} PRIVATE DB_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
