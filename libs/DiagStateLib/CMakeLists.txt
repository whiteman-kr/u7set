cmake_minimum_required(VERSION 3.28)

project(DiagStateLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Concurrent Qml)

set(TARGET DiagStateLib)

qt_add_library(${TARGET} STATIC
	./include/DiagStateLib/DiagSignalStateFlags.h
	./include/DiagStateLib/DiagState.h
	./include/DiagStateLib/IDiagStateManager.h
	./include/DiagStateLib/SimpleDiagSignalState.h

	DiagSignalStateFlags.cpp
	DiagState.cpp
	SimpleDiagSignalState.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<array>
	<atomic>
	<cassert>
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
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
)

target_compile_definitions(${TARGET} PRIVATE DIAG_STATE_LIB_DOMAIN)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Xml Qt6::Concurrent Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx HardwareLib UtilsLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)

