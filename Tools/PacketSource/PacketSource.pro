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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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
    ../../lib/Ui/DialogAbout.cpp \
    ../../lib/AppSignal.cpp \
    ../../lib/AppSignalStateFlags.cpp \
    ../../lib/TuningValue.cpp \
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
    ../../lib/Ui/DialogAbout.h \
    ../../lib/AppSignal.h \
    ../../lib/AppSignalStateFlags.h \
    ../../lib/TuningValue.h \
    ../../CommonLib/PropertyObject.h \
    ../../lib/Factory.h \
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

DISTFILES += \
    ../../Proto/network.proto \
    ../../Proto/serialization.proto

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
LIBS += -lOnlineLib

# ServiceLib
#
LIBS += -lServiceLib

# UtilsLib
#
LIBS += -lUtilsLib

# HardwareLib
#
LIBS += -lHardwareLib

# Protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../../Protobuf

# CommonLib
#
LIBS += -lCommonLib




