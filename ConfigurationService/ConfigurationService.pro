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
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

SOURCES += \
    ../lib/Address16.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SocketIO.cpp \
    ConfigurationService.cpp \
	../lib/Service.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CommandLineParser.cpp \
    CfgServiceMain.cpp \
    ../lib/XmlHelper.cpp \
    CfgChecker.cpp \
    CfgControlServer.cpp \
    ../lib/SoftwareInfo.cpp \
	../lib/SoftwareSettings.cpp \
	../lib/DeviceObject.cpp \
	../lib/DbStruct.cpp

HEADERS += \
    ../lib/Address16.h \
    ../lib/CfgServerLoader.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/SimpleThread.h \
    ../lib/BuildInfo.h \
    ../lib/SocketIO.h \
    ConfigurationService.h \
	../lib/Service.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
    ../lib/HostAddressPort.h \
    ../lib/CommandLineParser.h \
    ../lib/XmlHelper.h \
    CfgChecker.h \
    CfgControlServer.h \
    ../lib/Types.h \
    ../lib/SoftwareInfo.h \
	../lib/SoftwareSettings.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/PropertyObject.h \
	Stable.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

include(../qtservice/src/qtservice.pri)

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

