#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T14:59:09
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialDataTester
TEMPLATE = app


SOURCES += main.cpp\
        SerialDataTester.cpp \
    PortReceiver.cpp

HEADERS  += SerialDataTester.h \
    PortReceiver.h

FORMS    += SerialDataTester.ui

RESOURCES += resources.qrc
