cmake_minimum_required(VERSION 3.16)

project(AppSignalLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

target_link_libraries(AppSignalLib PRIVATE Qt6::Network)
target_link_libraries(AppSignalLib PUBLIC Protobuf)

target_compile_definitions(AppSignalLib PRIVATE APPSIGNAL_LIBRARY)
#target_include_directories(AppSignalLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(APP_SIGNAL_LIB_DOMAIN)		# TODO: APP_SIGNAL_LIB_DOMAIN -> APPSIGNAL_LIBRARY?
