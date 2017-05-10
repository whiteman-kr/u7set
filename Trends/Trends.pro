#-------------------------------------------------
#
# Project created by QtCreator 2017-04-04T09:34:47
#
#-------------------------------------------------

QT       += core gui widgets svg

TARGET = Trends
TEMPLATE = app

CONFIG += precompile_header		# Enables support for the use of precompiled headers in projects.
CONFIG += c++14					# C++14 support is enabled.
CONFIG += warn_on				# The compiler should output as many warnings as possible. If warn_off is also specified, the last one takes effect.

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

PRECOMPILED_HEADER = Stable.h

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp\
        TrendsMainWindow.cpp \
    Settings.cpp

HEADERS  += TrendsMainWindow.h \
    Stable.h \
    Settings.h \
    ../lib/TimeStamp.h

FORMS    += TrendsMainWindow.ui

# Library lTrendView
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TrendView/release/ -lTrendView
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TrendView/debug/ -lTrendView
else:unix:!macx: LIBS += -L$$OUT_PWD/../TrendView/ -lTrendView

INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/libTrendView.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/libTrendView.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/TrendView.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/TrendView.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../TrendView/libTrendView.a

DISTFILES +=

RESOURCES += \
    Trends.qrc
