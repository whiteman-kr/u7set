QT       += qml sql xml widgets svg

CONFIG += console
CONFIG -= app_bundle

include(../compiler.pri)
include(../warnings.pri)
include(../sanitizer.pri)
include(../codecoverage.pri)

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

SOURCES += \
        main.cpp \
		BuildTask.cpp \

HEADERS += \
	Stable.h \
	BuildTask.h \

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# Builder Lib
#
LIBS += -lBuilder
win32:PRE_TARGETDEPS += $$DESTDIR/Builder.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libBuilder.a
INCLUDEPATH += $$PWD/../Builder
DEPENDPATH += $$PWD/../Builder

# Simulator Lib
#
LIBS += -lSimulator
win32:PRE_TARGETDEPS += $$DESTDIR/Simulator.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libSimulator.a
INCLUDEPATH += $$PWD/../Simulator
DEPENDPATH += $$PWD/../Simulator

# VFrame30 library
#
LIBS += -lVFrame30
win32:PRE_TARGETDEPS += $$DESTDIR/VFrame30.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libVFrame30.a
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# TrendView
#
LIBS += -lTrendView
win32:PRE_TARGETDEPS += $$DESTDIR/TrendView.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libTrendView.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

# DbLib
#
LIBS += -lDbLib
win32:PRE_TARGETDEPS += $$DESTDIR/DbLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libDbLib.a

# HardwareLib
#
LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libHardwareLib.a

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../Protobuf

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a
