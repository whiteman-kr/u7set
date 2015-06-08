#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T14:26:26
#
#-------------------------------------------------

QT       += core gui widgets network xmlpatterns qml

TARGET = Monitor
TEMPLATE = app

INCLUDEPATH += $$PWD

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

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	MOC_DIR = debug/moc
	RCC_DIR = debug/rcc
	UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
	OBJECTS_DIR = release
	MOC_DIR = release/moc
	RCC_DIR = release/rcc
	UI_DIR = release/ui
}


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += main.cpp \
    MonitorMainWindow.cpp \
	MonitorCentralWidget.cpp \
	Settings.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    WorkflowSchemeView.cpp \
    WorkflowSchemeWidget.cpp

HEADERS  += \
    MonitorMainWindow.h \
    MonitorCentralWidget.h \
	Stable.h \
	Settings.h \
    ../include/SocketIO.h \
    DialogSettings.h \
    WorkflowSchemeView.h \
    WorkflowSchemeWidget.h

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

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}
