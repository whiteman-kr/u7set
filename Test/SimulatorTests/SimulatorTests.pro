QT += testlib xml qml core concurrent network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES +=  \
    SimCommandTest_LM5_LM6.cpp \
    SimProfilesTest.cpp \
    SimRamTests.cpp \
    main.cpp

HEADERS += \
	Stable.h

# --
#
LIBS += -L$$DESTDIR

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# Simulator Lib
#
LIBS += -lSimulator
INCLUDEPATH += $$PWD/../../Simulator
DEPENDPATH += $$PWD/../../Simulator

# VFrame30 library
#
LIBS += -lVFrame30
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

#protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../../Protobuf


HEADERS += \
    ../../lib/PropertyObject.h \
    SimCommandTest_LM5_LM6.h \
    SimProfilesTest.h \
    SimRamTests.h

RESOURCES += \
    Resources.qrc

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# UtilsLib
#
LIBS += -lCommonLib

# OnlineLib
#
LIBS += -lOnlineLib

# UtilsLib
#
LIBS += -lUtilsLib

# HardwareLib
#
LIBS += -lHardwareLib
