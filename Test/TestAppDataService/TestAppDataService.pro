#-------------------------------------------------
#
# Project created by QtCreator 2018-06-13T17:21:30
#
#-------------------------------------------------

QT       += testlib
QT       += qml
QT       += xml

TARGET = TestAppDataSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

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

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    ../../lib/Address16.h \
    ../../lib/ServiceSettings.h \
    ../../lib/SimpleMutex.h \
    ../../lib/XmlHelper.h \
    ../../lib/HostAddressPort.h \
    ../../lib/OutputLog.h \
    ../../lib/DeviceHelper.h \
    ../../lib/DeviceObject.h \
    ../../Builder/IssueLogger.h \
    ../../Builder/ModulesRawData.h \
    ../../lib/DbStruct.h \
    ../../lib/ProtoSerialization.h \
    ../../lib/Types.h \
    ../../lib/PropertyObject.h \
    ../../lib/DataSource.h \
    ../../lib/Crc.h \
    ../../lib/Queue.h \
    ../../lib/Times.h \
    ../../lib/DataProtocols.h \
    ../../lib/WUtils.h \
    AppDataServiceClient.h \
    ../../lib/SoftwareInfo.h \
    ../../lib/SimpleThread.h \
    ../../lib/Tcp.h \
    ../../Proto/network.pb.h \
    ../../lib/SocketIO.h \
    ../../lib/CircularLogger.h \
    TestAppDataService.h \
    ../../lib/CfgServerLoader.h \
    ../../lib/BuildInfo.h \
    ../../lib/TcpFileTransfer.h \
    ../../lib/CommandLineParser.h \
    TestUtils.h


SOURCES += \
    ../../lib/Address16.cpp \
    ../../lib/SimpleMutex.cpp \
    ../../lib/XmlHelper.cpp \
    ../../lib/ServiceSettings.cpp \
    ../../lib/HostAddressPort.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../lib/DeviceObject.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../Builder/ModulesRawData.cpp \
    ../../lib/DbStruct.cpp \
    ../../lib/ProtoSerialization.cpp \
    ../../lib/Types.cpp \
#    ../../lib/PropertyObject.cpp \
    ../../lib/DataSource.cpp \
    ../../lib/Crc.cpp \
    ../../lib/Queue.cpp \
    ../../lib/Times.cpp \
    ../../lib/DataProtocols.cpp \
    ../../lib/WUtils.cpp \
    AppDataServiceClient.cpp \
    ../../lib/SoftwareInfo.cpp \
    ../../lib/SimpleThread.cpp \
    ../../lib/Tcp.cpp \
    ../../Proto/network.pb.cc \
    ../../lib/SocketIO.cpp \
    ../../lib/CircularLogger.cpp \
    TestAppDataService.cpp \
    main.cpp \
    ../../lib/CfgServerLoader.cpp \
    ../../lib/BuildInfo.cpp \
    ../../lib/TcpFileTransfer.cpp \
    ../../lib/CommandLineParser.cpp

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
        LIBS += -L$$DESTDIR/. -lprotobuf

        INCLUDEPATH += $$PWD/../../Protobuf
}
unix {
    LIBS += -lprotobuf
}

DEFINES += SRCDIR=\\\"$$PWD/\\\"

DISTFILES += \
    ../../Proto/network.proto
