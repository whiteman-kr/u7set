cmake_minimum_required(VERSION 3.16)

project(Archivist LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensure the standard is required
set(CMAKE_CXX_EXTENSIONS OFF)       # Use only standard-compliant C++ (disable compiler-specific extensions)

include(../cmake/vld.cmake)
#include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()

set(TARGET Archivist)

add_executable(${TARGET} WIN32
	Archivist.cpp
	Archivist.h
	ArchivistMain.cpp
	FileArchivist.cpp
	FileArchivist.h
)

target_precompile_headers(${TARGET} PRIVATE
	<set>
	<map>
	<iostream>
)

target_link_libraries(${TARGET} Qt6::Core)
target_link_libraries(${TARGET} CommonLib OnlineLib)
