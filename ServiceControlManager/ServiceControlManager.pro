#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServiceControlManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    scanoptionswidget.cpp \
    servicetablemodel.cpp \
    ../lib/UdpSocket.cpp

HEADERS  += mainwindow.h \
    scanoptionswidget.h \
    servicetablemodel.h \
    ../include/UdpSocket.h

FORMS    += mainwindow.ui

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
                ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc

include(../qtsingleapplication/src/qtsingleapplication.pri)
