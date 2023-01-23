QT       += core gui widgets sql network qml xml


TARGET = TestSuite
TEMPLATE = app

INCLUDEPATH += $$PWD

include(../compiler.pri)
include(../warnings.pri)
include(../sanitizer.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#Application icon
#win32:RC_ICONS += Images/TuningClient.ico

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

SOURCES += \
    main.cpp \
    TestSuiteMainWindow.cpp

HEADERS += \
    TestSuiteMainWindow.h

FORMS += \
    TestSuiteMainWindow.ui

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# TestSuiteLib
#
LIBS += -lTestSuiteLib
win32:PRE_TARGETDEPS += $$DESTDIR/TestSuiteLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libTestSuiteLib.a
INCLUDEPATH += $$PWD/../TestSuiteLib
DEPENDPATH += $$PWD/../TestSuiteLib

