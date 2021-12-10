QT += qml 

TARGET = CommonLib

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD

include(../compiler.pri)
include(../warnings.pri)

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
	AfbParamValue.h \
	DebugInstCounter.h \
	Factory.h \
	Hash.h \
	HostAddressPort.h \
	OrderedHash.h \
	Stable.h \
	Times.h \
	Types.h \
	PropertyObject.h

SOURCES += \
	AfbParamValue.cpp \
	PropertyObject.cpp \
	Times.cpp \
	Types.cpp

DISTFILES += \
    PropertyObject.impl

