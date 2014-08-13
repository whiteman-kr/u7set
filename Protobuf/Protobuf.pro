#-------------------------------------------------
#
# Project created by QtCreator 2013-05-20T14:39:16
#
#-------------------------------------------------

QT       -= gui

TARGET = protobuf
TEMPLATE = lib
CONFIG += staticlib

win32 {
	CONFIG += warn_off
	QMAKE_CXXFLAGS += /wd4996
}

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

INCLUDEPATH += ../protobuf

SOURCES += \
	google/protobuf/io/coded_stream.cc \
	google/protobuf/stubs/common.cc \
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
	google/protobuf/message_lite.cc \
	google/protobuf/stubs/once.cc \
	google/protobuf/stubs/atomicops_internals_x86_msvc.cc \
	google/protobuf/compiler/parser.cc \
	google/protobuf/io/printer.cc \
	google/protobuf/reflection_ops.cc \
	google/protobuf/repeated_field.cc \
	google/protobuf/service.cc \
	google/protobuf/stubs/structurally_valid.cc \
	google/protobuf/stubs/strutil.cc \
	google/protobuf/stubs/substitute.cc \
	google/protobuf/text_format.cc \
	google/protobuf/io/tokenizer.cc \
	google/protobuf/unknown_field_set.cc \
	google/protobuf/wire_format.cc \
	google/protobuf/wire_format_lite.cc \
	google/protobuf/io/zero_copy_stream.cc \
	google/protobuf/io/zero_copy_stream_impl.cc \
	google/protobuf/io/zero_copy_stream_impl_lite.cc \
	google/protobuf/stubs/stringprintf.cc

HEADERS += \
	google/protobuf/io/coded_stream.h \
	google/protobuf/io/coded_stream_inl.h \
	google/protobuf/stubs/common.h \
	config.h \
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
	google/protobuf/stubs/map-util.h \
	google/protobuf/message.h \
	google/protobuf/message_lite.h \
	google/protobuf/stubs/atomicops.h \
	google/protobuf/stubs/atomicops_internals_x86_msvc.h \
	google/protobuf/stubs/platform_macros.h \
	google/protobuf/stubs/once.h \
	google/protobuf/compiler/parser.h \
	google/protobuf/io/printer.h \
	google/protobuf/reflection_ops.h \
	google/protobuf/repeated_field.h \
	google/protobuf/service.h \
	google/protobuf/stubs/stl_util.h \
	google/protobuf/stubs/stringprintf.h \
	google/protobuf/stubs/template_util.h \
	google/protobuf/stubs/type_traits.h \
	google/protobuf/stubs/strutil.h \
	google/protobuf/stubs/substitute.h \
	google/protobuf/text_format.h \
	google/protobuf/io/tokenizer.h \
	google/protobuf/unknown_field_set.h \
	google/protobuf/wire_format.h \
	google/protobuf/wire_format_lite.h \
	google/protobuf/wire_format_lite_inl.h \
	google/protobuf/io/zero_copy_stream.h \
	google/protobuf/io/zero_copy_stream_impl.h \
	google/protobuf/io/zero_copy_stream_impl_lite.h

