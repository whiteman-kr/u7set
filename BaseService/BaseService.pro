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
    BaseService.cpp \
    ../lib/UdpSocket.cpp

HEADERS += \
    BaseService.h \
    ../include/SocketIO.h \
    ../include/UdpSocket.h

include(../qtservice/src/qtservice.pri)
