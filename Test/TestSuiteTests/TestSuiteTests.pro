QT += testlib xml qml core concurrent network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

TEMPLATE = app

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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES +=  main.cpp

# --
#
LIBS += -L$$DESTDIR

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# TestSuiteLib
#
LIBS += -lTestSuiteLib
win32:PRE_TARGETDEPS += $$DESTDIR/TestSuiteLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libTestSuiteLib.a
INCLUDEPATH += $$PWD/../TestSuiteLib
DEPENDPATH += $$PWD/../TestSuiteLib

# protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../../Protobuf
