cmake_minimum_required(VERSION 3.16)

project(GatewayLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Network)

set(TARGET GatewayLib)

qt_add_library(${TARGET} STATIC
	AdsGateway.cpp
	AdsGateway.h
	GatewayDescription.cpp
	GatewayDescription.h
	GatewayParserLog.cpp
	GatewayParserLog.h
	IvsImpulseGateway.cpp
	IvsImpulseGateway.h
	ModbusSlaveGateway.cpp
	ModbusSlaveGateway.h
)

target_precompile_headers(${TARGET} PRIVATE
	<set>
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>
	<Hardware.pb.h>
	<Network.pb.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib SimulatorLib AdsGatewayLib)

target_compile_definitions(${TARGET} PRIVATE GATEWAY_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
