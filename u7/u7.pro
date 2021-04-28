#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT += core gui widgets sql network xmlpatterns qml svg serialport xml printsupport testlib concurrent
win32:QT += winextras

# --
# In Qt 5 using testlib module adds a console option via the MODULE_CONFIG mechanism.
# This forces a /SUBSYSTEM: CONSOLE onto the linker command line no matter what global
# options you specify, even if you use CONFIG -= console.
# The console configuration is given in the testlib module configuration within
# qtbase/src/testlib/testlib.pro. This means that it ends up in QT.testlib.CONFIG variable.

QT.testlib.CONFIG -= console

# --

TARGET = u7
TEMPLATE = app

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../lib

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

#Application icon
win32:RC_ICONS += Images/u7.ico

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


HEADERS  += \
	Stable.h \
    ../lib/ExportPrint.h \
    ../lib/QDoublevalidatorEx.h \
    ../lib/SoftwareXmlReader.h \
    ../lib/StandardColors.h \
    ../lib/Ui/DialogSignalInfo.h \
    ../lib/Ui/DialogSignalSearch.h \
    ../lib/Ui/DialogSignalSnapshot.h \
    ../lib/Ui/DragDropHelper.h \
    ../lib/Ui/FilesTreeView.h \
    ../lib/Ui/SchemaListWidget.h \
    ../lib/Ui/TabWidgetEx.h \
    ../lib/Ui/TagSelectorWidget.h \
	../lib/Ui/DialogProgress.h \
	../lib/OrderedHash.h \
	../lib/PropertyEditor.h \
	../lib/PropertyEditorDialog.h \
	../lib/diff_match_patch.h \
	../lib/Configurator.h \
	../lib/Tuning/TuningFilterEditor.h \
	../lib/Tuning/TuningModel.h \
	../lib/WidgetUtils.h \
	../lib/Ui/DialogAbout.h \
	../lib/AppSignalManager.h \
	../lib/Ui/TextEditCompleter.h \
	../lib/QScintillaLexers/LexerJavaScript.h \
	../lib/QScintillaLexers/LexerXML.h \
	../lib/PropertyTable.h \
	../Metrology/MetrologyConnection.h \
	DlgMetrologyConnection.h \
	CentralWidget.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogClientBehavior.h \
    DialogSettings.h \
    DialogTagsEditor.h \
    EditEngine/EditEngineNop.h \
	EquipmentEditor/EquipmentModel.h \
    EquipmentEditor/EquipmentView.h \
    FilesTabPage.h \
    Forms/DialogProjectDiff.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Reports/ProjectDiffGenerator.h \
    Reports/ReportTools.h \
    Reports/SchemasReportGenerator.h \
    SchemaEditor/EditSchemaSignalProvider.h \
    SchemaEditor/EditSchemaTypes.h \
    SchemaEditor/EditSchemaView.h \
    Settings.h \
    Simulator/SimConnectionPage.h \
    Simulator/SimLogicModulePage.h \
    Simulator/SimOverridePane.h \
    Simulator/SimOverrideValueWidget.h \
    Simulator/SimProfileEditor.h \
    Simulator/SimSignalInfo.h \
    Simulator/SimSelectSchemaPage.h \
    Simulator/SimSignalSnapshot.h \
    Simulator/SimTrend/SimTrends.h \
    TagsEditor.h \
    TestsTabPage.h \
    UserManagementDialog.h \
	EquipmentEditor/EquipmentTabPage.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    SignalsTabPage.h \
    SignalPropertiesDialog.h \
    EditEngine/EditEngine.h \
    EditEngine/EditEngineAddItem.h \
    EditEngine/EditEngineDeleteItem.h \
    EditEngine/EditEngineMoveItem.h \
    EditEngine/EditEngineSetPoints.h \
    EditEngine/EditEngineSetProperty.h \
    BuildTabPage.h \
    DialogFileEditor.h \
    DialogSubsystemListEditor.h \
    Forms/ChooseAfbDialog.h \
	EquipmentEditor/EquipmentVcsDialog.h \
    GlobalMessanger.h \
	SchemaEditor/EditSchemaWidget.h \
	SchemaEditor/SchemaPropertiesDialog.h \
	SchemaEditor/SchemaItemPropertiesDialog.h \
	SchemaEditor/SchemaLayersDialog.h \
	SchemaEditor/CreateSchemaDialog.h \
    EditEngine/EditEngineSetSchemaProperty.h \
    EditEngine/EditEngineSetOrder.h \
    UploadTabPage.h \
	EquipmentEditor/DialogChoosePreset.h \
    DialogSettingsConfigurator.h \
    Forms/ChooseUfbDialog.h \
    Forms/SelectChangesetDialog.h \
    Forms/FileHistoryDialog.h \
    Forms/ChangesetDetailsDialog.h \
    Forms/CompareDialog.h \
    Forms/ComparePropertyObjectDialog.h \
    DialogConnections.h \
    DialogBusEditor.h \
    BusStorage.h \
    Forms/DialogUpdateFromPreset.h \
    IdePropertyEditor.h \
    EditEngine/EditEngineSetObject.h \
	SchemaEditor/EditConnectionLine.h \
    EditEngine/EditEngineBatch.h \
	SchemaEditor/CreateSignalDialog.h \
    SimulatorTabPage.h \
    Simulator/SimIdeSimulator.h \
    Simulator/SimSchemaWidget.h \
    Simulator/SimSchemaManager.h \
    Simulator/SimSchemaView.h \
    Simulator/SimTuningTcpClient.h \
    Simulator/SimCodePage.h \
    Simulator/SimWidget.h \
    Simulator/SimSelectBuildDialog.h \
    Simulator/SimSchemaPage.h \
    Simulator/SimBasePage.h \
    Simulator/SimMemoryWidget.h \
    Simulator/SimOutputWidget.h \
    Simulator/SimProjectWidget.h \
    SpecificPropertiesEditor.h \
	SchemaEditor/SchemaTabPageEx.h \
    DialogAfbLibraryCheck.h \
    Forms/ProjectPropertiesForm.h \
    Forms/PendingChangesDialog.h \
    DialogShortcuts.h \
	SvgEditor.h \

