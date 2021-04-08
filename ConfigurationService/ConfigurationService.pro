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
    ../lib/Address16.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/SoftwareInfo.cpp \
	../lib/SoftwareSettings.cpp \
	../lib/DeviceObject.cpp \
	../lib/Crc.cpp \
	../lib/DbStruct.cpp \
	ConfigurationService.cpp \
	CfgServiceMain.cpp \
	CfgChecker.cpp \
	CfgControlServer.cpp \

HEADERS += \
	Stable.h \
	../lib/Address16.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/SimpleThread.h \
    ../lib/BuildInfo.h \
    ../lib/HostAddressPort.h \
    ../lib/XmlHelper.h \
    ../lib/Types.h \
    ../lib/SoftwareInfo.h \
	../lib/SoftwareSettings.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
	../lib/Crc.h \
    ../lib/PropertyObject.h \
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

