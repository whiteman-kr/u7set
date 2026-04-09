cmake_minimum_required(VERSION 3.16)

project(ProtosAndGrpc LANGUAGES CXX)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

#
#
# TARGET: Protos - regular protobuf messages
#
#
set(TARGET Protos)

set(PROTO_FILES 
    Afb.proto
    AppSignal.proto
    AppSignalList.proto
    ArchSignal.proto
    BuildInfo.proto
    Bus.proto
    Common.proto
    Comparator.proto
    Envelope.proto
    Hardware.proto
    Network.proto
    License.proto
    Simulator.proto
    Trends.proto
    VFrame30.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(${TARGET} STATIC
	${PROTO_FILES}
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
)

set_warning_level_1(${TARGET})
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON) # Ensure for shared libraries (AdsBridge)

target_link_libraries(${TARGET} PUBLIC protobuf::libprotobuf)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_BINARY_DIR})


# 
#
# TARGET: SimulatorGrpc - Simulator gRPC messages
#
#
set(TARGET SimulatorGrpc)

add_library(${TARGET} STATIC
	SimService.proto
)

set_compiler_standard(${TARGET})
set_warning_level_1(${TARGET})

target_link_libraries(${TARGET} PUBLIC gRPC::grpc++ Protos)

#target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
#target_include_directories(${TARGET} INTERFACE ${CMAKE_BINARY_DIR})

target_include_directories(${TARGET} INTERFACE ${Protobuf_INCLUDE_DIRS})

protobuf_generate(
    TARGET ${TARGET}
)

protobuf_generate(
    TARGET ${TARGET}
    LANGUAGE grpc
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    PLUGIN_OPTIONS generate_mock_code=true
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
)



