cmake_minimum_required(VERSION 3.21)

project(AdsBridgeCppExample
	LANGUAGES CXX C
	DESCRIPTION "AppDataServiceBridge Example"
	HOMEPAGE_URL "radiy.com"
)

find_package(AdsBridge REQUIRED)

add_executable(AdsBridgeCppExample
	src/main.cpp
)

target_compile_features(AdsBridgeCppExample PUBLIC cxx_std_20)
	
set_target_properties(AdsBridgeCppExample PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_link_libraries(AdsBridgeCppExample PRIVATE Radiy::AdsBridge)