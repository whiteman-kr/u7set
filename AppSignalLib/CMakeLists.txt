cmake_minimum_required(VERSION 3.16)

project(AppSignalLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

set(TARGET AppSignalLib)

qt_add_library(${TARGET} STATIC
	AppSignalStateFlags.h
	AppSignalStateFlags.cpp
	ComparatorSet.h
	ComparatorSet.cpp
	IAppSignalManager.h
	ITuningSignalManager.h
	ITuningSignalUpdater.h
	AppSignal.h
	AppSignal.cpp
	AppSignalParam.h
	AppSignalParam.cpp
	TuningSignalManager.h
	TuningSignalManager.cpp
	RecentUsed.h
	RecentUsed.cpp
	TuningValue.h
	TuningValue.cpp
	TuningSignalState.h
	TuningSignalState.cpp
	SimpleAppSignalState.h
	SimpleAppSignalState.cpp
)

target_link_libraries(${TARGET} PUBLIC Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib)

target_compile_definitions(${TARGET} PRIVATE APP_SIGNAL_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)

