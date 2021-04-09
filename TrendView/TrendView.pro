#-------------------------------------------------
#
# Project created by QtCreator 2017-04-04T09:30:09
#
#-------------------------------------------------

QT       += core widgets svg gui printsupport

TARGET = TrendView
TEMPLATE = lib

CONFIG += staticlib

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TrendScale.cpp \
    Forms/DialogChooseTrendSignals.cpp \
    TrendWidget.cpp \
    TrendSignal.cpp \
    TrendSlider.cpp \
    TrendSettings.cpp \
    TrendMainWindow.cpp \
    Trend.cpp \
    DialogTrendSignalProperties.cpp \
    TrendRuler.cpp \
    TrendParam.cpp \
    DialogTrendSignalPoints.cpp \
    DialogTrendSignalPoint.cpp

HEADERS += \
    Forms/DialogChooseTrendSignals.h \
    Stable.h \
    TrendScale.h \
    TrendWidget.h \
    TrendSignal.h \
    ../lib/TimeStamp.h \
    TrendSlider.h \
    TrendSettings.h \
    TrendMainWindow.h \
    Trend.h \
    DialogTrendSignalProperties.h \
    TrendRuler.h \
    TrendParam.h \
    ../lib/CUtils.h \
    DialogTrendSignalPoints.h \
    DialogTrendSignalPoint.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    TrendView.qrc

FORMS += \
    Forms/DialogChooseTrendSignals.ui \
    TrendsMainWindow.ui \
    DialogTrendSignalProperties.ui \
    DialogTrendSignalPoints.ui \
    DialogTrendSignalPoint.ui


# Protobuf
#
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/trends.proto

