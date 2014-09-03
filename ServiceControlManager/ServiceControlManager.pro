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
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp

HEADERS  += mainwindow.h \
    scanoptionswidget.h \
    servicetablemodel.h \
    ../include/UdpSocket.h \
    ../include/SocketIO.h

FORMS    +=

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
                ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc

include(../qtsingleapplication/src/qtsingleapplication.pri)


# Visual Leak Detector
#
win32 {
        contains(QMAKE_TARGET.arch, x86_64) {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
        } else {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
        }

        INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
        INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}
