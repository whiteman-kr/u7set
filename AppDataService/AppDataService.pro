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
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/OutputLog.cpp \
    ../lib/XmlHelper.cpp \
    AppDataService.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    AppDataProcessingThread.cpp \
    ../lib/WUtils.cpp \
    TcpAppDataServer.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/AppSignalStateFlags.cpp \
    ../lib/AppSignal.cpp \
    ../Builder/IssueLogger.cpp \
    TcpAppDataClient.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    AppDataSource.cpp \
    ../Builder/ModulesRawData.cpp \
    ../lib/CommandLineParser.cpp \
    AppDataServiceMain.cpp \
    TcpArchiveClient.cpp \
    ../lib/SoftwareInfo.cpp \
    AppDataReceiver.cpp \
    SignalStatesProcessingThread.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    RtTrendsServer.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/TuningValue.cpp \
    ../lib/SimpleMutex.cpp \
    ../lib/SimpleAppSignalState.cpp \
    DynamicAppSignalState.cpp

HEADERS += \
	Stable.h \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/Service.h \
    ../lib/CircularLogger.h \
    ../lib/DataSource.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/CfgServerLoader.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    ../lib/ServiceSettings.h \
    ../lib/DeviceHelper.h \
    ../lib/OutputLog.h \
    ../lib/XmlHelper.h \
    ../lib/DataProtocols.h \
    AppDataService.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/OrderedHash.h \
    AppDataProcessingThread.h \
    TcpAppDataServer.h \
    ../Proto/network.pb.h \
    ../lib/Hash.h \
    ../Proto/serialization.pb.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/AppSignal.h \
    ../Builder/IssueLogger.h \
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
    ../lib/ProtoSerialization.h \
    ../lib/TuningValue.h \
    ../lib/SimpleMutex.h \
    ../lib/SimpleAppSignalState.h \
    DynamicAppSignalState.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


win32:QMAKE_CXXFLAGS += /std:c++17

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
    ../Proto/network.proto \
    ../Proto/serialization.proto


CONFIG(debug, debug|release): DEFINES += Q_DEBUG
CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG
