#-------------------------------------------------
#
# Project created by QtCreator 2015-11-23T14:35:17
#
#-------------------------------------------------

QT       += core gui xml serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialDataTesterServer
TEMPLATE = app


SOURCES += main.cpp \
    SerialDataTesterServer.cpp \
    ../../lib/Crc.cpp

HEADERS  += \
    SerialDataTesterServer.h \
    ../../lib/Crc.h

FORMS    += \
    SerialDataTesterServer.ui
