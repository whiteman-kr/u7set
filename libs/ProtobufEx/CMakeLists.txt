cmake_minimum_required(VERSION 3.16)

project(ProtobufEx LANGUAGES CXX)

set(TARGET ProtobufEx)

qt_add_library(${TARGET} STATIC
    ProtoCommonHelper.cpp
    ProtoCommonHelper.h
    ProtoSerialization.h
    ProtoSerialization.cpp
)

set_compiler_standard(${TARGET})

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
)

target_link_libraries(${TARGET} PUBLIC CommonLib Protos)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
