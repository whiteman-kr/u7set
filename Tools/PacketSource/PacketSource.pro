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
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
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
    ../../lib/LanControllerInfoHelper.cpp \
    ../../lib/SoftwareSettings.cpp \
    ../../lib/ScriptDeviceObject.cpp \
    ../../lib/Ui/DialogAbout.cpp \
    ../../lib/Signal.cpp \
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
    ../../lib/Times.cpp \
    ../../lib/SoftwareInfo.cpp \
    ../../lib/BuildInfo.cpp \
	../../Builder/IssueLogger.cpp \
	main.cpp \
	BuildOption.cpp \
	ConfigSocket.cpp \
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
	Stable.h \
	../../lib/LanControllerInfo.h \
    ../../lib/LanControllerInfoHelper.h \
    ../../lib/SoftwareSettings.h \
    ../../lib/ScriptDeviceObject.h \
    ../../lib/Ui/DialogAbout.h \
    ../../lib/Signal.h \
    ../../lib/AppSignalStateFlags.h \
    ../../lib/DbStruct.h \
    ../../lib/DeviceObject.h \
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
    ../../lib/Times.h \
    ../../lib/SoftwareInfo.h \
    ../../lib/BuildInfo.h \
	../../Builder/IssueLogger.h \
	BuildOption.h \
	ConfigSocket.h \
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

# OnlineLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lOnlineLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lOnlineLib
}

# ServiceLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lServiceLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lServiceLib
}

# UtilsLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lUtilsLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lUtilsLib
}



