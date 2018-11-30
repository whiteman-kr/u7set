QT += core
QT -= gui
QT += network
QT += qml
QT += xml
QT += widgets


TARGET = TuningSrv
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
    ../lib/BuildInfo.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Service.cpp \
    ../lib/SocketIO.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Tcp.cpp \
    TuningService.cpp \
    ../lib/DataSource.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/Queue.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/OutputLog.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Types.cpp \
    ../lib/Signal.cpp \
    ../lib/Crc.cpp \
    ../lib/WUtils.cpp \
    ../lib/DataProtocols.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    TcpTuningServer.cpp \
    TcpTuningClient.cpp \
    TuningSource.cpp \
    ../Builder/ModulesRawData.cpp \
    TuningMemory.cpp \
    TuningClientContext.cpp \
    ../lib/CommandLineParser.cpp \
    TuningServiceMain.cpp \
    ../lib/AppSignal.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    TuningSourceThread.cpp \
    TuningDataStorage.cpp

HEADERS += \
    ../lib/BuildInfo.h \
    ../lib/CfgServerLoader.h \
    ../lib/CircularLogger.h \
    ../lib/OrderedHash.h \
    ../lib/Service.h \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/SimpleThread.h \
    ../lib/WUtils.h \
    ../lib/TcpFileTransfer.h \
    ../lib/Tcp.h \
    TuningService.h \
    ../lib/DataSource.h \
    ../lib/XmlHelper.h \
    ../lib/ServiceSettings.h \
    ../lib/Queue.h \
    ../lib/DeviceHelper.h \
    ../lib/DeviceObject.h \
    ../lib/PropertyObject.h \
    ../lib/DbStruct.h \
    ../lib/OutputLog.h \
    ../lib/ProtoSerialization.h \
    ../lib/Types.h \
    ../lib/Signal.h \
    ../lib/Crc.h \
    ../lib/Hash.h \
    ../Builder/IssueLogger.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    TcpTuningServer.h \
    TcpTuningClient.h \
    TuningSource.h \
    ../Builder/ModulesRawData.h \
    ../lib/DataProtocols.h \
    TuningMemory.h \
    TuningClientContext.h \
    ../lib/CommandLineParser.h \
    ../lib/AppSignal.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    TuningSourceThread.h \
    TuningDataStorage.h

include(../qtservice/src/qtservice.pri)

win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

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
    ../Proto/network.proto \
    ../Proto/serialization.proto
