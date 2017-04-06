#-------------------------------------------------
#
# Project created by QtCreator 2016-09-02T10:14:17
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg xml


TARGET = TuningClient
TEMPLATE = app

INCLUDEPATH += $$PWD

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

#protobuf
#
win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
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


# Force prebuild version control info
#
win32 {
	contains(QMAKE_TARGET.arch, x86_64){
		QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/TuningClient.pro)
	}
	else{
		QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/TuningClient.pro)
	}
}
unix {
	QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
	system(cd $$PWD/../GetGitProjectVersion; \
		qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
		make;)
	system(cd $$PWD; \
		$$PWD/../bin_unix/GetGitProjectVersion $$PWD/TuningClient.pro)
}


SOURCES +=\
        MainWindow.cpp \
    TuningPage.cpp \
    Settings.cpp \
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
    DialogTuningSources.cpp \
    ../Proto/network.pb.cc \
    ../lib/AppSignalState.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/PropertyEditor.cpp \
    ../lib/PropertyEditorDialog.cpp \
    ../lib/PropertyObject.cpp \
    LogFile.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
    UserManager.cpp \
    DialogUsers.cpp \
    DialogProperties.cpp \
    DialogTuningSourceInfo.cpp \
    TuningObjectManager.cpp \
    TuningClientFilterEditor.cpp \
    DialogPassword.cpp \
    Main.cpp \
    ../lib/TuningFilter.cpp \
    ../lib/TuningFilterEditor.cpp \
    ../lib/TuningModel.cpp \
    ../lib/TuningObject.cpp

HEADERS  += MainWindow.h \
    Stable.h \
    TuningPage.h \
    Settings.h \
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
    DialogTuningSources.h \
    ../Proto/network.pb.h \
    ../lib/AppSignalState.h \
    ../Proto/serialization.pb.h \
    ../lib/PropertyEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/PropertyObject.h \
    LogFile.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/OutputLog.h \
    UserManager.h \
    DialogUsers.h \
    DialogProperties.h \
    DialogTuningSourceInfo.h \
    TuningObjectManager.h \
    TuningClientFilterEditor.h \
    DialogPassword.h \
    Main.h \
    ../lib/TuningFilter.h \
    ../lib/TuningFilterEditor.h \
    ../lib/TuningModel.h \
    ../lib/TuningObject.h

FORMS    += \
    DialogSettings.ui \
    DialogTuningSources.ui \
    DialogUsers.ui \
    DialogTuningSourceInfo.ui \
    DialogPassword.ui

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# QtPropertyBrowser
#
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

TRANSLATIONS = languages/TuningClient_en.ts languages/TuningClient_ru.ts
