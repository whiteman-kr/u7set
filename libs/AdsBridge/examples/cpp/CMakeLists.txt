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

target_link_directories(AdsBridgeExample PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
target_link_libraries(AdsBridgeExample PRIVATE AdsBridge)

add_dependencies(AdsBridgeExample AdsBridge)
