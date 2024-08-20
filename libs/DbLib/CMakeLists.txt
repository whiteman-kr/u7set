cmake_minimum_required(VERSION 3.16)

project(DbLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Sql Concurrent Network Qml)

set(TARGET DbLib)

qt_add_library(${TARGET} STATIC
	include/DbLib/DbController.h
	include/DbLib/DbStruct.h
	include/DbLib/DbObjectStorage.h
	include/DbLib/DbControllerTools.h

	DbController.cpp
	DbControllerTools.cpp
	DbProgress.cpp
	DbProgress.h
	DbProgressDialog.cpp
	DbProgressDialog.h
	DbStruct.cpp
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

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	
	<ProtoSerialization.h>
	<AppSignal.pb.h>
	
	"../../AppSignalLib/AppSignal.h"
)

target_compile_definitions(${TARGET} PRIVATE DB_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib HardwareLib AppSignalLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
