cmake_minimum_required(VERSION 3.16)

project(OnlineLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

add_library(OnlineLib STATIC
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

target_link_libraries(OnlineLib PRIVATE Qt6::Network)
target_link_libraries(OnlineLib PUBLIC UtilsLib AppSignalLib HardwareLib)

target_compile_definitions(OnlineLib PRIVATE ONLINE_LIBRARY)
#target_include_directories(OnlineLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(ONLINE_LIB_DOMAIN)		# TODO: ONLINE_LIB_DOMAIN -> ONLINE_LIBRARY?
