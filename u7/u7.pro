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



## Force prebuild version control info
##
## for creating version.h at first build
#win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
#unix:system([ -e ./version.h ] || touch ./version.h)
## for any build
#versionTarget.target = version.h
#versionTarget.depends = FORCE
#win32 {
#        contains(QMAKE_TARGET.arch, x86_64){
#            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake & \
#            chdir $$PWD & \
#            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/u7.pro
#        }
#        else{
#            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake & \
#            chdir $$PWD & \
#            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/u7.pro
#        }
#}
#unix {
#    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
#        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
#        make; \
#        cd $$PWD; \
#        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/u7.pro
#}
#PRE_TARGETDEPS += version.h
#QMAKE_EXTRA_TARGETS += versionTarget


SOURCES +=\
    CentralWidget.cpp \
    ChangesetDialog.cpp \
    CreateProjectDialog.cpp \
    CreateUserDialogDialog.cpp \
    DialogSettings.cpp \
    FilesTabPage.cpp \
    LoginDialog.cpp \
    Main.cpp \
    MainTabPage.cpp \
    MainWindow.cpp \
    PasswordService.cpp \
    Settings.cpp \
    UserManagementDialog.cpp \
    ../lib/DbStruct.cpp \
	../lib/DeviceObject.cpp \
	../lib/DbController.cpp \
	../lib/DbWorker.cpp \
	../lib/DbProgressDialog.cpp \
	../lib/StreamedData.cpp \
	../lib/ProtoSerialization.cpp \
	../lib/Signal.cpp \
    EquipmentTabPage.cpp \
    CheckInDialog.cpp \
    ProjectsTabPage.cpp \
    DialogAfblEditor.cpp \
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
    Builder/ConfigurationBuilder.cpp \
    Builder/ApplicationLogicCode.cpp \
	Builder/ApplicationLogicCompiler.cpp \
    DialogSubsystemListEditor.cpp \
    Subsystem.cpp \
    CreateSchemeDialog.cpp \
    ChooseAfbDialog.cpp \
    EquipmentVcsDialog.cpp \
    ../lib/DataSource.cpp \
    ../lib/SocketIO.cpp \
	../lib/PropertyObject.cpp \
    Builder/FbParamCalculation.cpp \
    ../lib/PropertyEditorOld.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/Types.cpp

HEADERS  += \
    CentralWidget.h \
    ChangesetDialog.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogSettings.h \
    FilesTabPage.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Settings.h \
    Stable.h \
    UserManagementDialog.h \
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
    EquipmentTabPage.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    DialogAfblEditor.h \
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
    Builder/ConfigurationBuilder.h \
    Builder/ApplicationLogicCode.h \
	Builder/ApplicationLogicCompiler.h \
    DialogSubsystemListEditor.h \
    ../include/Types.h \
    Subsystem.h \
    CreateSchemeDialog.h \
    ChooseAfbDialog.h \
    EquipmentVcsDialog.h \
    ../include/DataSource.h \
    ../include/SocketIO.h \
    ../include/PropertyObject.h \
    ../include/PropertyEditorOld.h \
    ../include/PropertyEditor.h

FORMS    += \
    ChangesetDialog.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    LoginDialog.ui \
    UserManagementDialog.ui \
    CheckInDialog.ui \
    DialogAfblEditor.ui \
    SchemeItemPropertiesDialog.ui \
    SchemePropertiesDialog.ui \
    DialogFileEditor.ui \
    SchemeLayersDialog.ui \
    DialogSubsystemListEditor.ui \
    CreateSchemeDialog.ui \
    ChooseAfbDialog.ui \
    EquipmentVcsDialog.ui

RESOURCES += \
	Resources.qrc \
    ../DatabaseUpgrade/DatabaseUpgrade.qrc

OTHER_FILES += \
	../Proto/proto_compile.bat \
	../Proto/serialization.proto \
        Tools/afbschema.xsd \
        ../Proto/proto_compile.sh \
	month-report.txt \
    DatabaseUpgrade/Upgrade0039.txt

DISTFILES += \
	LogicModuleConfiguration.js \
    Afbl/_convert_all.bat \
    Afbl/bcomp_great_v1.afb \
    Afbl/bcomp_less_v1.afb \
    Afbl/bcomp_ne_v1.afb \
    Afbl/delay_on_v1.afb \
    Afbl/not_v1.afb \
    Afbl/maj_v1.afb \
    Afbl/or_v1.afb \
    Afbl/xor_v1.afb \
    Afbl/and_v1.afb \
    Afbl/bcomp_eq_v1.afb \
    Afbl/_afbl_all.sql \
    Afbl/tct_filter_v1.afb \
    Afbl/file2pgsql.exe \
    Afbl/tct_off_v1.afb \
    Afbl/tct_on_v1.afb \
    Afbl/tct_vibr_v1.afb

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

win32 {
	#CONFIG(debug, debug|release): DEFINES += _CRTDBG_MAP_ALLOC
	#CONFIG(debug, debug|release): DEFINES += "DBG_NEW=new(_NORMAL_BLOCK,__FILE__,__LINE__)"
	#CONFIG(debug, debug|release): DEFINES += "new=DBG_NEW"
}



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

