cmake_minimum_required(VERSION 3.16)

project(OnlineLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

set(TARGET OnlineLib)

add_library(${TARGET} STATIC
	BuildInfo.cpp
	BuildInfo.h
	CfgServerLoader.cpp
	CfgServerLoader.h
	CircularLogger.cpp
	CircularLogger.h
	SocketIO.cpp
	SocketIO.h
	SoftwareInfo.cpp
	SoftwareInfo.h
	SoftwareSettings.cpp
	SoftwareSettings.h
	Tcp.cpp
	Tcp.h
	TcpClientStatistics.cpp
	TcpClientStatistics.h
	TcpFileTransfer.cpp
	TcpFileTransfer.h
	UdpSocket.cpp
	UdpSocket.h
)

target_link_libraries(${TARGET} PRIVATE Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib)

target_compile_definitions(${TARGET} PRIVATE ONLINE_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
