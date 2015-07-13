#-------------------------------------------------
#
# Project created by QtCreator 2015-04-19T13:14:18
#
#-------------------------------------------------

QT       += core
QT       += network
QT       -= gui

TARGET = file2pgsql
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    Convertor.cpp

HEADERS += \
    Convertor.h


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11
