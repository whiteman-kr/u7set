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
    ../../lib/SoftwareSettings.cpp \
    ../../lib/Ui/DialogAbout.cpp \
	../../lib/DataSource.cpp \
    ../../lib/Times.cpp \
    ../../lib/BuildInfo.cpp \
	main.cpp \
	BuildOption.cpp \
	ConfigSocket.cpp \
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
    ../../lib/SoftwareSettings.h \
    ../../lib/Ui/DialogAbout.h \
    ../../lib/OrderedHash.h \
	../../lib/DataSource.h \
    ../../lib/Times.h \
    ../../lib/BuildInfo.h \
	../../CommonLib/PropertyObject.h \
	../../CommonLib/Types.h \
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

# UtilsLib
#
LIBS += -lUtilsLib

# Protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../../Protobuf

# CommonLib
#
LIBS += -lCommonLib

# AppSignalLib
#
LIBS += -lAppSignalLib




