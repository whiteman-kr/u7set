QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml

TARGET = DiagDataSrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
gcc:CONFIG += c++20
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
    DiagDataService.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CommandLineParser.cpp \
    DiagDataServiceMain.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/SimpleMutex.cpp

HEADERS += \
    ../lib/Address16.h \
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
    DiagDataService.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    ../lib/HostAddressPort.h \
    ../lib/CommandLineParser.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    ../lib/SimpleMutex.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
