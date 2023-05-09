cmake_minimum_required(VERSION 3.16)

project(UtilsLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Widgets)

add_library(UtilsLib STATIC
	CUtils.h
	CsvFile.h
	CsvFile.cpp
	ILogFile.h
	LogFile.h
	LogFile.cpp
	Address16.h
	Crc.h
	Crc.cpp
	DomXmlHelper.h
	DomXmlHelper.cpp
	Queue.h
	Queue.cpp
	SimpleMutex.h
	SimpleThread.h
	SimpleThread.cpp
	Ui/UiTools.h
	Ui/UiTools.cpp
	WUtils.h
	XmlHelper.h
	XmlHelper.cpp
	OutputLog.h
	OutputLog.cpp
	CrashExceptionHandler.h
	CrashExceptionHandler.cpp
)

target_link_libraries(UtilsLib PRIVATE Qt6::Core Qt6::Network Qt6::Xml Qt6::Widgets)

target_compile_definitions(UtilsLib PRIVATE UTILS_LIBRARY)
target_include_directories(UtilsLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(UTILS_LIB_DOMAIN)		# TODO: UTILS_LIB_DOMAIN -> UTILS_LIBRARY?


