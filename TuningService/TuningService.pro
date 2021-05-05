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
	Stable.h \
    ../lib/BuildInfo.h \
    ../lib/OrderedHash.h \
    ../lib/DataSource.h \
	../lib/SoftwareSettings.h \
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

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../Protobuf

# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
