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

SOURCES += \
    ../lib/Address16.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/Queue.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Types.cpp \
    ../lib/AppSignal.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Signal.cpp \
    ../lib/SignalProperties.cpp \
	../lib/Crc.cpp \
    ../lib/AppSignalStateFlags.cpp \
    ../lib/SimpleMutex.cpp \
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

HEADERS += \
	Stable.h \
	../lib/Address16.h \
    ../lib/HostAddressPort.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/Queue.h \
	../lib/SoftwareSettings.h \
    ../lib/Address16.h \
    ../lib/OrderedHash.h \
    ../lib/SimpleThread.h \
    ../lib/XmlHelper.h \
    ../lib/BuildInfo.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/AppSignal.h \
    ../lib/TimeStamp.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Signal.h \
    ../lib/Signal.h \
    ../lib/SignalProperties.h \
	../lib/Crc.h \
	../lib/AppSignalStateFlags.h \
	../lib/SimpleMutex.h \
	../lib/WUtils.h \
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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf
