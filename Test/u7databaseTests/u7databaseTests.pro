#-------------------------------------------------
#
# Project created by QtCreator 2015-06-23T12:45:26
#
#-------------------------------------------------

QT       += sql testlib

QT       -= gui

TARGET = u7databasetests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    UserTests.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    UserTests.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11
