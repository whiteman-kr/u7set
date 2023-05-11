cmake_minimum_required(VERSION 3.16)

project(AppSignalLib LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

add_library(AppSignalLib STATIC
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
	TuningValue.h
	TuningValue.cpp
	TuningSignalState.h
	TuningSignalState.cpp
	SimpleAppSignalState.h
	SimpleAppSignalState.cpp
)

target_link_libraries(AppSignalLib PUBLIC Qt6::Network)
target_link_libraries(AppSignalLib PUBLIC CommonLib Protobuf UtilsLib)

target_compile_definitions(AppSignalLib PRIVATE APP_SIGNAL_LIB_DOMAIN)
#target_include_directories(AppSignalLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

