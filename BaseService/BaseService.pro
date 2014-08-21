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
    UdpSocket.cpp

HEADERS += \
    BaseService.h \
    UdpSocket.h

include(../qtservice/src/qtservice.pri)
