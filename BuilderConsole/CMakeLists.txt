cmake_minimum_required(VERSION 3.16)

project(BuilderConsole LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Sql Xml Widgets Svg)
qt_standard_project_setup()

set(TARGET_NAME BuilderConsole)
add_executable(${TARGET_NAME}
	main.cpp
	BuildTask.h
	BuildTask.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Widgets Qt6::Svg)
target_link_libraries(${TARGET_NAME} PRIVATE HardwareLib VFrame30 Builder Simulator Protobuf)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET_NAME} PRIVATE vld.lib)
endif()
