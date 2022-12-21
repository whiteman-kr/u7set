QT += network

TARGET = ServiceLib

TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
include(../warnings.pri)
include(./qtservice/src/qtservice.pri)
include(../codecoverage.pri)

DEFINES -= QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD
INCLUDEPATH += ./../Protobuf

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

unix {
    target.path = /usr/lib
	INSTALLS += target
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

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	MOC_DIR = debug/moc
	RCC_DIR = debug/rcc
	UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
	OBJECTS_DIR = release
	MOC_DIR = release/moc
	RCC_DIR = release/rcc
	UI_DIR = release/ui
}

HEADERS += \
	Stable.h \
	Service.h \
	CommandLineParser.h \

SOURCES += \
	Service.cpp \
	CommandLineParser.cpp \

