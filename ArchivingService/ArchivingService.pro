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
    ../lib/OrderedHash.h \
    ../lib/BuildInfo.h \
	../CommonLib/PropertyObject.h \
    ../lib/AppSignalParam.h \
    ../lib/TimeStamp.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/AppSignal.h \
    ../lib/SignalProperties.h \
	../lib/AppSignalStateFlags.h \
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
	../lib/AppSignalParam.cpp \
	../lib/SoftwareInfo.cpp \
	../lib/TuningValue.cpp \
	../lib/AppSignal.cpp \
	../lib/SignalProperties.cpp \
	../lib/AppSignalStateFlags.cpp \
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
DEPENDPATH += ../VFrame30

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    Database/Database.qrc


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

# CommonLib
#
LIBS += -lCommonLib

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf
