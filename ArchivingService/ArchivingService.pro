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
    ../lib/HostAddressPort.cpp \
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
    ../u7/Builder/IssueLogger.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../u7/Builder/ModulesRawData.cpp \
    ../lib/CommandLineParser.cpp \
    ArchServiceMain.cpp \
    TcpAppDataServer.cpp \
    ../lib/AppSignal.cpp \
    ArchWriteThread.cpp \
    ArchRequestThread.cpp \
    TcpArchRequestsServer.cpp \
    Archive.cpp \
    TimeFilter.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Signal.cpp \
    ../lib/SignalProperties.cpp \
    FileArchWriter.cpp \
    ../lib/Crc16.cpp \
    FileArchReader.cpp

HEADERS += \
    ../lib/HostAddressPort.h \
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
    ../u7/Builder/IssueLogger.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/OutputLog.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../u7/Builder/ModulesRawData.h \
    ../lib/CommandLineParser.h \
    TcpAppDataServer.h \
    ../lib/AppSignal.h \
    ArchWriteThread.h \
    ArchRequestThread.h \
    TcpArchRequestsServer.h \
    Archive.h \
    ../lib/TimeStamp.h \
    TimeFilter.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Signal.h \
    ../lib/Signal.h \
    ../lib/SignalProperties.h \
    FileArchWriter.h \
    ../lib/Crc16.h \
    FileArchReader.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


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

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    Database/Database.qrc


