#-------------------------------------------------
#
# Project created by QtCreator 2016-09-02T10:14:17
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg xml


TARGET = TuningClient
TEMPLATE = app

INCLUDEPATH += $$PWD

# DESTDIR
# If you see somewhere 'LNK1146: no argument specified with option '/LIBPATH:' then most likely you have not added this section to a project file
#
win32 {
        CONFIG(debug, debug|release): DESTDIR = ../bin/debug
        CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
        CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
        CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}
# /DESTDIR
#

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
}


CONFIG(debug, debug|release) {
        OBJECTS_DIR = debug
        MOC_DIR = debug/moc
        RCC_DIR = debug/rcc
        UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
        OBJECTS_DIR = release
        MOC_DIR = release/moc
        RCC_DIR = release/rcc
        UI_DIR = release/ui
}



SOURCES += main.cpp\
        MainWindow.cpp \
    ObjectManager.cpp \
    ObjectFilter.cpp \
    TuningPage.cpp \
    TuningObject.cpp \
    Settings.cpp \
    TuningWorkspace.cpp \
    ConfigController.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Crc.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    DialogTuningSources.cpp \
    TcpTuningClient.cpp \
    ../Proto/network.pb.cc \
    ../lib/AppSignalState.cpp \
    ../Proto/serialization.pb.cc

HEADERS  += MainWindow.h \
    ObjectManager.h \
    ObjectFilter.h \
    Stable.h \
    TuningPage.h \
    TuningObject.h \
    Settings.h \
    TuningWorkspace.h \
    ConfigController.h \
    ../lib/HostAddressPort.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/Crc.h \
    ../lib/SocketIO.h \
    DialogSettings.h \
    DialogTuningSources.h \
    TcpTuningClient.h \
    ../Proto/network.pb.h \
    ../lib/AppSignalState.h \
    ../Proto/serialization.pb.h

FORMS    += \
    DialogSettings.ui \
    DialogTuningSources.ui

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h
