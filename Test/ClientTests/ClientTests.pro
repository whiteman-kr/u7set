
GOOGLETEST_DIR = ../googletest

include(gtest_dependency.pri)

QT += testlib xml qml core concurrent network
QT -= gui

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
#CONFIG -= qt

include(../../compiler.pri)
include(../../warnings.pri)
include(../../sanitizer.pri)
include(../../codecoverage.pri)


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
        AppSignalManagerTests.cpp \
        main.cpp


# Add curent dir to a list of library directory paths
#
#unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# ClientLib
#
LIBS += -lClientLib
win32:PRE_TARGETDEPS += $$DESTDIR/ClientLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libClientLib.a

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
