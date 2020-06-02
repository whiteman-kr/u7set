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

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

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
    ../lib/MemLeaksDetection.cpp \
    ../lib/SignalMacro.cpp \
    ArchivingService.cpp \
    ../lib/Queue.cpp \
    ../lib/Service.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/SocketIO.cpp \
    ../lib/Tcp.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/CfgServerLoader.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/UdpSocket.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/DeviceObject.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../Builder/ModulesRawData.cpp \
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
    ../lib/MemLeaksDetection.h \
    ../lib/SignalMacro.h \
    ArchivingService.h \
    Stable.h \
    ../lib/Queue.h \
    ../lib/Service.h \
    ../lib/ServiceSettings.h \
    ../lib/Address16.h \
    ../lib/OrderedHash.h \
    ../lib/SimpleThread.h \
    ../lib/SocketIO.h \
    ../lib/Tcp.h \
    ../lib/XmlHelper.h \
    ../lib/CfgServerLoader.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/UdpSocket.h \
    ../lib/BuildInfo.h \
    ../lib/CircularLogger.h \
    ../lib/DeviceHelper.h \
    ../lib/TcpFileTransfer.h \
    ../lib/DeviceObject.h \
    ../Builder/IssueLogger.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/OutputLog.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../Builder/ModulesRawData.h \
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

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
}

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    Database/Database.qrc


