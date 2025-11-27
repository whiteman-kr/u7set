cmake_minimum_required(VERSION 3.16)

project(AppSignalLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

set(TARGET AppSignalLib)

qt_add_library(${TARGET} STATIC
	AppSignal.cpp
	AppSignal.h
	AppSignalParam.cpp
	AppSignalParam.h
	AppSignalSpecPropValues.cpp
	AppSignalSpecPropValues.h
	AppSignalState.cpp
	AppSignalState.h
	AppSignalStateFlags.cpp
	AppSignalStateFlags.h
	Bus.cpp
	Bus.h
	ComparatorSet.cpp
	ComparatorSet.h
	DiscretesLogRecord.cpp
	DiscretesLogRecord.h
	IAppSignalManager.h
	ISignalManager.h
	ITuningSignalManager.h
	PropertyNames.cpp
	PropertyNames.h
	RecentUsed.cpp
	RecentUsed.h
	SimpleAppSignalState.cpp
	SimpleAppSignalState.h
	TuningDataStorage.cpp
	TuningDataStorage.h
	TuningSignalState.cpp
	TuningSignalState.h
	TuningValue.cpp
	TuningValue.h
)

target_precompile_headers(${TARGET} PRIVATE
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
	<version>

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
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>
	<Hardware.pb.h>
	<Network.pb.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib AppSignalLibStd)

target_compile_definitions(${TARGET} PRIVATE APP_SIGNAL_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)

