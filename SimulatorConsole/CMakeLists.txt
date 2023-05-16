cmake_minimum_required(VERSION 3.16)

project(SimulatorConsole LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Qml Core Concurrent Network)
qt_standard_project_setup()

set(TARGET SimulatorConsole)

add_executable(${TARGET}
	main.cpp
	SimulatorConsole.qrc
)

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Xml Qt6::Qml Qt6::Core Qt6::Concurrent Qt6::Network)
target_link_libraries(${TARGET} PRIVATE Protobuf Simulator)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
