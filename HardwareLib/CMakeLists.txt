cmake_minimum_required(VERSION 3.16)

project(HardwareLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Concurrent Qml)

set(TARGET HardwareLib)

qt_add_library(${TARGET} STATIC
	Afb.cpp
	Afb.h
	Connection.cpp
	Connection.h
	ConnectionsInfo.cpp
	ConnectionsInfo.h
	DataProtocols.cpp
	DataProtocols.h
	DeviceObject.cpp
	DeviceObject.h
	DiagSignal.cpp
	DiagSignal.h
	DiagSignalType.cpp
	DiagSignalType.h
	HardwareLibrary.cpp
	HardwareLibrary.h
	LanControllerInfo.cpp
	LanControllerInfo.h
	LmDescription.cpp
	LmDescription.h
	LogicModulesInfo.cpp
	LogicModulesInfo.h
	ModuleFirmware.cpp
	ModuleFirmware.h
	PropertyNames.cpp
	PropertyNames.h
	ScriptDeviceObject.cpp
	ScriptDeviceObject.h
	Subsystem.cpp
	Subsystem.h
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
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib)

target_compile_definitions(${TARGET} PRIVATE HARDWARE_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
