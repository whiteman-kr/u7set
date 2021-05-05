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
	../lib/DataSource.cpp \
    ../lib/BuildInfo.cpp \
	../lib/SoftwareSettings.cpp \
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
    ../lib/DataSource.h \
    ../lib/BuildInfo.h \
	../lib/SoftwareSettings.h \
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

INCLUDEPATH += ./../Protobuf

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a

# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libServiceLib.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
