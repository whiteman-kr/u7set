QT      += core
QT      -= gui
QT      += network
QT	+= widgets
QT      += qml
QT      += xml

TARGET = BaseSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

HEADERS += \
	Stable.h \
	../lib/SoftwareSettings.h \
	../lib/SoftwareInfo.h \

SOURCES += \
	../lib/SoftwareSettings.cpp \
	../lib/SoftwareInfo.cpp \
	BaseServiceMain.cpp \

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# ServiceLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lServiceLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lServiceLib
}

# OnlineLib
#
win32 {
        CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lOnlineLib
        CONFIG(release, debug|release): LIBS += -L../bin/release/ -lOnlineLib
}
unix {
        CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lOnlineLib
        CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lOnlineLib
}

# UtilsLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lUtilsLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lUtilsLib
}

# CommonLib
#
LIBS += -lCommonLib

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf
