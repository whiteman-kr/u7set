#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T12:51:14
#
#-------------------------------------------------

QT       += core network qml xml

QT       -= gui

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

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet


SOURCES += \
    ../lib/CfgServerLoader.cpp \
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
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/CommandLineParser.cpp \
    CfgServiceMain.cpp \
    ../lib/XmlHelper.cpp \
    CfgChecker.cpp \
    CfgControlServer.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
    ../Builder/ModulesRawData.cpp \
    ../lib/DbStruct.cpp \
    ../lib/PropertyObject.cpp \
    ../lib/ProtoSerialization.cpp

HEADERS += \
    ../lib/CfgServerLoader.h \
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
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/CommandLineParser.h \
    ../lib/XmlHelper.h \
    CfgChecker.h \
    CfgControlServer.h \
    ../lib/Types.h \
    ../lib/SoftwareInfo.h \
    ../lib/ServiceSettings.h \
    ../lib/DeviceHelper.h \
    ../lib/DeviceObject.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h \
    ../Builder/ModulesRawData.h \
    ../lib/DbStruct.h \
    ../lib/PropertyObject.h \
    ../lib/ProtoSerialization.h

include(../qtservice/src/qtservice.pri)

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

DISTFILES += \
    ../Proto/network.proto
