cmake_minimum_required(VERSION 3.16)

project(GrpcLib LANGUAGES CXX)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 
#
# TARGET: GrpcLib - gRPC servers/clients
#
#
set(TARGET GrpcLib)

add_library(${TARGET} STATIC
	GrpcAppDataSrv.proto
)

set_compiler_standard(${TARGET})
set_warning_level_1(${TARGET})

target_link_libraries(${TARGET} PUBLIC gRPC::grpc++ Protos)

target_include_directories(${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_BINARY_DIR})

protobuf_generate(
    TARGET ${TARGET}
	IMPORT_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/libs/Protos
)

protobuf_generate(
    TARGET ${TARGET}
    LANGUAGE grpc
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    PLUGIN_OPTIONS generate_mock_code=true
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
	IMPORT_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/libs/Protos
)



