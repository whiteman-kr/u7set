QT += core gui widgets sql network qml xml testlib


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

RESOURCES += \
    Resources.qrc

SOURCES += \
    ../lib/BuildInfo.cpp \
    ../lib/Ui/DialogAbout.cpp \
    ../lib/Ui/DialogAlert.cpp \
    ../lib/Ui/DialogTcpStatistics.cpp \
    ../lib/Ui/TabWidgetEx.cpp \
    AppConfigSettings.cpp \
    AppLogOutputWidget.cpp \
    TestListWidget.cpp \
    TestLogTabPage.cpp \
    TestSuiteDialogSettings.cpp \
    TestSuiteLog.cpp \
    main.cpp \
    TestSuiteMainWindow.cpp

HEADERS += \
    ../lib/BuildInfo.h \
    ../lib/Ui/DialogAbout.h \
    ../lib/Ui/DialogAlert.h \
    ../lib/Ui/DialogTcpStatistics.h \
    ../lib/Ui/TabWidgetEx.h \
    AppConfigSettings.h \
    AppLogOutputWidget.h \
    TestListWidget.h \
    TestLogTabPage.h \
    TestSuiteDialogSettings.h \
    TestSuiteLog.h \
    TestSuiteMainWindow.h

FORMS += \
    TestSuiteDialogSettings.ui

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

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

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

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    Images/logo.png
