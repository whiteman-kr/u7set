#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T11:50:49
#
#-------------------------------------------------

QT       += core gui network qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PacketViewer
TEMPLATE = app

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
    ../lib/PropertyObject.cpp \
    SignalTableModel.cpp \
    ../lib/Types.cpp \
    ../lib/XmlHelper.cpp \
    SendTuningFrameWidget.cpp

HEADERS  += SourceListWidget.h \
    PacketSourceModel.h \
    SourceStatusWidget.h \
    ../include/DataSource.h \
    ../include/DeviceObject.h \
    ../include/Signal.h \
    Stable.h \
    ../include/SocketIO.h \
    ../include/DbStruct.h \
    ../include/ProtoSerialization.h \
    PacketBufferTableModel.h \
    ../include/PropertyObject.h \
    SignalTableModel.h \
    ../include/Types.h \
    ../include/DataProtocols.h \
    ../include/XmlHelper.h \
    SendTuningFrameWidget.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
}
