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
    ../lib/DataSource.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../lib/AppSignal.cpp \
    ../lib/AppSignalParam.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
	TuningService.cpp \
	TcpTuningServer.cpp \
	TcpTuningClient.cpp \
	TuningSource.cpp \
	TuningMemory.cpp \
	TuningClientContext.cpp \
	TuningServiceMain.cpp \
	TuningSourceThread.cpp \
	TuningDataStorage.cpp


HEADERS += \
    ../lib/PropertyObject.h \
	Stable.h \
    ../lib/BuildInfo.h \
    ../lib/OrderedHash.h \
    ../lib/DataSource.h \
	../lib/SoftwareSettings.h \
    ../lib/OutputLog.h \
    ../lib/Types.h \
    ../lib/AppSignal.h \
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
	TuningDataStorage.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

# LibPath
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug
	CONFIG(release, debug|release): LIBS += -L../bin/release
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release
}

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# ServiceLib
#
LIBS += -lServiceLib

# UtilsLib
#
LIBS += -lUtilsLib

# OnlineLib
#
LIBS += -lOnlineLib
