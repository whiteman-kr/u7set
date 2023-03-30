
GOOGLETEST_DIR = ../googletest

include(gtest_dependency.pri)

QT += testlib xml qml core concurrent network gui widgets

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
#CONFIG -= qt

include(../../compiler.pri)
include(../../warnings.pri)
include(../../codecoverage.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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


SOURCES += \
        ../../lib/BuildInfo.cpp \
        AdsConnectionTests.cpp \
        AdsSourceStateConnectionTests.cpp \
        AppSignalManagerTests.cpp \
        ConfigControllerTests.cpp \
        ConnectionPorts.cpp \
        TuningConnectionTests.cpp \
        TuningSignalManagerTests.cpp \
        TuninsUserManagerTests.cpp \
        main.cpp

HEADERS += \
    ../../lib/BuildInfo.h \
    ConnectionPorts.h \
    Stable.h


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
unix:PRE_TARGETDEPS += $$DESTDIR/libSimulator.a

# ClientLib
#
LIBS += -lClientLib
win32:PRE_TARGETDEPS += $$DESTDIR/ClientLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libClientLib.a

# Authorization
#
win32:LIBS += -lAdvapi32
unix:LIBS += -lpam -lpam_misc

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

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

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../../Protobuf

