cmake_minimum_required(VERSION 3.16)

project(DiagStateLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Concurrent Qml)

set(TARGET DiagStateLib)

qt_add_library(${TARGET} STATIC
	DiagSignalStateFlags.cpp
	DiagSignalStateFlags.h
	DiagState.cpp
	DiagState.h
	IDiagStateManager.h
	SimpleDiagSignalState.cpp
	SimpleDiagSignalState.h
)

target_precompile_headers(${TARGET} PUBLIC
	<algorithm>
	<array>
	<atomic>
	<cassert>
	<chrono>
	<cmath>
	<concepts>
	<condition_variable>
	<cstdint>
	<functional>
	<limits>
	<list>
	<map>
	<memory>
	<queue>
	<set>
	<type_traits>
	<unordered_map>
	<utility>
	<vector>

	# Qt includes
	#
	<QDateTime>
	<QDebug>
	<QElapsedTimer>
	<QFile>
	<QHostAddress>
	<QMetaEnum>
	<QMultiHash>
	<QRegularExpression>
	<QVariant>

	# Project includes
	#
	#"../Proto/ProtoSerialization.h"
	#"../Proto/serialization.pb.h"
	"../Proto/network.pb.h"
	"../CommonLib/Types.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf HardwareLib UtilsLib)

target_compile_definitions(${TARGET} PRIVATE DIAG_STATE_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)

