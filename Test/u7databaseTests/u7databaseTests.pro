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
DEFINES += SRCDIR=\\\"$$PWD/\\\"

# DESTDIR
#
win32 {
        CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
        CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
        CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
        CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}


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
    ../../lib/DbProgressDialog.cpp \
    ../../lib/DataSource.cpp \
    ../../lib/SocketIO.cpp  \
    ../../lib/PropertyObject.cpp \
    ../../lib/Types.cpp \
    MultiThreadFileTest.cpp \
    MultiThreadSignalTests.cpp \
    PropertyObjectTests.cpp \
    ProjectPropertyTests.cpp \
    ../../lib/XmlHelper.cpp

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
    ../../include/DbProgressDialog.h \
    ../../include/DataSource.h \
    ../../include/SocketIO.h \
    ../../include/PropertyObject.h \
    ../../include/Types.h \
    MultiThreadFileTest.h \
    MultiThreadSignalTests.h \
    PropertyObjectTests.h \
    ProjectPropertyTests.h \
    ../../include/XmlHelper.h

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
