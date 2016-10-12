#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg serialport

TARGET = u7
TEMPLATE = app

win32:LIBS += -lGdi32

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
    EditEngine/EditEngine.cpp \
    EditEngine/EditEngineAddItem.cpp \
    EditEngine/EditEngineDeleteItem.cpp \
    EditEngine/EditEngineMoveItem.cpp \
    EditEngine/EditEngineSetPoints.cpp \
    EditEngine/EditEngineSetProperty.cpp \
    ../lib/ModuleConfiguration.cpp \
    BuildTabPage.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbProgress.cpp \
    ../lib/Crc.cpp \
    DialogFileEditor.cpp \
    Builder/Builder.cpp \
	Builder/BuildResultWriter.cpp \
    Builder/ConfigurationBuilder.cpp \
    Builder/ApplicationLogicCode.cpp \
	Builder/ApplicationLogicCompiler.cpp \
    DialogSubsystemListEditor.cpp \
    Subsystem.cpp \
    ChooseAfbDialog.cpp \
    EquipmentVcsDialog.cpp \
    ../lib/DataSource.cpp \
    ../lib/SocketIO.cpp \
    Builder/FbParamCalculation.cpp \
    ../lib/PropertyEditorOld.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/Types.cpp \
	Builder/Parser.cpp \
    Connection.cpp \
    DialogConnectionsEditor.cpp \
    ../lib/PropertyEditorDialog.cpp \
    Builder/SoftwareCfgGenerator.cpp \
    ../lib/BuildInfo.cpp \
    Rs232SignalListEditor.cpp \
    Builder/TuningBuilder.cpp \
    Builder/IssueLogger.cpp \
	Builder/OptoModule.cpp \
    GlobalMessanger.cpp \
    Builder/LmMemoryMap.cpp \
    Builder/ModuleLogicCompiler.cpp \
    EditSchemaWidget.cpp \
    SchemaPropertiesDialog.cpp \
    SchemaItemPropertiesDialog.cpp \
    SchemaLayersDialog.cpp \
    SchemaTabPage.cpp \
    CreateSchemaDialog.cpp \
    EditEngine/EditEngineSetSchemaProperty.cpp \
    EditEngine/EditEngineSetOrder.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/XmlHelper.cpp \
    Builder/MonitorCfgGenerator.cpp \
    ../lib/ServiceSettings.cpp \
    Builder/TuningServiceCfgGenerator.cpp \
    Builder/AppDataServiceCfgGenerator.cpp \
    Builder/DiagDataServiceCfgGenerator.cpp \
    ../lib/Queue.cpp \
    ../lib/ProtobufHelper.cpp \
    UploadTabPage.cpp \
    ../TuningService/TuningSource.cpp \
    DialogChoosePreset.cpp \
    ../lib/WUtils.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../lib/DataProtocols.cpp \
    Builder/ModuleFirmwareWriter.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Configurator.cpp \
    DialogSettingsConfigurator.cpp \
    Builder/ArchivingServiceCfgGenerator.cpp \
    Builder/TuningClientCfgGenerator.cpp \
    ChooseUfbDialog.cpp \
    Builder/ModulesRawData.cpp \
    Builder/BdfFile.cpp \
    SchemaListModel.cpp


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
    ../lib/DbStruct.h \
	../lib/DeviceObject.h \
	../lib/DbController.h \
	../lib/DbWorker.h \
	../lib/DbProgressDialog.h \
	../lib/StreamedData.h \
	../lib/ProtoSerialization.h \
	../lib/Factory.h \
	../lib/CUtils.h \
	../lib/Signal.h \
	../lib/OrderedHash.h \
    EquipmentTabPage.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    DialogAfblEditor.h \
    xmlsyntaxhighlighter.h \
    SignalsTabPage.h \
    SignalPropertiesDialog.h \
    EditEngine/EditEngine.h \
    EditEngine/EditEngineAddItem.h \
    EditEngine/EditEngineDeleteItem.h \
    EditEngine/EditEngineMoveItem.h \
    EditEngine/EditEngineSetPoints.h \
    EditEngine/EditEngineSetProperty.h \
    ../lib/ModuleConfiguration.h \
    BuildTabPage.h \
    ../lib/OutputLog.h \
    ../lib/DbProgress.h \
    version.h \
    ../lib/Crc.h \
    DialogFileEditor.h \
    Builder/Builder.h \
	Builder/BuildResultWriter.h \
    Builder/ConfigurationBuilder.h \
    Builder/ApplicationLogicCode.h \
	Builder/ApplicationLogicCompiler.h \
    DialogSubsystemListEditor.h \
    ../lib/Types.h \
    Subsystem.h \
    ChooseAfbDialog.h \
    EquipmentVcsDialog.h \
    ../lib/DataSource.h \
    ../lib/SocketIO.h \
    ../lib/PropertyObject.h \
    ../lib/PropertyEditorOld.h \
    ../lib/PropertyEditor.h \
	Builder/Parser.h \
    Connection.h \
    DialogConnectionsEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/DebugInstCounter.h \
    Builder/SoftwareCfgGenerator.h \
    ../lib/BuildInfo.h \
    Rs232SignalListEditor.h \
    Builder/TuningBuilder.h \
    Builder/IssueLogger.h \
	Builder/OptoModule.h \
	GlobalMessanger.h \
    Builder/LmMemoryMap.h \
    ../lib/Address16.h \
    Builder/ModuleLogicCompiler.h \
    ../lib/WUtils.h \
    EditSchemaWidget.h \
    SchemaPropertiesDialog.h \
    SchemaItemPropertiesDialog.h \
    SchemaTabPage.h \
    SchemaLayersDialog.h \
    CreateSchemaDialog.h \
    EditEngine/EditEngineSetSchemaProperty.h \
    EditEngine/EditEngineSetOrder.h \
    ../lib/DeviceHelper.h \
    ../lib/XmlHelper.h \
    Builder/MonitorCfgGenerator.h \
    ../lib/ServiceSettings.h \
    Builder/TuningServiceCfgGenerator.h \
    Builder/AppDataServiceCfgGenerator.h \
    Builder/DiagDataServiceCfgGenerator.h \
    ../lib/Queue.h \
    ../lib/ProtobufHelper.h \
    UploadTabPage.h \
    ../TuningService/TuningSource.h \
    DialogChoosePreset.h \
    ../TuningService/TuningDataStorage.h \
    ../lib/DataProtocols.h \
    Builder/ModuleFirmwareWriter.h \
    ../lib/HostAddressPort.h \
    ../lib/SignalProperties.h \
    ../lib/Configurator.h \
    DialogSettingsConfigurator.h \
    Builder/ArchivingServiceCfgGenerator.h \
    Builder/TuningClientCfgGenerator.h \
    ChooseUfbDialog.h \
    ../lib/LmLimits.h \
    Builder/ModulesRawData.h \
    Builder/BdfFile.h \
    SchemaListModel.h

FORMS    += \
    ChangesetDialog.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    LoginDialog.ui \
    UserManagementDialog.ui \
    CheckInDialog.ui \
    DialogAfblEditor.ui \
    DialogFileEditor.ui \
    DialogSubsystemListEditor.ui \
    ChooseAfbDialog.ui \
    EquipmentVcsDialog.ui \
    DialogConnectionsEditor.ui \
    CreateSchemaDialog.ui \
    SchemaLayersDialog.ui \
    SchemaPropertiesDialog.ui \
    SchemaItemPropertiesDialog.ui \
    DialogChoosePreset.ui \
    DialogSettingsConfigurator.ui \
    ChooseUfbDialog.ui

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
    Afbl/tct_vibr_v1.afb \
    Ufbl/UFB_A3_LANDSCAPE.templ_ufb \
    Ufbl/UFB_A4_LANDSCAPE.templ_ufb \
    Ufbl/file2pgsql.exe \
    Ufbl/_convert_all.bat

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
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

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

