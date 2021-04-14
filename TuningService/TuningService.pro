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

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
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
    ../lib/BuildInfo.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/DataSource.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../lib/AppSignal.cpp \
    ../lib/AppSignalParam.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
	../Builder/IssueLogger.cpp \
	../Builder/ModulesRawData.cpp \
	TuningService.cpp \
	TcpTuningServer.cpp \
	TcpTuningClient.cpp \
	TuningSource.cpp \
	TuningMemory.cpp \
	TuningClientContext.cpp \
	TuningServiceMain.cpp \
	TuningSourceThread.cpp \
	TuningDataStorage.cpp \


HEADERS += \
	Stable.h \
    ../lib/BuildInfo.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/OrderedHash.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/DataSource.h \
	../lib/SoftwareSettings.h \
    ../lib/DeviceHelper.h \
    ../lib/DeviceObject.h \
    ../lib/PropertyObject.h \
    ../lib/DbStruct.h \
    ../lib/OutputLog.h \
    ../lib/Types.h \
    ../lib/AppSignal.h \
    ../Builder/IssueLogger.h \
    ../Builder/ModulesRawData.h \
    ../lib/AppSignalParam.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
	TuningService.h \
	TcpTuningServer.h \
	TcpTuningClient.h \
	TuningSource.h \
	TuningMemory.h \
	TuningClientContext.h \
	TuningSourceThread.h \
	TuningDataStorage.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
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
