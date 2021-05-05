QT -= gui
QT += xml qml core concurrent network

CONFIG += console
CONFIG -= app_bundle

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# DESTDIR
# If you see somewhere 'LNK1146: no argument specified with option '/LIBPATH:' then most likely you have not added this section to a project file
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}
# /DESTDIR
#

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += main.cpp \

HEADERS += \
		Stable.h \

RESOURCES += \
    SimulatorConsole.qrc


# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# Simulator Lib
#
LIBS += -lSimulator
win32:PRE_TARGETDEPS += $$DESTDIR/Simulator.lib
INCLUDEPATH += $$PWD/../Simulator
DEPENDPATH += $$PWD/../Simulator

# VFrame30 library
#
LIBS += -lVFrame30
win32:PRE_TARGETDEPS += $$DESTDIR/VFrame30.lib
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../Protobuf

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib

# HardwareLib
#
LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
