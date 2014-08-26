#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:14:34
#
#-------------------------------------------------

QT       += core gui

QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpServer
TEMPLATE = app


SOURCES +=\
    servermain.cpp \
    ../lib/UdpSocket.cpp \
    ServerSocket.cpp \
    mainwindow2.cpp

HEADERS  += mainwindow.h \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ServerSocket.h

FORMS    += mainwindow.ui
