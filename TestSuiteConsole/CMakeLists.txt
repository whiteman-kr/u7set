cmake_minimum_required(VERSION 3.16)

project(TestSuiteConsole LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Sql Qml Svg Test)
qt_standard_project_setup()

set(TARGET TestSuiteConsole)

add_executable(${TARGET}
	main.cpp
)

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Xml Qt6::Sql Qt6::Qml Qt6::Svg Qt6::Test)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib TestSuiteLib)


# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
