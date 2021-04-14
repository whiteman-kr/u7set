#-------------------------------------------------
#
# Project created by QtCreator 2016-09-02T10:14:17
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg xml
 

TARGET = TuningClient
TEMPLATE = app

INCLUDEPATH += $$PWD

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# generate PDBs for release
#
win32:QMAKE_CXXFLAGS_RELEASE += /Zi
win32:QMAKE_LFLAGS_RELEASE += /DEBUG

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

unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
        CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
        CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
        CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
        CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

#libs
#
win32 {
        LIBS += -L$$DESTDIR -lprotobuf -lKernel32 -lAdvapi32
        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -L$$DESTDIR -lprotobuf -lpam -lpam_misc
		INCLUDEPATH += ./../Protobuf
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


SOURCES +=\
    ../lib/ClientBehavior.cpp \
    ../lib/ComparatorSet.cpp \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/SoftwareSettings.cpp \
        MainWindow.cpp \
    TuningPage.cpp \
    Settings.cpp \
    TuningSignalInfo.cpp \
    TuningWorkspace.cpp \
    ConfigController.cpp \
    ../lib/BuildInfo.cpp \
    DialogSettings.cpp \
    ../lib/AppSignalParam.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/PropertyEditorDialog.cpp \
    ../lib/LogFile.cpp \
    UserManager.cpp \
    DialogProperties.cpp \
    ../lib/Ui/DialogSourceInfo.cpp \
    DialogPassword.cpp \
    Main.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/Tuning/TuningFilterEditor.cpp \
    ../lib/Tuning/TuningModel.cpp \
    TuningClientFilterStorage.cpp \
    SchemasWorkspace.cpp \
    TuningSchemaView.cpp \
    TuningSchemaWidget.cpp \
	../lib/Tuning/TuningSignalManager.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Types.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    DialogFilterEditor.cpp \
    ../lib/Tuning/TuningTcpClient.cpp \
    TuningClientTcpClient.cpp \ 
    ../lib/SoftwareInfo.cpp \
    ../lib/Tuning/TuningLog.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    TuningSchemaManager.cpp \
    ../lib/AppSignal.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Ui/DialogAlert.cpp \
    ../lib/Ui/UiTools.cpp \
    ../lib/Ui/DialogAbout.cpp \
    DialogChooseFilter.cpp \
    DialogTuningSources.cpp \
    ../lib/Ui/TuningSourcesWidget.cpp \
    ../lib/TcpClientsStatistics.cpp \
    SwitchFiltersPage.cpp \
    SwitchFiltersPageOptions.cpp

HEADERS  += MainWindow.h \
    ../lib/ClientBehavior.h \
    ../lib/ComparatorSet.h \
    ../lib/ConstStrings.h \
    ../lib/ILogFile.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/SoftwareSettings.h \
    Stable.h \
    TuningPage.h \
    Settings.h \
    TuningSignalInfo.h \
    TuningWorkspace.h \
    ConfigController.h \
    ../lib/BuildInfo.h \
    DialogSettings.h \
    ../lib/AppSignalParam.h \
    ../lib/PropertyEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/PropertyObject.h \
    ../lib/LogFile.h \
    UserManager.h \
    DialogProperties.h \
    DialogPassword.h \
    Main.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/Tuning/TuningFilterEditor.h \
    ../lib/Tuning/TuningModel.h \
    TuningClientFilterStorage.h \
    TuningSchemaView.h \
    TuningSchemaWidget.h \
    SchemasWorkspace.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/DeviceObject.h \
    ../lib/Types.h \
    ../lib/DbStruct.h \
    ../lib/Tuning/TuningSignalState.h \
    DialogFilterEditor.h \
    ../lib/Tuning/TuningTcpClient.h \
    TuningClientTcpClient.h \ 
    ../lib/SoftwareInfo.h \
    ../lib/Tuning/TuningLog.h \
    ../lib/TuningValue.h \
    ../lib/Tuning/TuningSourceState.h \
    TuningSchemaManager.h \
    ../lib/AppSignal.h \
    ../lib/SignalProperties.h \
    ../lib/Ui/DialogAlert.h \
    ../lib/Ui/UiTools.h \
    ../lib/Ui/DialogAbout.h \
    DialogChooseFilter.h \
    DialogTuningSources.h \
    ../lib/Ui/TuningSourcesWidget.h \
    ../lib/Ui/DialogSourceInfo.h \
    ../lib/TcpClientsStatistics.h \
    SwitchFiltersPage.h \
    SwitchFiltersPageOptions.h

FORMS    += \
    DialogSettings.ui \
    DialogPassword.ui \
    SwitchFiltersPageOptions.ui \
    TuningSignalInfo.ui

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

TRANSLATIONS = languages/TuningClient_ru.ts

DISTFILES += \
    Images/logo.png \
    Images/TuningClient.ico

RESOURCES += \
    Resources.qrc

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lOnlineLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lOnlineLib
}

# UtilsLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lUtilsLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lUtilsLib
}


