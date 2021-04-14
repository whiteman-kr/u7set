QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml
QT  += sql

TARGET = AppDataSrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest		# CONFIG += c++20 has no effect yet

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
    ../lib/ScriptDeviceObject.cpp \
    ../lib/DataSource.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
	../lib/AppSignal.cpp \
    ../lib/Types.cpp \
    ../lib/BuildInfo.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/AppSignalStateFlags.cpp \
	../lib/AppSignalParam.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/TuningValue.cpp \
	AppDataService.cpp \
	AppDataProcessingThread.cpp \
	AppDataSource.cpp \
	AppDataServiceMain.cpp \
	AppDataReceiver.cpp \
	DynamicAppSignalState.cpp	\
	SignalStatesProcessingThread.cpp \
	RtTrendsServer.cpp \
	TcpAppDataServer.cpp \
	TcpArchiveClient.cpp \

HEADERS += \
	Stable.h \
	../lib/ConstStrings.h \
    ../lib/LanControllerInfo.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/DataSource.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
	../lib/AppSignal.h \
    ../lib/CUtils.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/BuildInfo.h \
	../lib/SoftwareSettings.h \
    ../lib/AppSignalStateFlags.h \
	../lib/AppSignalParam.h \
    ../lib/TimeStamp.h \
    ../lib/SoftwareInfo.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    ../lib/TuningValue.h \
	../Builder/ModulesRawData.h \
	AppDataService.h \
	AppDataProcessingThread.h \
	AppDataSource.h \
	AppDataReceiver.h \
	DynamicAppSignalState.h \
	SignalStatesProcessingThread.h \
	RtTrendsServer.h \
	TcpAppDataServer.h \
	TcpArchiveClient.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG

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

