#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:12:02
#
#-------------------------------------------------

QT       += core gui

QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpClient
TEMPLATE = app


SOURCES += \
        mainwindow.cpp \
        clientmain.cpp \
        ../lib/UdpSocket.cpp \
    ClientSocket.cpp


HEADERS  += mainwindow.h \
        ClientSocket.h \
        ../include/SocketIO.h \
        ../include/UdpSocket.h

FORMS    += mainwindow.ui