SOURCES +=\
	../lib/ExportPrint.cpp \
	../lib/SoftwareXmlReader.cpp \
	../lib/Ui/DialogSignalInfo.cpp \
	../lib/Ui/DialogSignalSearch.cpp \
	../lib/Ui/DialogSignalSnapshot.cpp \
	../lib/Ui/DragDropHelper.cpp \
	../lib/Ui/FilesTreeView.cpp \
	../lib/Ui/SchemaListWidget.cpp \
	../lib/Ui/TabWidgetEx.cpp \
	../lib/Ui/TagSelectorWidget.cpp \
	../lib/Ui/DialogProgress.cpp \
	../lib/PropertyEditor.cpp \
	../lib/PropertyEditorDialog.cpp \
	../lib/Configurator.cpp \
	../lib/Tuning/TuningFilterEditor.cpp \
	../lib/Tuning/TuningModel.cpp \
	../lib/AppSignalManager.cpp \
	../lib/Ui/DialogAbout.cpp \
	../lib/WidgetUtils.cpp \
	../lib/Ui/TextEditCompleter.cpp \
	../lib/QScintillaLexers/LexerJavaScript.cpp \
	../lib/QScintillaLexers/LexerXML.cpp \
	../lib/PropertyTable.cpp \
	../Metrology/MetrologyConnection.cpp \
	DlgMetrologyConnection.cpp \
	CentralWidget.cpp \
	CreateProjectDialog.cpp \
	CreateUserDialogDialog.cpp \
	DialogClientBehavior.cpp \
	DialogSettings.cpp \
	DialogTagsEditor.cpp \
	EditEngine/EditEngineNop.cpp \
	EquipmentEditor/EquipmentModel.cpp \
	EquipmentEditor/EquipmentView.cpp \
	FilesTabPage.cpp \
	Forms/DialogProjectDiff.cpp \
	LoginDialog.cpp \
	Main.cpp \
	MainTabPage.cpp \
	MainWindow.cpp \
	PasswordService.cpp \
	Reports/ProjectDiffGenerator.cpp \
	Reports/ReportTools.cpp \
	Reports/SchemasReportGenerator.cpp \
	SchemaEditor/EditSchemaSignalProvider.cpp \
	SchemaEditor/EditSchemaTypes.cpp \
	SchemaEditor/EditSchemaView.cpp \
	Settings.cpp \
	Simulator/SimConnectionPage.cpp \
	Simulator/SimLogicModulePage.cpp \
	Simulator/SimOverridePane.cpp \
	Simulator/SimOverrideValueWidget.cpp \
	Simulator/SimProfileEditor.cpp \
	Simulator/SimSignalInfo.cpp \
	Simulator/SimSelectSchemaPage.cpp \
	Simulator/SimSignalSnapshot.cpp \
	Simulator/SimTrend/SimTrends.cpp \
	TagsEditor.cpp \
	TestsTabPage.cpp \
	UserManagementDialog.cpp \
	EquipmentEditor/EquipmentTabPage.cpp \
	CheckInDialog.cpp \
	ProjectsTabPage.cpp \
	SignalsTabPage.cpp \
	SignalPropertiesDialog.cpp \
	EditEngine/EditEngine.cpp \
	EditEngine/EditEngineAddItem.cpp \
	EditEngine/EditEngineDeleteItem.cpp \
	EditEngine/EditEngineMoveItem.cpp \
	EditEngine/EditEngineSetPoints.cpp \
	EditEngine/EditEngineSetProperty.cpp \
	BuildTabPage.cpp \
	DialogFileEditor.cpp \
	DialogSubsystemListEditor.cpp \
	EquipmentEditor/EquipmentVcsDialog.cpp \
	GlobalMessanger.cpp \
	SchemaEditor/EditSchemaWidget.cpp \
	SchemaEditor/SchemaPropertiesDialog.cpp \
	SchemaEditor/SchemaItemPropertiesDialog.cpp \
	SchemaEditor/SchemaLayersDialog.cpp \
	SchemaEditor/CreateSchemaDialog.cpp \
	EditEngine/EditEngineSetSchemaProperty.cpp \
	EditEngine/EditEngineSetOrder.cpp \
	UploadTabPage.cpp \
	EquipmentEditor/DialogChoosePreset.cpp \
	DialogSettingsConfigurator.cpp \
	Forms/ChooseUfbDialog.cpp \
	Forms/SelectChangesetDialog.cpp \
	Forms/FileHistoryDialog.cpp \
	Forms/ChangesetDetailsDialog.cpp \
	Forms/CompareDialog.cpp \
	Forms/ComparePropertyObjectDialog.cpp \
	DialogConnections.cpp \
	DialogBusEditor.cpp \
	BusStorage.cpp \
	Forms/DialogUpdateFromPreset.cpp \
	Forms/ChooseAfbDialog.cpp \
	IdePropertyEditor.cpp \
	EditEngine/EditEngineSetObject.cpp \
	SchemaEditor/EditConnectionLine.cpp \
	EditEngine/EditEngineBatch.cpp \
	SchemaEditor/CreateSignalDialog.cpp \
	SimulatorTabPage.cpp \
	Simulator/SimIdeSimulator.cpp \
	Simulator/SimSchemaWidget.cpp \
	Simulator/SimSchemaManager.cpp \
	Simulator/SimSchemaView.cpp \
	Simulator/SimTuningTcpClient.cpp \
	Simulator/SimCodePage.cpp \
	Simulator/SimWidget.cpp \
	Simulator/SimSelectBuildDialog.cpp \
	Simulator/SimSchemaPage.cpp \
	Simulator/SimProjectWidget.cpp \
	Simulator/SimOutputWidget.cpp \
	Simulator/SimMemoryWidget.cpp \
	Simulator/SimBasePage.cpp \
	SpecificPropertiesEditor.cpp \
	SchemaEditor/SchemaTabPageEx.cpp \
	DialogAfbLibraryCheck.cpp \
	Forms/ProjectPropertiesForm.cpp \
	Forms/PendingChangesDialog.cpp \
	DialogShortcuts.cpp \
	SvgEditor.cpp \

