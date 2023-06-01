QT += gui widgets qml xml testlib

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
    AdsInputController.cpp \
    Control.cpp \
    ScriptRunner.cpp \
    ScriptTestLog.cpp \
    TestController.cpp \
    TestLog.cpp \
    TestReport.cpp \
    TestScriptsStorage.cpp \
    TestSuite.cpp \
    TestSuiteConfigController.cpp \
    TestSuiteSettings.cpp \
    TunsOutputController.cpp

HEADERS += \
    AdsInputController.h \
    Control.h \
    IInputController.h \
    IOutputController.h \
    ScriptRunner.h \
    ScriptTestLog.h \
    Stable.h \
    TestController.h \
    TestLog.h \
    TestReport.h \
    TestScriptsStorage.h \
    TestSuite.h \
    TestSuiteConfigController.h \
    TestSuiteSettings.h \
    TunsOutputController.h

# protobuf
#
INCLUDEPATH += ./../Protobuf
