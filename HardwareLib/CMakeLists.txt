cmake_minimum_required(VERSION 3.16)

project(HardwareLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Concurrent Qml)

set(TARGET HardwareLib)

qt_add_library(${TARGET} STATIC
	./include/HardwareLib/Afb.h
	./include/HardwareLib/Connection.h
	./include/HardwareLib/ConnectionsInfo.h
	./include/HardwareLib/DataProtocols.h
	./include/HardwareLib/DeviceAppSignal.h
	./include/HardwareLib/DeviceChassis.h
	./include/HardwareLib/DeviceController.h
	./include/HardwareLib/DeviceModule.h
	./include/HardwareLib/DeviceObject.h
	./include/HardwareLib/DeviceRack.h
	./include/HardwareLib/DeviceRoot.h
	./include/HardwareLib/DeviceSystem.h
	./include/HardwareLib/DiagSignal.h
	./include/HardwareLib/DiagSignalType.h
	./include/HardwareLib/EquipmentSet.h
	./include/HardwareLib/HardwareLibrary.h
	./include/HardwareLib/LanControllerInfo.h
	./include/HardwareLib/LmDescription.h
	./include/HardwareLib/LogicModulesInfo.h
	./include/HardwareLib/ModuleFirmware.h
	./include/HardwareLib/PropertyNames.h
	./include/HardwareLib/ScriptDeviceObject.h
	./include/HardwareLib/Software.h
	./include/HardwareLib/Subsystem.h
	./include/HardwareLib/Workstation.h
	Afb.cpp
	Connection.cpp
	ConnectionsInfo.cpp
	DataProtocols.cpp
	DeviceAppSignal.cpp
	DeviceChassis.cpp
	DeviceController.cpp
	DeviceModule.cpp
	DeviceObject.cpp
	DeviceObjectFactory.cpp
	DeviceObjectFactory.h
	DeviceRack.cpp
	DeviceRoot.cpp
	DeviceSystem.cpp
	DiagSignal.cpp
	DiagSignalType.cpp
	EquipmentSet.cpp
	HardwareLibrary.cpp
	LanControllerInfo.cpp
	LmDescription.cpp
	LogicModulesInfo.cpp
	ModuleFirmware.cpp
	PropertyNames.cpp
	ScriptDeviceObject.cpp
	Software.cpp
	Subsystem.cpp
	Workstation.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<cassert>
	<cmath>
	<condition_variable>
	<cstdint>
	<cstdlib>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<optional>
	<ranges>
	<queue>
	<set>
	<type_traits>
	<unordered_map>
	<utility>
	<vector>

	# Qt includes
	#
	<QBitArray>
	<QDate>
	<QDebug>
	<QDomElement>
	<QFile>
	<QFileInfo>
	<QFuture>
	<QHash>
	<QHostAddress>
	<QJSEngine>
	<QJsonArray>
	<QJsonDocument>
	<QJsonObject>
	<QJsonValue>
	<QMap>
	<QMetaObject>
	<QMetaProperty>
	<QObject>
	<QJSEngine>
	<QtEndian>
	<QTime>
	<QUuid>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	<QtConcurrent>
)

target_precompile_headers(${TARGET} PRIVATE
	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"
	#"../CommonLib/AfbParamValue.h"
	
	"../Proto/ProtoSerialization.h"
	"../Proto/Network.pb.h"
	"../Proto/Hardware.pb.h"	
)

target_compile_definitions(${TARGET} PRIVATE HARDWARE_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
