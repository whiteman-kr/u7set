cmake_minimum_required(VERSION 3.16)

project(ServiceLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_library(ServiceLib STATIC
	CommandLineParser.h
	CommandLineParser.cpp
	Service.h
	Service.cpp
)

if(WIN32)
	target_sources(ServiceLib PRIVATE
		qtservice/src/qtservice.cpp
		qtservice/src/qtservice_win.cpp
	)
endif(WIN32)

if(UNIX)
	target_sources(ServiceLib PRIVATE
		qtservice/src/qtservice.cpp
		qtservice/src/qtservice_unix.cpp
		qtservice/src/qtunixsocket.cpp
		qtservice/src/qtunixserversocket.cpp
	)
endif(UNIX)


target_link_libraries(ServiceLib PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(ServiceLib PUBLIC Protobuf OnlineLib CommonLib)

target_compile_definitions(ServiceLib PRIVATE SERVICE_LIBRARY)
#target_include_directories(ServiceLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(SERVICE_LIB_DOMAIN)		# TODO: SERVICE_LIB_DOMAIN -> SERVICE_LIBRARY?
