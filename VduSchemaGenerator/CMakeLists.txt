cmake_minimum_required(VERSION 3.16)

project(VduSchemaGenerator LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(TARGET VduSchemaGenerator)

qt_add_library(${TARGET} STATIC
	VduSchemaGenerator.cpp
	VduSchemaGenerator.h
)

target_precompile_headers(${TARGET} PUBLIC
	<QByteArray>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core)
target_link_libraries(${TARGET} PUBLIC VFrame30)

target_compile_definitions(${TARGET} PRIVATE VDUSCHEMAGENERATOR_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
