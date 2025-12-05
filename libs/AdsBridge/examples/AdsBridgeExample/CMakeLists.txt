cmake_minimum_required(VERSION 3.21)

project(AdsBridgeExample
	LANGUAGES CXX C
	DESCRIPTION "AppDataServiceBridge Example"
	HOMEPAGE_URL "radiy.com"
)

find_package(AdsBridge CONFIG REQUIRED)

add_executable(AdsBridgeExample
	src/dump.cpp
	src/dump.hpp
	src/main.cpp
)

target_compile_features(AdsBridgeExample PUBLIC cxx_std_20)
	
set_target_properties(AdsBridgeExample PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_link_libraries(AdsBridgeExample PRIVATE Radiy::AdsBridge)