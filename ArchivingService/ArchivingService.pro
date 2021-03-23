#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T11:49:17
#
#-------------------------------------------------

QT  += core
QT  += network
QT  -= gui
QT  += widgets
QT  += qml
QT  += sql
QT  += xml

TARGET = ArchSrv
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

SOURCES += \
    ../lib/Address16.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ArchivingService.cpp \
    ../lib/Queue.cpp \
    ../lib/Service.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/SocketIO.cpp \
    ../lib/Tcp.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Types.cpp \
    ../lib/CommandLineParser.cpp \
    ArchServiceMain.cpp \
    TcpAppDataServer.cpp \
    ../lib/AppSignal.cpp \
    TcpArchRequestsServer.cpp \
    Archive.cpp \
    TimeFilter.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Signal.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Crc16.cpp \
    FileArchReader.cpp \
    ArchFile.cpp \
    ArchWriterThread.cpp \
    ArchRequest.cpp \
    ArchMaintenance.cpp \
    ArchFileRecord.cpp \
    ../lib/AppSignalStateFlags.cpp \
    ArchFileBuffer.cpp \
    ../lib/SimpleMutex.cpp \
    ../lib/SimpleAppSignalState.cpp

HEADERS += \
    ../lib/Address16.h \
    ../lib/HostAddressPort.h \
    ../lib/ScriptDeviceObject.h \
    ArchivingService.h \
    Stable.h \
    ../lib/Queue.h \
    ../lib/Service.h \
	../lib/SoftwareSettings.h \
    ../lib/Address16.h \
    ../lib/OrderedHash.h \
    ../lib/SimpleThread.h \
    ../lib/SocketIO.h \
    ../lib/Tcp.h \
    ../lib/XmlHelper.h \
    ../lib/CfgServerLoader.h \
    ../lib/UdpSocket.h \
    ../lib/BuildInfo.h \
    ../lib/CircularLogger.h \
    ../lib/TcpFileTransfer.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/CommandLineParser.h \
    TcpAppDataServer.h \
    ../lib/AppSignal.h \
    TcpArchRequestsServer.h \
    Archive.h \
    ../lib/TimeStamp.h \
    TimeFilter.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Signal.h \
    ../lib/Signal.h \
    ../lib/SignalProperties.h \
    ../lib/Crc16.h \
    FileArchReader.h \
    ArchFile.h \
    BinSearch.h \
    ArchWriterThread.h \
    ArchRequest.h \
    ArchMaintenance.h \
    ArchFileRecord.h \
    ../lib/AppSignalStateFlags.h \
    ArchFileBuffer.h \
    ../lib/SimpleMutex.h \
    ../lib/WUtils.h \
    ../lib/SimpleAppSignalState.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    Database/Database.qrc


# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
