QT -= gui

QT += qml xml

TARGET = TestSuiteLib

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD

include(../compiler.pri)
include(../warnings.pri)
include(../codecoverage.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

unix {
    target.path = /usr/lib
        INSTALLS += target
}

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

SOURCES += \
    InputController.cpp \
    OutputController.cpp \
    TestLibrary.cpp \
    TestLog.cpp \
    TestLogController.cpp \
    TestScriptsStorage.cpp \
    TestSuiteConfigController.cpp \
    TestSuiteLib.cpp \
    TestWorker.cpp

HEADERS += \
    InputController.h \
    OutputController.h \
    Stable.h \
    TestLibrary.h \
    TestLog.h \
    TestLogController.h \
    TestScriptsStorage.h \
    TestSuiteConfigController.h \
    TestSuiteLib.h \
    TestWorker.h

# protobuf
#
INCLUDEPATH += ./../Protobuf
