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

target_precompile_headers(${TARGET} PUBLIC
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
	<QQmlEngine>
	<QtEndian>
	<QTime>
	<QUuid>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	<QtConcurrent>

	# Project includes
	#
	"../Proto/ProtoSerialization.h"
	"../Proto/serialization.pb.h"
	"../Proto/network.pb.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib)

target_compile_definitions(${TARGET} PRIVATE HARDWARE_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
