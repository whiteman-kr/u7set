#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T11:50:49
#
#-------------------------------------------------

QT       += core gui network qml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PacketViewer
TEMPLATE = app

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

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

SOURCES += main.cpp\
    ../lib/Address16.cpp \
    ../lib/SignalMacro.cpp \
        SourceListWidget.cpp \
    PacketSourceModel.cpp \
    SourceStatusWidget.cpp \
    ../lib/DataSource.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Signal.cpp \
    ../lib/SocketIO.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    PacketBufferTableModel.cpp \
    SignalTableModel.cpp \
    ../lib/Types.cpp \
    ../lib/XmlHelper.cpp \
    SendTuningFrameWidget.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/TuningValue.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/OutputLog.cpp \
    ../Builder/ModulesRawData.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/SimpleMutex.cpp

HEADERS  += SourceListWidget.h \
    ../lib/Address16.h \
    ../lib/SignalMacro.h \
    PacketSourceModel.h \
    SourceStatusWidget.h \
    ../lib/DataSource.h \
    ../lib/DeviceObject.h \
    ../lib/Signal.h \
    Stable.h \
    ../lib/SocketIO.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    PacketBufferTableModel.h \
    ../lib/PropertyObject.h \
    SignalTableModel.h \
    ../lib/Types.h \
    ../lib/DataProtocols.h \
    ../lib/XmlHelper.h \
    SendTuningFrameWidget.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    ../lib/HostAddressPort.h \
    ../lib/TuningValue.h \
    ../Builder/IssueLogger.h \
    ../lib/DeviceHelper.h \
    ../lib/OutputLog.h \
    ../Builder/ModulesRawData.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    ../lib/SimpleMutex.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

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
win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
}