FORMS    += \
    ../lib/Ui/DialogSignalInfo.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    Forms/DialogProjectDiff.ui \
    DialogTagsEditor.ui \
    LoginDialog.ui \
    Simulator/SimSelectBuildDialog.ui \
    UserManagementDialog.ui \
    CheckInDialog.ui \
    DialogSubsystemListEditor.ui \
    Forms/ChooseAfbDialog.ui \
	EquipmentEditor/EquipmentVcsDialog.ui \
	SchemaEditor/CreateSchemaDialog.ui \
	SchemaEditor/SchemaLayersDialog.ui \
	SchemaEditor/SchemaPropertiesDialog.ui \
	SchemaEditor/SchemaItemPropertiesDialog.ui \
	EquipmentEditor/DialogChoosePreset.ui \
    DialogSettingsConfigurator.ui \
    Forms/ChooseUfbDialog.ui \
    Forms/SelectChangesetDialog.ui \
    Forms/FileHistoryDialog.ui \
    Forms/ChangesetDetailsDialog.ui \
    Forms/CompareDialog.ui \
    Forms/ComparePropertyObjectDialog.ui \
    DialogTuningClients.ui

RESOURCES += \
    Resources.qrc \
    ../DatabaseUpgrade/DatabaseUpgrade.qrc

