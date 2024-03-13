cmake_minimum_required(VERSION 3.16)

project(u7databasetests LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Sql Network Widgets Xml Qml)
qt_standard_project_setup()

set(TARGET u7databasetests)

add_executable(${TARGET}
	main.cpp
	DbControllerFileManagementTests.cpp
	DbControllerFileManagementTests.h
	DbControllerHardwareConfigurationTests.cpp
	DbControllerHardwareConfigurationTests.h
	DbControllerProjectTests.cpp
	DbControllerProjectTests.h
	DbControllerSignalManagementTests.cpp
	DbControllerSignalManagementTests.h
	DbControllerUserTests.cpp
	DbControllerUserTests.h
	DbControllerVersionControlTests.cpp
	DbControllerVersionControlTests.h
	DeviceObjectTests.cpp
	DeviceObjectTests.h
	FileTests.cpp
	FileTests.h
	OtherTests.cpp
	OtherTests.h
	ProjectPropertyTests.cpp
	ProjectPropertyTests.h
	PropertyObjectTests.cpp
	PropertyObjectTests.h
	Settings.cpp
	Settings.h
	TestDbBase.cpp
	TestDbBase.h
	UserPropertyTest.cpp
	UserPropertyTest.h
	UserTests.cpp
	UserTests.h

	../../DatabaseUpgrade/DatabaseUpgrade.qrc
	FutureDatabaseUpgrade.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	<memory>

	<QDebug>
	<QFile>
	<QSqlError>
	<QSqlQuery>
	<QThread>

	"../../CommonLib/Types.h"
	"../../CommonLib/Hash.h"
	"../../CommonLib/PropertyObject.h"
	"../../CommonLib/AfbParamValue.h"
	"../../CommonLib/DebugInstCounter.h"
	"../../CommonLib/Factory.h"
	"../../Proto/ProtoSerialization.h"
	"../../Proto/AppSignal.pb.h"
	"../../AppSignalLib/AppSignal.h"
	
	<DbLib/DbController.h>
	"../../libs/DbLib/DbWorker.h"  # include private libraray part for testing reaon 
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Qt6::Core Qt6::Sql Qt6::Network Qt6::Widgets Qt6::Xml Qt6::Qml)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib DbLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
