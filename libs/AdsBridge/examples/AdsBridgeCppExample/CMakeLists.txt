cmake_minimum_required(VERSION 3.16)

project(AdsBridgeExampleCpp
	LANGUAGES CXX 
	DESCRIPTION "AppDataService Bridge Example"
	HOMEPAGE_URL "radiy.com")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TARGET AdsBridgeCppExampleCpp)

add_executable(${TARGET}
	src/main.cpp
)

# Set the path to the AdsBridge library used for the standalone build.
#
set(ADS_BRIDGE_INLCUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set(ADS_BRIDGE_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

target_include_directories(${TARGET} PRIVATE ${ADS_BRIDGE_INLCUDE_DIR})
target_link_directories(${TARGET} PRIVATE ${ADS_BRIDGE_BIN_DIR})
target_link_libraries(${TARGET} PRIVATE AdsBridge)
