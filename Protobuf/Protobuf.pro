#-------------------------------------------------
#
# Project created by QtCreator 2013-05-20T14:39:16
#
#-------------------------------------------------

QT       -= gui

TARGET = protobuf
TEMPLATE = lib
CONFIG += staticlib

CONFIG += warn_off

# std::clamp is part op cpp17
#
unix:QMAKE_CXXFLAGS += --std=c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}

unix {
    INCLUDEPATH += /usr/include		# pthread.h is here
	LIBS += -lpthread
	DEFINES += HAVE_PTHREAD
}

SOURCES += \
        ../Proto/network.pb.cc \
	../Proto/serialization.pb.cc \
        ../Proto/trends.pb.cc \
	../Proto/ProtoSerialization.cpp \
	google/protobuf/api.pb.cc \
	google/protobuf/duration.pb.cc \
	google/protobuf/implicit_weak_message.cc \
	google/protobuf/io/coded_stream.cc \
	google/protobuf/io/io_win32.cc \
	google/protobuf/io/strtod.cc \
	google/protobuf/map.cc \
	google/protobuf/map_field.cc \
	google/protobuf/message_lite.cc \
	google/protobuf/parse_context.cc \
	google/protobuf/stubs/bytestream.cc \
	google/protobuf/stubs/common.cc \
	google/protobuf/any.cc \
	google/protobuf/any.pb.cc \
	google/protobuf/any_lite.cc \
	google/protobuf/arena.cc \
	google/protobuf/arenastring.cc \
	google/protobuf/descriptor.cc \
	google/protobuf/descriptor.pb.cc \
	google/protobuf/descriptor_database.cc \
	google/protobuf/dynamic_message.cc \
	google/protobuf/extension_set.cc \
	google/protobuf/extension_set_heavy.cc \
	google/protobuf/generated_message_reflection.cc \
	google/protobuf/generated_message_util.cc \
	google/protobuf/io/gzip_stream.cc \
	google/protobuf/compiler/importer.cc \
	google/protobuf/message.cc \
	google/protobuf/compiler/parser.cc \
	google/protobuf/io/printer.cc \
	google/protobuf/reflection_ops.cc \
	google/protobuf/repeated_field.cc \
	google/protobuf/service.cc \
	google/protobuf/stubs/int128.cc \
	google/protobuf/stubs/status.cc \
	google/protobuf/stubs/stringpiece.cc \
	google/protobuf/stubs/structurally_valid.cc \
	google/protobuf/stubs/strutil.cc \
	google/protobuf/stubs/substitute.cc \
	google/protobuf/stubs/time.cc \
	google/protobuf/text_format.cc \
	google/protobuf/io/tokenizer.cc \
	google/protobuf/unknown_field_set.cc \
	google/protobuf/util/field_comparator.cc \
	google/protobuf/util/field_mask_util.cc \
	google/protobuf/util/json_util.cc \
	google/protobuf/util/message_differencer.cc \
	google/protobuf/util/time_util.cc \
	google/protobuf/util/type_resolver_util.cc \
	google/protobuf/wire_format.cc \
	google/protobuf/wire_format_lite.cc \
	google/protobuf/io/zero_copy_stream.cc \
	google/protobuf/io/zero_copy_stream_impl.cc \
	google/protobuf/io/zero_copy_stream_impl_lite.cc \
	google/protobuf/stubs/stringprintf.cc

HEADERS += \
        ../Proto/network.pb.h \
        ../Proto/serialization.pb.h \
        ../Proto/trends.pb.h \
	../Proto/ProtoSerialization.h \
	google/protobuf/api.pb.h \
	google/protobuf/duration.pb.h \
	google/protobuf/implicit_weak_message.h \
	google/protobuf/io/coded_stream.h \
	google/protobuf/io/io_win32.h \
	google/protobuf/io/strtod.h \
	google/protobuf/map.h \
	google/protobuf/map_entry.h \
	google/protobuf/map_entry_lite.h \
	google/protobuf/map_field.h \
	google/protobuf/map_field_inl.h \
	google/protobuf/map_field_lite.h \
	google/protobuf/map_test_util.h \
	google/protobuf/map_test_util.inc \
	google/protobuf/map_test_util_impl.h \
	google/protobuf/map_type_handler.h \
	google/protobuf/message_lite.h \
	google/protobuf/parse_context.h \
	google/protobuf/stubs/bytestream.h \
	google/protobuf/stubs/common.h \
	google/protobuf/any.h \
	google/protobuf/any.pb.h \
	google/protobuf/arena.h \
	google/protobuf/arena_impl.h \
	google/protobuf/arenastring.h \
	google/protobuf/descriptor.h \
	google/protobuf/descriptor.pb.h \
	google/protobuf/descriptor_database.h \
	google/protobuf/dynamic_message.h \
	google/protobuf/extension_set.h \
	google/protobuf/generated_message_reflection.h \
	google/protobuf/generated_message_util.h \
	google/protobuf/io/gzip_stream.h \
	google/protobuf/stubs/hash.h \
	google/protobuf/compiler/importer.h \
	google/protobuf/message.h \
	google/protobuf/stubs/int128.h \
	google/protobuf/stubs/logging.h \
	google/protobuf/stubs/platform_macros.h \
	google/protobuf/stubs/once.h \
	google/protobuf/compiler/parser.h \
	google/protobuf/io/printer.h \
	google/protobuf/reflection_ops.h \
	google/protobuf/repeated_field.h \
	google/protobuf/service.h \
	google/protobuf/stubs/status.h \
	google/protobuf/stubs/stl_util.h \
	google/protobuf/stubs/stringpiece.h \
	google/protobuf/stubs/stringprintf.h \
	google/protobuf/stubs/template_util.h \
	google/protobuf/stubs/strutil.h \
	google/protobuf/stubs/substitute.h \
	google/protobuf/stubs/time.h \
	google/protobuf/text_format.h \
	google/protobuf/io/tokenizer.h \
	google/protobuf/unknown_field_set.h \
	google/protobuf/util/field_comparator.h \
	google/protobuf/util/field_mask_util.h \
	google/protobuf/util/json_util.h \
	google/protobuf/util/message_differencer.h \
	google/protobuf/util/package_info.h \
	google/protobuf/util/time_util.h \
	google/protobuf/util/type_resolver.h \
	google/protobuf/util/type_resolver_util.h \
	google/protobuf/wire_format.h \
	google/protobuf/wire_format_lite.h \
	google/protobuf/io/zero_copy_stream.h \
	google/protobuf/io/zero_copy_stream_impl.h \
	google/protobuf/io/zero_copy_stream_impl_lite.h

gcc {
#	SOURCES += google/protobuf/stubs/atomicops_internals_x86_gcc.cc
#	HEADERS += google/protobuf/stubs/atomicops_internals_x86_gcc.h
}

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto \
    ../Proto/trends.proto

