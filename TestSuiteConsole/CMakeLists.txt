cmake_minimum_required(VERSION 3.16)

project(TestSuiteConsole LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Sql Qml Svg Test)
qt_standard_project_setup()

add_executable(TestSuiteConsole
	main.cpp
)

target_link_libraries(TestSuiteConsole PRIVATE Qt6::Xml Qt6::Sql Qt6::Qml Qt6::Svg Qt6::Test)
target_link_libraries(TestSuiteConsole PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib TestSuiteLib)


# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(TestSuiteConsole PRIVATE vld.lib)
endif()
