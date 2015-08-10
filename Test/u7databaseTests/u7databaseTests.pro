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
    UserTests.cpp \
    FileTests.cpp \
    OtherTests.cpp \
    SignalTests.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    UserTests.h \
    FileTests.h \
    OtherTests.h \
    SignalTests.h
