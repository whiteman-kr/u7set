#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T12:51:14
#
#-------------------------------------------------

QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml

TARGET = CfgSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

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

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

SOURCES += \
    ../lib/BuildInfo.cpp \
    ../lib/SoftwareInfo.cpp \
	../lib/SoftwareSettings.cpp \
	ConfigurationService.cpp \
	CfgServiceMain.cpp \
	CfgChecker.cpp \
	CfgControlServer.cpp \

HEADERS += \
	Stable.h \
    ../lib/BuildInfo.h \
    ../lib/SoftwareInfo.h \
	../lib/SoftwareSettings.h \
	ConfigurationService.h \
	CfgChecker.h \
	CfgControlServer.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


DISTFILES += \
    ../Proto/network.proto

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
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
