#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T11:50:49
#
#-------------------------------------------------

QT       += core gui network qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PacketViewer
TEMPLATE = app


SOURCES += main.cpp\
        SourceListWidget.cpp \
    PacketSourceModel.cpp \
    SourceStatusWidget.cpp \
    ../lib/DataSource.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Signal.cpp \
    ../lib/SocketIO.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp

HEADERS  += SourceListWidget.h \
    PacketSourceModel.h \
    SourceStatusWidget.h \
    ../include/DataSource.h \
    ../include/DeviceObject.h \
    ../include/Signal.h \
    Stable.h \
    ../include/SocketIO.h \
    ../include/DbStruct.h \
    ../include/ProtoSerialization.h

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
