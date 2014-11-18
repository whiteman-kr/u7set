#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T13:17:21
#
#-------------------------------------------------

QT       += core

QT       -= gui

QT       += network

QT		 += widgets

TARGET = BaseSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/BaseService.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp

HEADERS += \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ../include/BaseService.h \
    ../include/CircularLogger.h

include(../qtservice/src/qtservice.pri)

unix:QMAKE_CXXFLAGS += -std=c++11


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