OTHER_FILES += \
    ../Proto/proto_compile.bat \
    ../Proto/serialization.proto \
    Tools/afbschema.xsd \
    ../Proto/proto_compile.sh

DISTFILES += \
    ../Etc/ClientBehavior/ClientBehavior.xml \
    ../Scripts/AIFM/AIFM.js \
    ../Scripts/AIM/AIM.js \
    ../Scripts/AIM_4PH/AIM_4PH.js \
    ../Scripts/AOM/AOM.js \
    ../Scripts/AOM_4PH/AOM_4PH.js \
    ../Scripts/BVB15/BVB15Conf0000.ts \
    ../Scripts/ChildRestriction/ChildRestriction.js \
    ../Scripts/DIM/DIM.js \
    ../Scripts/DOM/DOM.js \
    ../Scripts/FSCChassis/FSCChassis.js \
    ../Scripts/LM1-SF00/LM1_SF00_Conf.ts \
    ../Scripts/MSO3/MSO3Conf0000.ts \
    ../Scripts/MSO4/MSO4_SR21_Conf.ts \
    ../Scripts/OCM/OCM.js \
    ../Scripts/OCMN/OCMN.js \
    ../Scripts/RIM/RIM.js \
    ../Scripts/TIM/TIM.js \
    ../Scripts/WAIM/WAIM.js \
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
    LogicModuleDescription/MSO4_SR21.xml \
    Ufbl/UFB_A3_LANDSCAPE.templ_ufb \
    Ufbl/UFB_A4_LANDSCAPE.templ_ufb \
    Ufbl/file2pgsql.exe \
    Ufbl/_convert_all.bat \
    LogicModuleDescription/LogicModule0000.xml \
    ../Proto/network.proto \
    LogicModuleDescription/LM1_SF00.xml \
    LogicModuleDescription/LM1_SR01.xml \
    LogicModuleDescription/LM1_SF00.xml \
    LogicModuleDescription/LM1_SR01.xml \
    LogicModuleDescription/BVB15Module0000.xml \
    LogicModuleDescription/LM1_SR02.xml \
    ../Etc/SignalPropertyBehavior/SignalPropertyBehavior.csv \
    LogicModuleDescription/MSO3Module0000.xml \
    Images/u7.ico \
    LogicModuleDescription/LM1_SR04.xml \
    LogicModuleDescription/LM1_SR03.xml \
    LogicModuleDescription/LM1_SF40.xml

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Optimization flags
#
win32 {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
    CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
    CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

# --
#
win32:LIBS += -lGdi32
LIBS += -L$$DESTDIR
LIBS += -L.

# QScintilla
#
LIBS += -lQScintilla
INCLUDEPATH += ./../QScintilla/Qt4Qt5
DEPENDPATH += ./../QScintilla/Qt4Qt5
#win32:PRE_TARGETDEPS += $$DESTDIR/QScintilla.lib
#unix:PRE_TARGETDEPS += $$DESTDIR/libQScintilla.a

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# VFrame30 library
#
LIBS += -lVFrame30
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# Builder Lib
#
LIBS += -lBuilder
INCLUDEPATH += $$PWD/../Builder
DEPENDPATH += $$PWD/../Builder

#CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/debug/Builder.lib
#CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/release/Builder.lib

# QtPropertyBrowser
#
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

# QtKeychain
#
INCLUDEPATH += ../Tools/qtkeychain-0.10
include(../Tools/qtkeychain-0.10/qt5keychain.pri)

DEFINES += QTKEYCHAIN_NO_EXPORT
DEFINES += USE_CREDENTIAL_STORE

win32 {
    LIBS += Advapi32.lib
}
unix {
}

# Simulator Lib
#
LIBS += -lSimulator
INCLUDEPATH += $$PWD/../Simulator
DEPENDPATH += $$PWD/../Simulator


# TrendView library
#
LIBS += -lTrendView
INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

# Protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../Protobuf

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
LIBS += -lOnlineLib

# UtilsLib
#
LIBS += -lUtilsLib

# DbLib
#
LIBS += -lDbLib

# HardwareLib
#
LIBS += -lHardwareLib

# CommonLib
#
LIBS += -lCommonLib

# AppSignalLib
#
LIBS += -lAppSignalLib
