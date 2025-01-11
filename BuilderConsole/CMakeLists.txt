cmake_minimum_required(VERSION 3.28)

project(BuilderConsole LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Qml Sql Xml Widgets Svg)
qt_standard_project_setup()

set(TARGET BuilderConsole)

add_executable(${TARGET}
	main.cpp
	BuildTask.h
	BuildTask.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>

	"../AppSignalLib/AppSignal.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Widgets Qt6::Svg)
target_link_libraries(${TARGET} PRIVATE CommonLib Behavior ReportLib HardwareLib VFrame30 Builder SimulatorLib ProtobufEx)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
