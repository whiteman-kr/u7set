#-------------------------------------------------
#
# Project created by QtCreator 2016-09-02T10:14:17
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg xml
 

TARGET = TuningClient
TEMPLATE = app

INCLUDEPATH += $$PWD

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet
win32:QMAKE_CXXFLAGS += /analyze		# Static code analyze


#generate PDBs for release
#
win32:QMAKE_CXXFLAGS_RELEASE += /Zi
win32:QMAKE_LFLAGS_RELEASE += /DEBUG

# Warning level
#
gcc:CONFIG += warn_on

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'

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

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

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
        LIBS += -lprotobuf -lpam -lpam_misc
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
    ../lib/Address16.cpp \
    ../lib/ClientBehavior.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/SoftwareSettings.cpp \
        MainWindow.cpp \
    TuningPage.cpp \
    Settings.cpp \
    TuningSignalInfo.cpp \
    TuningWorkspace.cpp \
    ConfigController.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Crc.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    ../Proto/network.pb.cc \
    ../lib/AppSignal.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/PropertyEditor.cpp \
    ../lib/PropertyEditorDialog.cpp \
    ../lib/LogFile.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
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
    ../lib/CircularLogger.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/Types.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    DialogFilterEditor.cpp \
    ../lib/Tuning/TuningTcpClient.cpp \
    TuningClientTcpClient.cpp \ 
    ../lib/SoftwareInfo.cpp \
    ../lib/Tuning/TuningLog.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    TuningSchemaManager.cpp \
    ../lib/Signal.cpp \
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
    ../lib/Address16.h \
    ../lib/ClientBehavior.h \
    ../lib/ConstStrings.h \
    ../lib/DeviceHelper.h \
    ../lib/ILogFile.h \
    ../lib/SoftwareSettings.h \
    Stable.h \
    TuningPage.h \
    Settings.h \
    TuningSignalInfo.h \
    TuningWorkspace.h \
    ConfigController.h \
    ../lib/HostAddressPort.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/Crc.h \
    ../lib/SocketIO.h \
    DialogSettings.h \
    ../Proto/network.pb.h \
    ../lib/AppSignal.h \
    ../Proto/serialization.pb.h \
    ../lib/PropertyEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/PropertyObject.h \
    ../lib/LogFile.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h \
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
    ../lib/CircularLogger.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/DeviceObject.h \
    ../lib/XmlHelper.h \
    ../lib/Types.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/Tuning/TuningSignalState.h \
    DialogFilterEditor.h \
    ../lib/Tuning/TuningTcpClient.h \
    TuningClientTcpClient.h \ 
    ../lib/SoftwareInfo.h \
    ../lib/Tuning/TuningLog.h \
    ../lib/TuningValue.h \
    ../lib/Tuning/TuningSourceState.h \
    TuningSchemaManager.h \
    ../lib/Signal.h \
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
