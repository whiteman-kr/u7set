QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml
QT  += sql

TARGET = AppDataSrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest		# CONFIG += c++20 has no effect yet

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
    ../lib/ScriptDeviceObject.cpp \
	../lib/UdpSocket.cpp \
	../lib/Service.cpp \
	../lib/SocketIO.cpp \
	../lib/CircularLogger.cpp \
    ../lib/DataSource.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Signal.cpp \
    ../lib/Types.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/XmlHelper.cpp \
    AppDataService.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    AppDataProcessingThread.cpp \
    ../lib/WUtils.cpp \
    TcpAppDataServer.cpp \
    ../lib/AppSignalStateFlags.cpp \
    ../lib/AppSignal.cpp \
    TcpAppDataClient.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    AppDataSource.cpp \
#    ../Builder/ModulesRawData.cpp \
    ../lib/CommandLineParser.cpp \
    AppDataServiceMain.cpp \
    TcpArchiveClient.cpp \
    ../lib/SoftwareInfo.cpp \
    AppDataReceiver.cpp \
    SignalStatesProcessingThread.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    RtTrendsServer.cpp \
    ../lib/TuningValue.cpp \
    ../lib/SimpleMutex.cpp \
    ../lib/SimpleAppSignalState.cpp \
    DynamicAppSignalState.cpp

HEADERS += \
    ../lib/Address16.h \
	../lib/ConstStrings.h \
    ../lib/LanControllerInfo.h \
    ../lib/ScriptDeviceObject.h \
	Stable.h \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/Service.h \
    ../lib/CircularLogger.h \
    ../lib/DataSource.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/CfgServerLoader.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
	../lib/SoftwareSettings.h \
    ../lib/XmlHelper.h \
    ../lib/DataProtocols.h \
    AppDataService.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/OrderedHash.h \
    AppDataProcessingThread.h \
    TcpAppDataServer.h \
    ../lib/Hash.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/AppSignal.h \
    TcpAppDataClient.h \
    ../lib/Crc.h \
    ../lib/HostAddressPort.h \
    AppDataSource.h \
    ../Builder/ModulesRawData.h \
    ../lib/CommandLineParser.h \
    TcpArchiveClient.h \
    ../lib/TimeStamp.h \
    ../lib/SoftwareInfo.h \
    AppDataReceiver.h \
    ../lib/Socket.h \
    SignalStatesProcessingThread.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    RtTrendsServer.h \
    ../lib/TuningValue.h \
    ../lib/SimpleMutex.h \
    ../lib/SimpleAppSignalState.h \
    DynamicAppSignalState.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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

#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG

include(../qtservice/src/qtservice.pri)

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
