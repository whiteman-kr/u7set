cmake_minimum_required(VERSION 3.16)

project(Protobuf LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Threads REQUIRED)
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

set(TARGET Protobuf)

qt_add_library(${TARGET} STATIC
	../Proto/AppSignal.pb.cc
	../Proto/AppSignal.pb.h
	../Proto/ArchSignal.pb.cc
	../Proto/ArchSignal.pb.h
	../Proto/BuildInfo.pb.cc
	../Proto/BuildInfo.pb.h
	../Proto/Common.pb.cc
	../Proto/Common.pb.h
	../Proto/Comparator.pb.cc
	../Proto/Comparator.pb.h
	../Proto/Network.pb.cc
	../Proto/Network.pb.h
	../Proto/ProtoSerialization.cpp
	../Proto/ProtoSerialization.h
	../Proto/Serialization.pb.cc
	../Proto/Serialization.pb.h
	../Proto/Simulator.pb.cc
	../Proto/Simulator.pb.h
	../Proto/Trends.pb.cc
	../Proto/Trends.pb.h
	../Proto/ProtoCommonHelper.cpp
	../Proto/ProtoCommonHelper.h
	google/protobuf/any.cc
	google/protobuf/any.h
	google/protobuf/any.pb.cc
	google/protobuf/any.pb.h
	google/protobuf/any_lite.cc
	google/protobuf/api.pb.cc
	google/protobuf/api.pb.h
	google/protobuf/arena.cc
	google/protobuf/arena.h
	google/protobuf/arena_impl.h
	google/protobuf/arenastring.cc
	google/protobuf/arenastring.h
	google/protobuf/compiler/importer.cc
	google/protobuf/compiler/importer.h
	google/protobuf/compiler/parser.cc
	google/protobuf/compiler/parser.h
	google/protobuf/descriptor.cc
	google/protobuf/descriptor.h
	google/protobuf/descriptor.pb.cc
	google/protobuf/descriptor.pb.h
	google/protobuf/descriptor_database.cc
	google/protobuf/descriptor_database.h
	google/protobuf/duration.pb.cc
	google/protobuf/duration.pb.h
	google/protobuf/dynamic_message.cc
	google/protobuf/dynamic_message.h
	google/protobuf/extension_set.cc
	google/protobuf/extension_set.h
	google/protobuf/extension_set_heavy.cc
	google/protobuf/generated_message_reflection.cc
	google/protobuf/generated_message_reflection.h
	google/protobuf/generated_message_util.cc
	google/protobuf/generated_message_util.h
	google/protobuf/implicit_weak_message.cc
	google/protobuf/implicit_weak_message.h
	google/protobuf/io/coded_stream.cc
	google/protobuf/io/coded_stream.h
	google/protobuf/io/gzip_stream.cc
	google/protobuf/io/gzip_stream.h
	google/protobuf/io/io_win32.cc
	google/protobuf/io/io_win32.h
	google/protobuf/io/printer.cc
	google/protobuf/io/printer.h
	google/protobuf/io/strtod.cc
	google/protobuf/io/strtod.h
	google/protobuf/io/tokenizer.cc
	google/protobuf/io/tokenizer.h
	google/protobuf/io/zero_copy_stream.cc
	google/protobuf/io/zero_copy_stream.h
	google/protobuf/io/zero_copy_stream_impl.cc
	google/protobuf/io/zero_copy_stream_impl.h
	google/protobuf/io/zero_copy_stream_impl_lite.cc
	google/protobuf/io/zero_copy_stream_impl_lite.h
	google/protobuf/map.cc
	google/protobuf/map.h
	google/protobuf/map_entry.h
	google/protobuf/map_entry_lite.h
	google/protobuf/map_field.cc
	google/protobuf/map_field.h
	google/protobuf/map_field_inl.h
	google/protobuf/map_field_lite.h
	google/protobuf/map_test_util.h
	google/protobuf/map_test_util.inc
	google/protobuf/map_test_util_impl.h
	google/protobuf/map_type_handler.h
	google/protobuf/message.cc
	google/protobuf/message.h
	google/protobuf/message_lite.cc
	google/protobuf/message_lite.h
	google/protobuf/parse_context.cc
	google/protobuf/parse_context.h
	google/protobuf/reflection_ops.cc
	google/protobuf/reflection_ops.h
	google/protobuf/repeated_field.cc
	google/protobuf/repeated_field.h
	google/protobuf/service.cc
	google/protobuf/service.h
	google/protobuf/stubs/bytestream.cc
	google/protobuf/stubs/bytestream.h
	google/protobuf/stubs/common.cc
	google/protobuf/stubs/common.h
	google/protobuf/stubs/hash.h
	google/protobuf/stubs/int128.cc
	google/protobuf/stubs/int128.h
	google/protobuf/stubs/logging.h
	google/protobuf/stubs/once.h
	google/protobuf/stubs/platform_macros.h
	google/protobuf/stubs/status.cc
	google/protobuf/stubs/status.h
	google/protobuf/stubs/stl_util.h
	google/protobuf/stubs/stringpiece.cc
	google/protobuf/stubs/stringpiece.h
	google/protobuf/stubs/stringprintf.cc
	google/protobuf/stubs/stringprintf.h
	google/protobuf/stubs/structurally_valid.cc
	google/protobuf/stubs/strutil.cc
	google/protobuf/stubs/strutil.h
	google/protobuf/stubs/substitute.cc
	google/protobuf/stubs/substitute.h
	google/protobuf/stubs/template_util.h
	google/protobuf/stubs/time.cc
	google/protobuf/stubs/time.h
	google/protobuf/text_format.cc
	google/protobuf/text_format.h
	google/protobuf/unknown_field_set.cc
	google/protobuf/unknown_field_set.h
	google/protobuf/util/field_comparator.cc
	google/protobuf/util/field_comparator.h
	google/protobuf/util/field_mask_util.cc
	google/protobuf/util/field_mask_util.h
	google/protobuf/util/json_util.cc
	google/protobuf/util/json_util.h
	google/protobuf/util/message_differencer.cc
	google/protobuf/util/message_differencer.h
	google/protobuf/util/package_info.h
	google/protobuf/util/time_util.cc
	google/protobuf/util/time_util.h
	google/protobuf/util/type_resolver.h
	google/protobuf/util/type_resolver_util.cc
	google/protobuf/util/type_resolver_util.h
	google/protobuf/wire_format.cc
	google/protobuf/wire_format.h
	google/protobuf/wire_format_lite.cc
	google/protobuf/wire_format_lite.h
)

target_precompile_headers(${TARGET} PRIVATE
	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"	
)

if(Threads_FOUND)
	target_compile_definitions(${TARGET} PRIVATE HAVE_PTHREAD)
endif()

target_link_libraries(${TARGET} PUBLIC Threads::Threads Qt6::Core)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
