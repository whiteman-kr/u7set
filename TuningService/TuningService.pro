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
    ../lib/BuildInfo.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/MemLeaksDetection.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/Service.cpp \
    ../lib/SocketIO.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Tcp.cpp \
    TuningService.cpp \
    ../lib/DataSource.cpp \
    ../lib/XmlHelper.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/Queue.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../lib/Signal.cpp \
    ../lib/Crc.cpp \
    ../lib/WUtils.cpp \
    ../lib/DataProtocols.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/HostAddressPort.cpp \
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
    TuningDataStorage.cpp \
    ../lib/SimpleMutex.cpp

HEADERS += \
    ../lib/Address16.h \
    ../lib/BuildInfo.h \
    ../lib/CfgServerLoader.h \
    ../lib/CircularLogger.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/MemLeaksDetection.h \
    ../lib/OrderedHash.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/Service.h \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/SimpleThread.h \
    ../lib/WUtils.h \
    ../lib/TcpFileTransfer.h \
    ../lib/Tcp.h \
    Stable.h \
    TuningService.h \
    ../lib/DataSource.h \
    ../lib/XmlHelper.h \
	../lib/SoftwareSettings.h \
    ../lib/Queue.h \
    ../lib/DeviceHelper.h \
    ../lib/DeviceObject.h \
    ../lib/PropertyObject.h \
    ../lib/DbStruct.h \
    ../lib/OutputLog.h \
    ../lib/Types.h \
    ../lib/Signal.h \
    ../lib/Crc.h \
    ../lib/Hash.h \
    ../Builder/IssueLogger.h \
    ../lib/HostAddressPort.h \
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
    TuningDataStorage.h \
    ../lib/SimpleMutex.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto
