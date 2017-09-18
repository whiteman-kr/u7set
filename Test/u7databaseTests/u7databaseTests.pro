#-------------------------------------------------
#
# Project created by QtCreator 2015-06-23T12:45:26
#
#-------------------------------------------------

QT       += core sql testlib network qml widgets
QT       -= gui

TARGET = u7databasetests
CONFIG   += console coverage
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += Q_CONSOLE_APP
DEFINES += SRCDIR=\\\"$$PWD/\\\"

# Use this flags for code coverage info. Must be generated only for unix system (need libgcov)

unix {
        QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
        QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage

        LIBS += \
        -lgcov
}

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
    ../../lib/Types.cpp \
    ../../Proto/network.pb.cc \
    ../../Proto/serialization.pb.cc \
    MultiThreadFileTest.cpp \
    MultiThreadSignalTests.cpp \
    PropertyObjectTests.cpp \
    ProjectPropertyTests.cpp \
    ../../lib/XmlHelper.cpp \
    DbControllerProjectManagementTests.cpp \
    ../../lib/Queue.cpp \
    DbControllerUserManagementTests.cpp \
    DbControllerFileManagementTests.cpp \
    ../../lib/WUtils.cpp \
    ../../lib/DataProtocols.cpp \
    ../../lib/Crc.cpp \
    DbControllerSignalManagementTests.cpp \
    DbControllerHardwareConfigurationTests.cpp \
    ../../lib/HostAddressPort.cpp \
    DbControllerBuildManagementTests.cpp \
    DbControllerVersionControlTests.cpp

HEADERS += \
    UserTests.h \
    FileTests.h \
    OtherTests.h \
    SignalTests.h \
    ../../lib/DbController.h \
    ../../lib/DbWorker.h \
    ../../lib/DbStruct.h \
    ../../lib/DeviceObject.h \
    ../../lib/Factory.h \
    ../../lib/DbProgress.h \
    ../../lib/Signal.h \
    ../../lib/ProtoSerialization.h \
    ../../lib/DbProgressDialog.h \
    ../../lib/DataSource.h \
    ../../lib/SocketIO.h \
    ../../lib/PropertyObject.h \
    ../../lib/Types.h \
    ../../Proto/network.pb.h \
    ../../Proto/serialization.pb.h \
    MultiThreadFileTest.h \
    MultiThreadSignalTests.h \
    PropertyObjectTests.h \
    ProjectPropertyTests.h \
    ../../lib/XmlHelper.h \
    DbControllerProjectManagementTests.h \
    ../../lib/Queue.h \
    DbControllerUserManagementTests.h \
    DbControllerFileManagementTests.h \
    ../../lib/WUtils.h \
    ../../lib/DataProtocols.h \
    ../../lib/Crc.h \
    DbControllerSignalManagementTests.h \
    DbControllerHardwareConfigurationTests.h \
    ../../lib/HostAddressPort.h \
    DbControllerBuildManagementTests.h \
    DbControllerVersionControlTests.h

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

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

RESOURCES += \
    ../../DatabaseUpgrade/DatabaseUpgrade.qrc
