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
    Stable.h \
	../../CommonLib/PropertyObject.h \
	SimCommandTest_LM5_LM6.h \
	SimProfilesTest.h \
	SimRamTests.h

RESOURCES += \
    Resources.qrc

# --
#
LIBS += -L$$DESTDIR

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# Simulator Lib
#
LIBS += -lSimulator
win32:PRE_TARGETDEPS += $$DESTDIR/Simulator.lib
INCLUDEPATH += $$PWD/../../Simulator
DEPENDPATH += $$PWD/../../Simulator

# VFrame30 library
#
LIBS += -lVFrame30
win32:PRE_TARGETDEPS += $$DESTDIR/VFrame30.lib
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../../Protobuf

# UtilsLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

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

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
