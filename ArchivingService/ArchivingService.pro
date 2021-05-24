#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T11:49:17
#
#-------------------------------------------------

QT  += core
QT  += network
QT  -= gui
QT  += widgets
QT  += qml
QT  += sql
QT  += xml

TARGET = ArchSrv
CONFIG   += console
CONFIG   -= app_bundle

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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

HEADERS += \
	Stable.h \
	../lib/SoftwareSettings.h \
    ../lib/BuildInfo.h \
    FileArchReader.h \
    ArchFile.h \
    BinSearch.h \
    ArchWriterThread.h \
    ArchRequest.h \
    ArchMaintenance.h \
    ArchFileRecord.h \
    ArchFileBuffer.h \
	ArchivingService.h \
	TcpAppDataServer.h \
	TcpArchRequestsServer.h \
	Archive.h \
	TimeFilter.h \

SOURCES += \
	../lib/SoftwareSettings.cpp \
	../lib/BuildInfo.cpp \
	ArchivingService.cpp \
	ArchServiceMain.cpp \
	TcpAppDataServer.cpp \
	TcpArchRequestsServer.cpp \
	Archive.cpp \
	TimeFilter.cpp \
	FileArchReader.cpp \
	ArchFile.cpp \
	ArchWriterThread.cpp \
	ArchRequest.cpp \
	ArchMaintenance.cpp \
	ArchFileRecord.cpp \
	ArchFileBuffer.cpp \

INCLUDEPATH += ../VFrame30
INCLUDEPATH += ./../Protobuf
DEPENDPATH += ../VFrame30

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    Database/Database.qrc

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

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

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a

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
