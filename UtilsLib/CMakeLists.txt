cmake_minimum_required(VERSION 3.16)

project(UtilsLib LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Xml Widgets)

set(TARGET UtilsLib)

add_library(${TARGET} STATIC
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

target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Xml Qt6::Widgets)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_compile_definitions(${TARGET} PRIVATE UTILS_LIB_DOMAIN)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})



