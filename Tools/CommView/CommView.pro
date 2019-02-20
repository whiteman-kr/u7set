#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T10:51:11
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CommView
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    Options.cpp \
    SerialPortDialog.cpp \
    SerialPortList.cpp \
    SerialPortWorker.cpp \
    Crc.cpp

HEADERS  += MainWindow.h \
    Options.h \
    SerialPortDialog.h \
    SerialPortList.h \
    SerialPortWorker.h \
    SerialPortPacket.h \
    Crc.h

RESOURCES += \
    resources.qrc \
    resources.qrc

DISTFILES +=
