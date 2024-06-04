cmake_minimum_required(VERSION 3.16)

project(AdsBridgeExample
	LANGUAGES CXX 
	DESCRIPTION "AppDataService Bridge Example"
	HOMEPAGE_URL "radiy.com")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(AdsBridgeExample
	src/main.cpp
)

# These are the paths to the AdsBridge library used only for the standalone build.
# This can happen if you build this example from the distribution package.
#
target_include_directories(AdsBridgeExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if (WIN32)
	set(ADSBRIDGE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()	
if (UNIX)
	set(ADSBRIDGE_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
endif()

target_link_directories(AdsBridgeExample PRIVATE ${ADSBRIDGE_LIB_DIR})
target_link_libraries(AdsBridgeExample PRIVATE AdsBridge)

if (TARGET AdsBridge)
	add_dependencies(AdsBridgeExample AdsBridge)
endif()
