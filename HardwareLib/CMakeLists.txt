cmake_minimum_required(VERSION 3.16)

project(HardwareLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Concurrent Qml)

add_library(HardwareLib STATIC
	Afb.cpp
	Afb.h
	Connection.cpp
	Connection.h
	ConnectionsInfo.h
	ConnectionsInfo.cpp
	DataProtocols.cpp
	DataProtocols.h
	DeviceObject.cpp
	DeviceObject.h
	LmDescription.cpp
	LmDescription.h
	LogicModulesInfo.cpp
	LogicModulesInfo.h
	LanControllerInfo.h
	LanControllerInfo.cpp
	ModuleFirmware.cpp
	ModuleFirmware.h
	ScriptDeviceObject.cpp
	ScriptDeviceObject.h
	Subsystem.cpp
	Subsystem.h
)

target_link_libraries(HardwareLib PRIVATE Qt6::Core Qt6::Network Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(HardwareLib PUBLIC Protobuf)

target_compile_definitions(HardwareLib PRIVATE HARDWARE_LIBRARY)
target_include_directories(HardwareLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(HARDWARE_LIB_DOMAIN)		# TODO: HARDWARE_LIB_DOMAIN -> HARDWARE_LIBRARY?


