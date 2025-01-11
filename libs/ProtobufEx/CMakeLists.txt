cmake_minimum_required(VERSION 3.25)

project(ProtobufEx LANGUAGES CXX)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core)
if (MSVC)
    # MSVC warning suppressions
    add_compile_options(
	/wd4018 # '<': signed/unsigned mismatch
	/wd4065 # switch statement contains 'default' but no 'case' labels
	/wd4100 # unreferenced formal parameter
	/wd4127 # conditional expression is constant
	/wd4146 # unary minus operator applied to unsigned type
	/wd4125 # decimal digit terminates octal escape sequence
	/wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
	/wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	/wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
	/wd4305 # 'identifier' : truncation from 'type1' to 'type2'
	/wd4307 # 'operator' : integral constant overflow
	/wd4309 # 'conversion' : truncation of constant value
	/wd4310 #
	/wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
	/wd4355 # 'this' : used in base member initializer list
	/wd4389 #
	/wd4456 #
	/wd4506 # no definition for inline function 'function'
	/wd4702 # unreachable code
	/wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
	/wd4996 # The compiler encountered a deprecated declaration.
	/wd5054 # operator '|': deprecated between enumerations of different types
	/wd26495 # Variable 'variable' is uninitialized. Always initialize a member variable (type.6).
    )
endif (MSVC)

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

# Print ${PROTO_SRCS} ${PROTO_HDRS}
message("PROTO_SRCS: ${PROTO_SRCS}")
message("PROTO_HDRS: ${PROTO_HDRS}")

# --
#
set(TARGET ProtobufEx)

qt_add_library(${TARGET} STATIC
	${PROTO_FILES}
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
	
    ProtoCommonHelper.cpp
    ProtoCommonHelper.h
    ProtoSerialization.h
    ProtoSerialization.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Core protobuf::libprotobuf)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_BINARY_DIR})
