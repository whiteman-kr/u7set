cmake_minimum_required(VERSION 3.16)

project(BuilderConsole LANGUAGES CXX)

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Sql Xml Widgets Svg)
qt_standard_project_setup()

add_executable(BuilderConsole
	main.cpp
	BuildTask.h
	BuildTask.cpp
)

target_link_libraries(BuilderConsole PRIVATE Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Widgets Qt6::Svg)
target_link_libraries(BuilderConsole PUBLIC HardwareLib VFrame30 Builder Simulator Protobuf)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(BuilderConsole PRIVATE vld.lib)
endif()
