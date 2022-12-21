QT += testlib core network sql
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

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


SOURCES += \
        ../../Metrology/MetrologyFormula.cpp \
        ../../Metrology/UnitsConvertor.cpp \
        ../../Metrology/UnitsConvertorTable.cpp \
        MetrologyFormulaTests.cpp \
        UnitsConvertorTests.cpp \
        main.cpp

HEADERS += \
    ../../Metrology/MetrologyFormula.h \
    ../../Metrology/UnitsConvertor.h \
    ../../Metrology/UnitsConvertorTable.h \
    MetrologyFormulaTests.h \
    Stable.h \
    UnitsConvertorTests.h


# --
#
LIBS += -L$$DESTDIR

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''


# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

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
