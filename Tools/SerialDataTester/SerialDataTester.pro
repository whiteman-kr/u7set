#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T14:59:09
#
#-------------------------------------------------

QT       += core gui xml serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialDataTester
TEMPLATE = app

CONFIG += c++11


SOURCES += main.cpp\
        SerialDataTester.cpp \
    SettingsDialog.cpp \
    PortReceiver.cpp \
    SerialDataParser.cpp \
    ../../lib/Crc.cpp

HEADERS  += SerialDataTester.h \
    SettingsDialog.h \
    PortReceiver.h \
    SerialDataParser.h \
    ../../lib/Crc.h

FORMS    += SerialDataTester.ui \
    SettingsDialog.ui

RESOURCES +=
