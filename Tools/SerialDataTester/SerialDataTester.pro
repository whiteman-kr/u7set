#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T14:59:09
#
#-------------------------------------------------

QT       += core gui xml serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialDataTester
TEMPLATE = app


SOURCES += main.cpp\
        SerialDataTester.cpp \
    PortReceiver.cpp \
    SettingsDialog.cpp

HEADERS  += SerialDataTester.h \
    PortReceiver.h \
    SettingsDialog.h

FORMS    += SerialDataTester.ui \
    SettingsDialog.ui

RESOURCES +=
