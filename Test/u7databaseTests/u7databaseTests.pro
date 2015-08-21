#-------------------------------------------------
#
# Project created by QtCreator 2015-06-23T12:45:26
#
#-------------------------------------------------

QT       += core sql testlib network qml widgets
QT       -= gui

TARGET = u7databasetests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += Q_CONSOLE_APP


SOURCES += main.cpp \
    UserTests.cpp \
    FileTests.cpp \
    OtherTests.cpp \
    SignalTests.cpp \
    ../../lib/DbController.cpp \
    ../../lib/DbWorker.cpp \
    ../../lib/DbStruct.cpp \
    ../../lib/DeviceObject.cpp \
    ../../lib/DbProgress.cpp \
    ../../lib/Signal.cpp \
    ../../lib/ProtoSerialization.cpp \
    ../../lib/DbProgressDialog.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    UserTests.h \
    FileTests.h \
    OtherTests.h \
    SignalTests.h \
    ../../include/DbController.h \
    ../../include/DbWorker.h \
    ../../include/DbStruct.h \
    ../../include/DeviceObject.h \
    ../../include/Factory.h \
    ../../include/DbProgress.h \
    ../../include/Signal.h \
    ../../include/ProtoSerialization.h \
    ../../include/DbProgressDialog.h

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

	INCLUDEPATH += ./../../Protobuf
}
unix {
	LIBS += -lprotobuf
}

RESOURCES += \
    ../../DatabaseUpgrade/DatabaseUpgrade.qrc
