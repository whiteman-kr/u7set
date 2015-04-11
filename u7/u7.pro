#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml

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


# Force prebuild version control info
#
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
        contains(QMAKE_TARGET.arch, x86_64){
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/u7.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/u7.pro
        }
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/u7.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES +=\
    CentralWidget.cpp \
    ChangesetDialog.cpp \
    ConfigurationsTabPage.cpp \
    CreateProjectDialog.cpp \
    CreateUserDialogDialog.cpp \
    DialogSettings.cpp \
    DialogValueEdit.cpp \
    FilesTabPage.cpp \
    LoginDialog.cpp \
    Main.cpp \
    MainTabPage.cpp \
    MainWindow.cpp \
    PasswordService.cpp \
    Settings.cpp \
    UserManagementDialog.cpp \
    ../lib/ConfigData.cpp \
    ../lib/DbStruct.cpp \
	../lib/DeviceObject.cpp \
	../lib/DbController.cpp \
	../lib/DbWorker.cpp \
	../lib/DbProgressDialog.cpp \
	../lib/StreamedData.cpp \
	../lib/ProtoSerialization.cpp \
	../lib/Signal.cpp \
	../lib/PropertyEditor.cpp \
    EquipmentTabPage.cpp \
    VideoFramePropertiesDialog.cpp \
    CheckInDialog.cpp \
    ProjectsTabPage.cpp \
    DialogAfblEditor.cpp \
    DialogAfbProperties.cpp \
    xmlsyntaxhighlighter.cpp \
    SignalsTabPage.cpp \
    SignalPropertiesDialog.cpp \
    SchemeTabPage.cpp \
    EditSchemeWidget.cpp \
    SchemeItemPropertiesDialog.cpp \
    EditEngine/EditEngine.cpp \
    EditEngine/EditEngineAddItem.cpp \
    EditEngine/EditEngineDeleteItem.cpp \
    EditEngine/EditEngineMoveItem.cpp \
    EditEngine/EditEngineSetPoints.cpp \
    EditEngine/EditEngineSetProperty.cpp \
    SchemePropertiesDialog.cpp \
    EditEngine/EditEngineSetSchemeProperty.cpp \
    ../lib/ModuleConfiguration.cpp \
    BuildTabPage.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbProgress.cpp \
    FileListView.cpp \
    ../lib/Crc.cpp \
    DialogFileEditor.cpp \
    Builder/Builder.cpp \
    Builder/ApplicationLogicBuilder.cpp \
	Builder/BuildResultWriter.cpp \
	SchemeLayersDialog.cpp \
    Builder/ConfigurationBuilder.cpp



HEADERS  += \
    CentralWidget.h \
    ChangesetDialog.h \
    ConfigurationsTabPage.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogSettings.h \
    DialogValueEdit.h \
    FilesTabPage.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Settings.h \
    Stable.h \
    UserManagementDialog.h \
    ../include/ConfigData.h \
    ../include/DbStruct.h \
	../include/DeviceObject.h \
	../include/DbController.h \
	../include/DbWorker.h \
	../include/DbProgressDialog.h \
	../include/StreamedData.h \
	../include/ProtoSerialization.h \
	../include/Factory.h \
	../include/CUtils.h \
	../include/Signal.h \
	../include/OrderedHash.h \
	../include/PropertyEditor.h \
    EquipmentTabPage.h \
    VideoFramePropertiesDialog.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    DialogAfblEditor.h \
    DialogAfbProperties.h \
    xmlsyntaxhighlighter.h \
    SignalsTabPage.h \
    SignalPropertiesDialog.h \
    SchemeTabPage.h \
    EditSchemeWidget.h \
    SchemeItemPropertiesDialog.h \
    EditEngine/EditEngine.h \
    EditEngine/EditEngineAddItem.h \
    EditEngine/EditEngineDeleteItem.h \
    EditEngine/EditEngineMoveItem.h \
    EditEngine/EditEngineSetPoints.h \
    EditEngine/EditEngineSetProperty.h \
    SchemePropertiesDialog.h \
    EditEngine/EditEngineSetSchemeProperty.h \
    ../include/ModuleConfiguration.h \
    BuildTabPage.h \
    ../include/OutputLog.h \
    ../include/DbProgress.h \
    FileListView.h \
    version.h \
    ../include/Crc.h \
    DialogFileEditor.h \
    Builder/Builder.h \
    Builder/ApplicationLogicBuilder.h \
	Builder/BuildResultWriter.h \
    SchemeLayersDialog.h \
    Builder/ConfigurationBuilder.h


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
    DialogAfbProperties.ui \
    SchemeItemPropertiesDialog.ui \
    SchemePropertiesDialog.ui \
    DialogFileEditor.ui \
    SchemeLayersDialog.ui

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
        DatabaseUpgrade/Upgrade0024.sql \
        DatabaseUpgrade/Upgrade0025.sql \
        DatabaseUpgrade/Upgrade0026.sql \
        DatabaseUpgrade/Upgrade0027.sql \
        DatabaseUpgrade/Upgrade0028.sql \
        DatabaseUpgrade/Upgrade0029.sql \
        DatabaseUpgrade/Upgrade0030.sql \
        DatabaseUpgrade/Upgrade0031.sql \
        DatabaseUpgrade/Upgrade0032.sql \
        DatabaseUpgrade/Upgrade0033.sql \
        Tools/afbschema.xsd \
        ../Proto/proto_compile.sh \
    DatabaseUpgrade/Upgrade0034.sql \
    DatabaseUpgrade/Upgrade0035.sql \
    month-report.txt

DISTFILES += \
    LogicModuleConfiguration.js \
    DatabaseUpgrade/Upgrade0036.sql

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

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
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
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
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

# OpenMP
#
#win32 {
#	QMAKE_CXXFLAGS += /openmp
	#CONFIG(debug, debug|release): LIBS += -lvcompd
	#CONFIG(release, debug|release): LIBS += -lvcomp;
#}
#unix {
	#CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	#CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
#}

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


