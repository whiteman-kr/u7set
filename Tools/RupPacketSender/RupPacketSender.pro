QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RupPacketSender
TEMPLATE = app


SOURCES += main.cpp\
        Widget.cpp

HEADERS  += Widget.h \
    ../../include/DataProtocols.h
