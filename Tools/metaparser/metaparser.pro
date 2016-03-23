QT += core
QT -= gui

TARGET = metaparser
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    Metaparser.cpp

HEADERS += \
    Metaparser.h

