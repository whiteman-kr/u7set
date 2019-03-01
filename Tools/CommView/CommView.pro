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
    Crc.cpp \
    WorkerBase.cpp \
    TestResultDialog.cpp \
    OptionDialog.cpp

HEADERS  += MainWindow.h \
    Options.h \
    SerialPortDialog.h \
    SerialPortList.h \
    SerialPortWorker.h \
    SerialPortPacket.h \
    Crc.h \
    WorkerBase.h \
    TestResultDialog.h \
    OptionDialog.h

RESOURCES += \
	resources.qrc

DISTFILES +=
