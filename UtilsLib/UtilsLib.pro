QT += network xml

TARGET = UtilsLib

TEMPLATE = lib
CONFIG += staticlib

#win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD
INCLUDEPATH += ./../Protobuf

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# Optimization flags
#
win32 {
}
unix {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

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
	Address16.h \
	Crc.h \
	DomXmlHelper.h \
	Hash.h \
	Queue.h \
	SimpleMutex.h \
	SimpleThread.h \
	WUtils.h \
	XmlHelper.h \

SOURCES += \
	Crc.cpp \
	DomXmlHelper.cpp \
	Queue.cpp \
	SimpleThread.cpp \
	XmlHelper.cpp \
