#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network

TARGET = u7
TEMPLATE = app

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD

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

CONFIG += precompile_header

SOURCES +=\
    CentralWidget.cpp \
    ChangesetDialog.cpp \
    ConfigurationsTabPage.cpp \
    CreateProjectDialog.cpp \
    CreateUserDialogDialog.cpp \
    DialogSettings.cpp \
    DialogValueEdit.cpp \
    FilesTabPage.cpp \
    FileView.cpp \
    LoginDialog.cpp \
    Main.cpp \
    MainTabPage.cpp \
    MainWindow.cpp \
    PasswordService.cpp \
    Settings.cpp \
    UserManagementDialog.cpp \
    ../lib/ConfigData.cpp \
    ../lib/DbStore.cpp \
    ../lib/DbStruct.cpp \
    EquipmentTabPage.cpp \
    ../lib/DeviceObject.cpp \
    VideoFramePropertiesDialog.cpp \
    VideoFrameTabPage.cpp \
    EditVideoFrameWidget.cpp \
    EditEngine.cpp \
    EditEngineAddItem.cpp \
    EditEngineSetPoints.cpp \
    EditEngineDeleteItem.cpp \
    EditEngineMoveItem.cpp \
    CheckInDialog.cpp \
    ProjectsTabPage.cpp \
    ../lib/DbController.cpp \
    ../lib/DbWorker.cpp \
    ../lib/DbProgressDialog.cpp

HEADERS  += \
    CentralWidget.h \
    ChangesetDialog.h \
    ConfigurationsTabPage.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogSettings.h \
    DialogValueEdit.h \
    FilesTabPage.h \
    FileView.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Settings.h \
    Stable.h \
    UserManagementDialog.h \
    ../include/ConfigData.h \
    ../include/DbStruct.h \
    ../include/DbStore.h \
    EquipmentTabPage.h \
    ../include/DeviceObject.h \
    VideoFramePropertiesDialog.h \
    VideoFrameTabPage.h \
    EditVideoFrameWidget.h \
    EditEngine.h \
    EditEngineAddItem.h \
    EditEngineSetPoints.h \
    EditEngineDeleteItem.h \
    EditEngineMoveItem.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    ../include/DbController.h \
    ../include/DbWorker.h \
    ../include/DbProgressDialog.h

FORMS    += \
    ChangesetDialog.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    DialogValueEdit.ui \
    LoginDialog.ui \
    UserManagementDialog.ui \
    VideoFramePropertiesDialog.ui \
    CheckInDialog.ui

PRECOMPILED_HEADER = stable.h

# g++ compilator settings
unix:{
    QMAKE_CXXFLAGS += -std=c++11
}

#Optimization flags
#
win32 {
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

# Add curent dir to a list of library directory paths
#
#QMAKE_RPATHDIR += ./
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

# QtPropertyBrowser
#
#include(../QtPropertyBrowser/src/qtpropertybrowser.pri)

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

RESOURCES += \
    Resources.qrc

OTHER_FILES += \
    DatabaseUpgrade/Upgrade0007.sql \
    DatabaseUpgrade/Upgrade0001.sql \
    DatabaseUpgrade/Upgrade0002.sql \
    DatabaseUpgrade/Upgrade0003.sql \
    DatabaseUpgrade/Upgrade0004.sql \
    DatabaseUpgrade/Upgrade0005.sql \
    DatabaseUpgrade/Upgrade0006.sql \
    DatabaseUpgrade/Upgrade0008.sql \
    DatabaseUpgrade/Upgrade0009.sql \
    DatabaseUpgrade/Upgrade0010.sql \
    DatabaseUpgrade/Upgrade0011.sql \
    DatabaseUpgrade/Upgrade0012.sql \
    DatabaseUpgrade/Upgrade0002.sql \
    DatabaseUpgrade/Upgrade0001.sql


# Visual Leak Detector
#
win32 {
	contains(QMAKE_TARGET.arch, x86_64) {
		LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
		LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	} else {
		LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
		LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
	}

	INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
	INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}
