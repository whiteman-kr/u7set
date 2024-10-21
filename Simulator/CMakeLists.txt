cmake_minimum_required(VERSION 3.16)

project(Simulator LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SchemaClientLib)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)
qt_standard_project_setup()

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set(TARGET Simulator)

add_executable(${TARGET} WIN32
	./src/Main.cpp
	./src/SimPropertyStorage.cpp
	./src/SimPropertyStorage.h
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Widgets)
target_link_libraries(${TARGET} PRIVATE CommonLib LicenseLib HardwareLib VFrame30 SimulatorUi)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
