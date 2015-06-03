#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T14:26:26
#
#-------------------------------------------------

QT       += core gui widgets network xmlpatterns qml

TARGET = Monitor
TEMPLATE = app

INCLUDEPATH += $$PWD

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += main.cpp \
    MonitorMainWindow.cpp \
	MonitorCentralWidget.cpp \
	Settings.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    WorkflowSchemeView.cpp

HEADERS  += \
    MonitorMainWindow.h \
    MonitorCentralWidget.h \
	Stable.h \
	Settings.h \
    ../include/SocketIO.h \
    DialogSettings.h \
    WorkflowSchemeView.h

FORMS    += \
    DialogSettings.ui


# Optimization flags
#
win32 {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG


# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

