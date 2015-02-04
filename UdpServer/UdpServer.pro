#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:14:34
#
#-------------------------------------------------

QT       += core gui

QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpServer
TEMPLATE = app


SOURCES +=\
    servermain.cpp \
    ../lib/UdpSocket.cpp \
    ServerSocket.cpp \
    ../lib/BaseService.cpp \
    servermainwindow.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp

HEADERS  += \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ServerSocket.h \
    ../include/BaseService.h \
    servermainwindow.h \
    ../include/CircularLogger.h

include(../qtservice/src/qtservice.pri)

FORMS    += \
    servermainwindow.ui

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
