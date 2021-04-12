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
	../../lib/ScriptDeviceObject.cpp \
	../../lib/DataSource.cpp \
	../../lib/DeviceObject.cpp \
	../../lib/Signal.cpp \
	../../lib/DbStruct.cpp \
	../../lib/Types.cpp \
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
	../../lib/ScriptDeviceObject.h \
	../../lib/DataSource.h \
	../../lib/DeviceObject.h \
	../../lib/Signal.h \
	../../lib/DbStruct.h \
	../../lib/PropertyObject.h \
	../../lib/Types.h \
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

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../../Protobuf

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


