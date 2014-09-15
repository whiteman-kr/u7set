#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns

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
    ../lib/DbProgressDialog.cpp \
    AfbLibrary.cpp \
    DialogAfblEditor.cpp \
    DialogAfbProperties.cpp \
    xmlsyntaxhighlighter.cpp \
    SignalsTabPage.cpp \
    ../lib/StreamedData.cpp \
    ../lib/ProtoSerialization.cpp \
	../lib/CUtils.cpp \
    ../lib/Signal.cpp


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
    ../include/DbProgressDialog.h \
    AfbLibrary.h \
    DialogAfblEditor.h \
    DialogAfbProperties.h \
    xmlsyntaxhighlighter.h \
    SignalsTabPage.h \
    ../include/StreamedData.h \
    ../include/ProtoSerialization.h \
    ../include/Factory.h \
    ../include/CUtils.h
    ../include/Signal.h

FORMS    += \
    ChangesetDialog.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    DialogValueEdit.ui \
    LoginDialog.ui \
    UserManagementDialog.ui \
    VideoFramePropertiesDialog.ui \
    CheckInDialog.ui \
    DialogAfblEditor.ui \
    DialogAfbProperties.ui

RESOURCES += \
	Resources.qrc

OTHER_FILES += \
	../Proto/proto_compile.bat \
	../Proto/serialization.proto \
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
	DatabaseUpgrade/Upgrade0001.sql \
	DatabaseUpgrade/Upgrade0013.sql \
	DatabaseUpgrade/Upgrade0014.sql \
	DatabaseUpgrade/Upgrade0015.sql \
	DatabaseUpgrade/Upgrade0016.sql \
	DatabaseUpgrade/Upgrade0017.sql \
	DatabaseUpgrade/Upgrade0018.sql \
	DatabaseUpgrade/Upgrade0019.sql \
	DatabaseUpgrade/Upgrade0020.sql \
	DatabaseUpgrade/Upgrade0021.sql \
    DatabaseUpgrade/Upgrade0022.sql \
    DatabaseUpgrade/Upgrade0023.sql \
    DatabaseUpgrade/Upgrade0024.sql

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#Optimization flags
#
win32 {
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
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

