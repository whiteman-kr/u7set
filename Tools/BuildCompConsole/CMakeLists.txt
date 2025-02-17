cmake_minimum_required(VERSION 3.16)

project(BuildCompConsole LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

set(TARGET BuildCompConsole)

add_executable(${TARGET}
	./src/Main.cpp
)

 target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
)

target_link_libraries(${TARGET} PRIVATE Qt6::Core)
target_link_libraries(${TARGET} PRIVATE BuildCompLib)
