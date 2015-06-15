#-------------------------------------------------
#
# Project created by QtCreator 2015-06-13T11:50:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tCalc
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Conversion.cpp

HEADERS  += mainwindow.h \
    Conversion.h

FORMS    +=


QMAKE_CXXFLAGS += -std=c++11

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    Resources.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

