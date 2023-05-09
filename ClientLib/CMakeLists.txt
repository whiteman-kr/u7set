cmake_minimum_required(VERSION 3.16)

project(ClientLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets)

add_library(ClientLib STATIC
	AdsConnection.cpp
	AdsConnection.h
	AdsSourceStateConnection.cpp
	AdsSourceStateConnection.h
	AppSignalManager.cpp
	AppSignalManager.h
	ConfigController.cpp
	ConfigController.h
	IAppSignalUpdater.h
	IRecentAppSignals.h
	ITuningLog.h
	RtDataProvider.cpp
	RtDataProvider.h
	RtTrendTcpClient.cpp
	RtTrendTcpClient.h
	TcpAppSourcesState.cpp
	TcpAppSourcesState.h
	TcpSignalClient.cpp
	TcpSignalClient.h
	TcpSignalRecents.cpp
	TcpSignalRecents.h
	TuningConnection.cpp
	TuningConnection.h
	TuningLog.cpp
	TuningLog.h
	TuningSourceState.cpp
	TuningSourceState.h
	TuningTcpClient.cpp
	TuningTcpClient.h
	TuningUserManager.cpp
	TuningUserManager.h
)

target_link_libraries(ClientLib PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets)
target_link_libraries(ClientLib PUBLIC CommonLib Protobuf UtilsLib OnlineLib AppSignalLib)

if (MSVC)
	target_link_libraries(ClientLib PUBLIC Advapi32)
endif (MSVC)
if (UNIX)
	target_link_libraries(ClientLib PUBLIC pam pam_misc)
endif (UNIX)

target_compile_definitions(ClientLib PRIVATE CLIENT_LIBRARY)
#target_include_directories(ClientLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_compile_definitions(CLIENT_LIB_DOMAIN)		# TODO: CLIENT_LIB_DOMAIN-> CLIENT_LIBRARY?
