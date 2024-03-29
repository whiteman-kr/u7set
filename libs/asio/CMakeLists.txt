cmake_minimum_required(VERSION 3.19)

project(asio VERSION 1.24.0 LANGUAGES CXX)

set(TARGET asio)

add_library(${TARGET} INTERFACE
	./include/asio.hpp
)

target_compile_definitions(${TARGET} INTERFACE ASIO_STANDALONE)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
