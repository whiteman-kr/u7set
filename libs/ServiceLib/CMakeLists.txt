cmake_minimum_required(VERSION 3.16)

project(ServiceLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml)

set(TARGET ServiceLib)

qt_add_library(${TARGET} STATIC
	./include/ServiceLib/CommandLineParser.h
	./include/ServiceLib/Service.h
	./include/ServiceLib/ServiceStarter.h
	./include/ServiceLib/TcpSrvInfoServer.h
	CommandLineParser.cpp
	TcpSrvInfoServer.cpp
	Service.cpp
	ServiceStarter.cpp
)

if(WIN32)
	target_sources(${TARGET} PRIVATE
		qtservice/src/qtservice.cpp
		qtservice/src/qtservice_win.cpp
	)
endif(WIN32)

if(UNIX)
	target_sources(${TARGET} PRIVATE
		qtservice/src/qtservice.cpp
		qtservice/src/qtservice_unix.cpp
		qtservice/src/qtunixsocket.cpp
		qtservice/src/qtunixserversocket.cpp
	)
endif(UNIX)

target_precompile_headers(${TARGET} PRIVATE
	<QObject>
	<QXmlStreamReader>

	<CommonLib/Types.h>
	<Network.pb.h>
)


target_link_libraries(${TARGET} PUBLIC Qt6::Core Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC ProtobufEx OnlineLib CommonLib)

target_compile_definitions(${TARGET} PRIVATE SERVICE_LIB_DOMAIN)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
