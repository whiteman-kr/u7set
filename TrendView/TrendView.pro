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

include(../compiler.pri)
include(../warnings.pri)
include(../codecoverage.pri)

TRANSLATIONS = languages/TrendView_ua.ts

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

SOURCES += \
    DialogChooseTrendSignals.cpp \
    TrendArchiveServer.cpp \
    TrendScale.cpp \
    TrendSignalSet.cpp \
    TrendSignalState.cpp \
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
    ../lib/ISignalDataServer.h \
    ../lib/ISignalHasTag.h \
    DialogChooseTrendSignals.h \
    ITrendDataProvider.h \
    Stable.h \
    TrendArchiveServer.h \
    TrendScale.h \
    TrendSignalSet.h \
    TrendSignalState.h \
    TrendWidget.h \
    TrendSignal.h \
    TrendSlider.h \
    TrendSettings.h \
    TrendMainWindow.h \
    Trend.h \
    DialogTrendSignalProperties.h \
    TrendRuler.h \
    TrendParam.h \
    DialogTrendSignalPoints.h \
    DialogTrendSignalPoint.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    TrendView.qrc

FORMS += \
    DialogChooseTrendSignals.ui \
    TrendsMainWindow.ui \
    DialogTrendSignalProperties.ui \
    DialogTrendSignalPoints.ui \
    DialogTrendSignalPoint.ui


# Protobuf
#
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/trends.proto

