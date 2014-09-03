#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T13:17:21
#
#-------------------------------------------------

QT       += core

QT       -= gui

QT       += network

TARGET = BaseService
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/BaseService.cpp \
    ../lib/SocketIO.cpp

HEADERS += \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ../include/BaseService.h

include(../qtservice/src/qtservice.pri)


# Visual Leak Detector
#
win32 {
        contains(QMAKE_TARGET.arch, x86_64) {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
        } else {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
        }

        INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
        INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}
