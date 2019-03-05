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
    CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet


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
    PropertyObjectTests.cpp \
    ProjectPropertyTests.cpp \
    ../../lib/XmlHelper.cpp \
    ../../lib/Queue.cpp \
    DbControllerFileManagementTests.cpp \
    ../../lib/WUtils.cpp \
    ../../lib/DataProtocols.cpp \
    ../../lib/Crc.cpp \
    DbControllerSignalManagementTests.cpp \
    DbControllerHardwareConfigurationTests.cpp \
    ../../lib/HostAddressPort.cpp \
    DbControllerVersionControlTests.cpp \
    ../../lib/TuningValue.cpp \
    ../../lib/Times.cpp \
    ../../lib/OutputLog.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../Builder/ModulesRawData.cpp \
    ../../lib/SignalProperties.cpp \
    TestDbBase.cpp \
    DbControllerUserTests.cpp \
    DbControllerProjectTests.cpp \
    ../../lib/SimpleMutex.cpp

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
    PropertyObjectTests.h \
    ProjectPropertyTests.h \
    ../../lib/XmlHelper.h \
    ../../lib/Queue.h \
    DbControllerFileManagementTests.h \
    ../../lib/WUtils.h \
    ../../lib/DataProtocols.h \
    ../../lib/Crc.h \
    DbControllerSignalManagementTests.h \
    DbControllerHardwareConfigurationTests.h \
    ../../lib/HostAddressPort.h \
    DbControllerVersionControlTests.h \
    ../../lib/TuningValue.h \
    ../../lib/Times.h \
    ../../lib/OutputLog.h \
    ../../Builder/IssueLogger.h \
    ../../lib/DeviceHelper.h \
    ../../Builder/ModulesRawData.h \
    ../../lib/SignalProperties.h \
    TestDbBase.h \
    DbControllerUserTests.h \
    DbControllerProjectTests.h \
    ../../lib/SimpleMutex.h

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
        LIBS += -L$$DESTDIR/. -lprotobuf
        INCLUDEPATH += $$PWD/../../Protobuf
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
