#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T11:50:49
#
#-------------------------------------------------

QT       += core gui network qml xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PacketViewer
TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

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
	../../lib/DataSource.cpp \
	../../lib/AppSignal.cpp \
	../../lib/TuningValue.cpp \
	../../lib/DeviceHelper.cpp \
	../../lib/OutputLog.cpp \
	../../lib/Times.cpp \
	../../lib/SignalProperties.cpp \
	../../Builder/IssueLogger.cpp \
	main.cpp \
	SourceListWidget.cpp \
	PacketSourceModel.cpp \
	SourceStatusWidget.cpp \
	PacketBufferTableModel.cpp \
	SignalTableModel.cpp \
	SendTuningFrameWidget.cpp \


HEADERS  += \
	Stable.h \
	../../Proto/serialization.pb.h \
	../../lib/LanControllerInfo.h \
	../../lib/LanControllerInfoHelper.h \
	../../lib/DataSource.h \
	../../lib/AppSignal.h \
	../../CommonLib/PropertyObject.h \
	../../lib/TuningValue.h \
	../../lib/DeviceHelper.h \
	../../lib/OutputLog.h \
	../../lib/Times.h \
	../../lib/SignalProperties.h \
	../../Builder/IssueLogger.h \
	SourceListWidget.h \
	PacketSourceModel.h \
	SourceStatusWidget.h \
	PacketBufferTableModel.h \
	SignalTableModel.h \
	SendTuningFrameWidget.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# HardwareLib
#
LIBS += -lHardwareLib

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

# CommonLib
#
LIBS += -lCommonLib

# Protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../../Protobuf


