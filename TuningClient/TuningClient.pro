
#
# Project created by QtCreator 2016-09-02T10:14:17
#
#-------------------------------------------------

QT       += core gui widgets sql network qml svg xml
 

TARGET = TuningClient
TEMPLATE = app

INCLUDEPATH += $$PWD

include(../compiler.pri)
include(../warnings.pri)
include(../sanitizer.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#Application icon
win32:RC_ICONS += Images/TuningClient.ico

# DESTDIR
# If you see somewhere 'LNK1146: no argument specified with option '/LIBPATH:' then most likely you have not added this section to a project file
#
win32 {
        CONFIG(debug, debug|release): DESTDIR = ../bin/debug
        CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
        CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
        CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}
# /DESTDIR
#

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


SOURCES +=\
    ../lib/ClientBehavior.cpp \
    ../lib/Ui/DialogTcpStatistics.cpp \
    LogonWorkspace.cpp \
	MainWindow.cpp \
    ScriptTuningClientApplication.cpp \
    TuningConfigController.cpp \
    TuningPage.cpp \
    Settings.cpp \
    TuningSignalInfo.cpp \
    TuningWorkspace.cpp \
    ../lib/BuildInfo.cpp \
    DialogSettings.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/PropertyEditorDialog.cpp \
    DialogProperties.cpp \
    ../lib/Ui/DialogSourceInfo.cpp \
    Main.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/Tuning/TuningFilterEditor.cpp \
    ../lib/Tuning/TuningModel.cpp \
    TuningClientFilterStorage.cpp \
    TuningSourcesHelper.cpp \
    SchemasWorkspace.cpp \
    TuningSchemaView.cpp \
    TuningSchemaWidget.cpp \
    DialogFilterEditor.cpp \
    TuningSchemaManager.cpp \
    ../lib/Ui/DialogAlert.cpp \
    ../lib/Ui/DialogAbout.cpp \
    DialogChooseFilter.cpp \
    DialogTuningSources.cpp \
    ../lib/Ui/TuningSourcesWidget.cpp \
    SwitchFiltersPage.cpp \
    SwitchFiltersPageOptions.cpp

HEADERS  += MainWindow.h \
    ../lib/ClientBehavior.h \
    ../lib/ConstStrings.h \
	../UtilsLib/ILogFile.h \
    ../lib/Ui/DialogTcpStatistics.h \
    LogonWorkspace.h \
    ScriptTuningClientApplication.h \
    Stable.h \
    TuningConfigController.h \
    TuningPage.h \
    Settings.h \
    TuningSignalInfo.h \
    TuningWorkspace.h \
    ../lib/BuildInfo.h \
    DialogSettings.h \
    ../lib/PropertyEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../CommonLib/PropertyObject.h \
    DialogProperties.h \
    Main.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/Tuning/TuningFilterEditor.h \
    ../lib/Tuning/TuningModel.h \
    TuningClientFilterStorage.h \
    TuningSourcesHelper.h \
    TuningSchemaView.h \
    TuningSchemaWidget.h \
    SchemasWorkspace.h \
    DialogFilterEditor.h \
    TuningSchemaManager.h \
    ../lib/Ui/DialogAlert.h \
    ../lib/Ui/DialogAbout.h \
    DialogChooseFilter.h \
    DialogTuningSources.h \
    ../lib/Ui/TuningSourcesWidget.h \
    ../lib/Ui/DialogSourceInfo.h \
    SwitchFiltersPage.h \
    SwitchFiltersPageOptions.h

FORMS    += \
    DialogSettings.ui \
    SwitchFiltersPageOptions.ui \
    TuningSignalInfo.ui

TRANSLATIONS = languages/TuningClient_ru.ts

DISTFILES += \
    Images/logo.png \
    Images/TuningClient.ico

RESOURCES += \
    Resources.qrc


# --
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# ClientLib
#
LIBS += -lClientLib
win32:PRE_TARGETDEPS += $$DESTDIR/ClientLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libClientLib.a

# Authorization
#
win32:LIBS += -lAdvapi32
unix:LIBS += -lpam -lpam_misc


# VFrame30 library
#
LIBS += -lVFrame30
win32:PRE_TARGETDEPS += $$DESTDIR/VFrame30.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libVFrame30.a
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# TrendView
#
LIBS += -lTrendView
win32:PRE_TARGETDEPS += $$DESTDIR/TrendView.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libTrendView.a

# HardwareLib
#
LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libHardwareLib.a

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

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../Protobuf

