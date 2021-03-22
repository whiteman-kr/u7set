#-------------------------------------------------
#
# Project created by QtCreator 2018-03-27T13:22:36
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += widgets
QT       += network
QT       += sql
QT       += qml
QT       += xml
QT       += concurrent

TARGET = PacketSource
CONFIG += gui
TEMPLATE = app

# c++20 support
#
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

win32:RC_ICONS += icons/PacketSource.ico

# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

SOURCES += \
    ../../lib/CfgServerLoader.cpp \
    ../../lib/LanControllerInfoHelper.cpp \
    ../../lib/SoftwareSettings.cpp \
    ../../lib/TcpFileTransfer.cpp \
    BuildOption.cpp \
    ConfigSocket.cpp \
    ../../lib/ScriptDeviceObject.cpp \
    main.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../lib/CommandLineParser.cpp \
    ../../lib/XmlHelper.cpp \
    ../../lib/SocketIO.cpp \
    ../../lib/HostAddressPort.cpp \
    ../../lib/SimpleThread.cpp \
    ../../lib/Crc.cpp \
    ../../lib/DataProtocols.cpp \
    ../../lib/WUtils.cpp \
    ../../lib/Ui/DialogAbout.cpp \
    ../../lib/Signal.cpp \
    ../../lib/Address16.cpp \
    ../../lib/AppSignalStateFlags.cpp \
    ../../lib/DbStruct.cpp \
    ../../lib/DeviceObject.cpp \
    ../../lib/TuningValue.cpp \
    ../../lib/Types.cpp \
    ../../lib/ModuleFirmware.cpp \
    ../../lib/SignalProperties.cpp \
    ../../lib/DataSource.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../lib/SimpleMutex.cpp \
    ../../lib/Times.cpp \
    ../../lib/SoftwareInfo.cpp \
    ../../lib/CircularLogger.cpp \
    ../../lib/Tcp.cpp \
    ../../lib/BuildInfo.cpp \
    CmdLineParam.cpp \
    PacketSourceCore.cpp \
    MainWindow.cpp \
    Options.cpp \
    OptionsDialog.cpp \
    SourceWorker.cpp \
    SourceBase.cpp \
    SourceList.cpp \
    FindSignalPanel.cpp \
    SignalList.cpp \
    SignalBase.cpp \
    History.cpp \
    HistoryList.cpp \
    FrameBase.cpp \
    FrameDataPanel.cpp \
    UalTesterServer.cpp

HEADERS += \
    ../../lib/CfgServerLoader.h \
    ../../lib/LanControllerInfo.h \
    ../../lib/LanControllerInfoHelper.h \
    ../../lib/SoftwareSettings.h \
    ../../lib/TcpFileTransfer.h \
    BuildOption.h \
    ConfigSocket.h \
    ../../lib/ScriptDeviceObject.h \
    Stable.h \
    ../../Builder/IssueLogger.h \
    ../../lib/CommandLineParser.h \
    ../../lib/XmlHelper.h \
    ../../lib/SocketIO.h \
    ../../lib/HostAddressPort.h \
    ../../lib/SimpleThread.h \
    ../../lib/Crc.h \
    ../../lib/DataProtocols.h \
    ../../lib/WUtils.h \
    ../../lib/Ui/DialogAbout.h \
    ../../lib/Signal.h \
    ../../lib/Address16.h \
    ../../lib/AppSignalStateFlags.h \
    ../../lib/DbStruct.h \
    ../../lib/DeviceObject.h \
    ../../lib/Hash.h \
    ../../lib/TuningValue.h \
    ../../lib/Types.h \
    ../../lib/PropertyObject.h \
    ../../lib/Factory.h \
    ../../lib/ModuleFirmware.h \
    ../../lib/DebugInstCounter.h \
    ../../lib/OrderedHash.h \
    ../../lib/SignalProperties.h \
    ../../lib/DataSource.h \
    ../../lib/OutputLog.h \
    ../../lib/DeviceHelper.h \
    ../../lib/SimpleMutex.h \
    ../../lib/Times.h \
    ../../lib/SoftwareInfo.h \
    ../../lib/CircularLogger.h \
    ../../lib/Tcp.h \
    ../../lib/BuildInfo.h \
    CmdLineParam.h \
    PacketSourceCore.h \
    MainWindow.h \
    Options.h \
    OptionsDialog.h \
    SourceWorker.h \
    SourceBase.h \
    SourceList.h \
    FindSignalPanel.h \
    SignalBase.h \
    SignalList.h \
    History.h \
    HistoryList.h \
    FrameBase.h \
    FrameDataPanel.h \
    UalTesterServer.h


RESOURCES += \
    resources.qrc

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../../Protobuf

DISTFILES += \
	../../Proto/network.proto \
    ../../Proto/serialization.proto

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
