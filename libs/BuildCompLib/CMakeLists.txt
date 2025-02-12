cmake_minimum_required(VERSION 3.16)

project(BuildCompLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(TARGET BuildCompLib)

qt_add_library(${TARGET} STATIC
	./include/BuildCompLib/BuildComp.h
	./include/BuildCompLib/BuildCompDialog.h
	./include/BuildCompLib/CompareResult.h

	./src/BuildComp.cpp
	./src/BuildCompDialog.cpp
	./src/BuildCompImpl.cpp
	./src/BuildCompImpl.h
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Widgets Qt6::Svg)
target_link_libraries(${TARGET} PUBLIC HardwareLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

