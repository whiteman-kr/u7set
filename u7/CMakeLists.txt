cmake_minimum_required(VERSION 3.16)

project(u7 LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network Qml Svg SerialPort Xml PrintSupport Test Concurrent DBus)
qt_standard_project_setup()

set (U7_SOURCES
	../Builder/DiagSignalTypesStorage.cpp
	../Builder/DiagSignalTypesStorage.h
	../lib/CodeEditor.cpp
	../lib/CodeEditor.h
	../lib/Configurator.cpp
	../lib/Configurator.h
	../lib/ConstStrings.h
	../lib/diff_match_patch.h
	../lib/ExportPrint.cpp
	../lib/ExportPrint.h
	../lib/PropertyEditor.cpp
	../lib/PropertyEditor.h
	../lib/PropertyEditorDialog.cpp
	../lib/PropertyEditorDialog.h
	../lib/PropertyTable.cpp
	../lib/PropertyTable.h
	../lib/QDoublevalidatorEx.h
	../lib/StandardColors.h
	../lib/Tuning/TuningFilterEditor.cpp
	../lib/Tuning/TuningFilterEditor.h
	../lib/Tuning/TuningModel.cpp
	../lib/Tuning/TuningModel.h
	../lib/Ui/ClickableLabel.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	../lib/Ui/ChooseTagsWidget.cpp
	../lib/Ui/ChooseTagsWidget.h
	../lib/Ui/DialogProgress.cpp
	../lib/Ui/DialogProgress.h
	../lib/Ui/DialogSignalInfo.cpp
	../lib/Ui/DialogSignalInfo.h
	../lib/Ui/DialogSignalSearch.cpp
	../lib/Ui/DialogSignalSearch.h
	../lib/Ui/DialogSignalSnapshot.cpp
	../lib/Ui/DialogSignalSnapshot.h
	../lib/Ui/DialogWriteValues.cpp
	../lib/Ui/DialogWriteValues.h
	../lib/Ui/DragDropHelper.cpp
	../lib/Ui/DragDropHelper.h
	../lib/Ui/FilesTreeView.cpp
	../lib/Ui/FilesTreeView.h
	../lib/Ui/SchemaListWidget.cpp
	../lib/Ui/SchemaListWidget.h
	../lib/Ui/TabWidgetEx.cpp
	../lib/Ui/TabWidgetEx.h
	../lib/Ui/TagSelectorWidget.cpp
	../lib/Ui/TagSelectorWidget.h
	../lib/Ui/TextEditCompleter.cpp
	../lib/Ui/TextEditCompleter.h
	../lib/WidgetUtils.cpp
	../lib/WidgetUtils.h
	AppSignalPropertyManager.cpp
	AppSignalPropertyManager.h
	AppSignalSetProvider.cpp
	AppSignalSetProvider.h
	BuildTabPage.cpp
	BuildTabPage.h
	BusStorage.cpp
	BusStorage.h
	CentralWidget.cpp
	CentralWidget.h
	CheckInDialog.cpp
	CheckInDialog.h
	CheckinSignalsDialog.cpp
	CheckinSignalsDialog.h
	CreateProjectDialog.cpp
	CreateProjectDialog.h
	CreateSignalsDialog.cpp
	CreateSignalsDialog.h
	CreateUserDialogDialog.cpp
	CreateUserDialogDialog.h
	DbTagsEditor.cpp
	DbTagsEditor.h
	DialogAfbLibraryCheck.cpp
	DialogAfbLibraryCheck.h
	DialogBusEditor.cpp
	DialogBusEditor.h
	DialogClientBehavior.cpp
	DialogClientBehavior.h
	DialogConnections.cpp
	DialogConnections.h
	DialogFileEditor.cpp
	DialogFileEditor.h
	DialogMatsUsersEditor.cpp 
	DialogMatsUsersEditor.h
	DialogSettings.cpp
	DialogSettings.h
	DialogSettingsConfigurator.cpp
	DialogSettingsConfigurator.h
	DialogShortcuts.cpp
	DialogShortcuts.h
	DialogSubsystemListEditor.cpp
	DialogSubsystemListEditor.h
	DialogTagsEditor.cpp
	DialogTagsEditor.h
	DlgMetrologyConnection.cpp
	DlgMetrologyConnection.h
	EditEngine/EditEngine.cpp
	EditEngine/EditEngine.h
	EditEngine/EditEngineAddItem.cpp
	EditEngine/EditEngineAddItem.h
	EditEngine/EditEngineBatch.cpp
	EditEngine/EditEngineBatch.h
	EditEngine/EditEngineDeleteItem.cpp
	EditEngine/EditEngineDeleteItem.h
	EditEngine/EditEngineMoveItem.cpp
	EditEngine/EditEngineMoveItem.h
	EditEngine/EditEngineNop.cpp
	EditEngine/EditEngineNop.h
	EditEngine/EditEngineSetObject.cpp
	EditEngine/EditEngineSetObject.h
	EditEngine/EditEngineSetOrder.cpp
	EditEngine/EditEngineSetOrder.h
	EditEngine/EditEngineSetPoints.cpp
	EditEngine/EditEngineSetPoints.h
	EditEngine/EditEngineSetProperty.cpp
	EditEngine/EditEngineSetProperty.h
	EditEngine/EditEngineSetSchemaProperty.cpp
	EditEngine/EditEngineSetSchemaProperty.h
	EquipmentEditor/DialogChoosePreset.cpp
	EquipmentEditor/DialogChoosePreset.h
	EquipmentEditor/DialogImportPreset.cpp
	EquipmentEditor/DialogImportPreset.h
	EquipmentEditor/EquipmentModel.cpp
	EquipmentEditor/EquipmentModel.h
	EquipmentEditor/EquipmentTabPage.cpp
	EquipmentEditor/EquipmentTabPage.h
	EquipmentEditor/EquipmentVcsDialog.cpp
	EquipmentEditor/EquipmentVcsDialog.h
	EquipmentEditor/EquipmentView.cpp
	EquipmentEditor/EquipmentView.h
	FilesTabPage.cpp
	FilesTabPage.h
	FindSignalDialog.cpp
	FindSignalDialog.h
	Forms/ChangesetDetailsDialog.cpp
	Forms/ChangesetDetailsDialog.h
	Forms/ChooseAfbDialog.cpp
	Forms/ChooseAfbDialog.h
	Forms/ChooseUfbDialog.cpp
	Forms/ChooseUfbDialog.h
	Forms/CompareDialog.cpp
	Forms/CompareDialog.h
	Forms/ComparePropertyObjectDialog.cpp
	Forms/ComparePropertyObjectDialog.h
	Forms/DialogDiagSignalTypes.cpp
	Forms/DialogDiagSignalTypes.h
	Forms/DialogProjectDiff.cpp
	Forms/DialogProjectDiff.h
	Forms/DialogUpdateFromPreset.cpp
	Forms/DialogUpdateFromPreset.h
	Forms/FileHistoryDialog.cpp
	Forms/FileHistoryDialog.h
	Forms/PendingChangesDialog.cpp
	Forms/PendingChangesDialog.h
	Forms/ProjectPropertiesForm.cpp
	Forms/ProjectPropertiesForm.h
	Forms/SelectChangesetDialog.cpp
	Forms/SelectChangesetDialog.h
	GlobalMessanger.cpp
	GlobalMessanger.h
	IdePropertyEditor.cpp
	IdePropertyEditor.h
	Locator/AppSignalLocatorProvider.cpp
	Locator/AppSignalLocatorProvider.h
	Locator/ConnectionLocatorProvider.cpp
	Locator/ConnectionLocatorProvider.h
	Locator/EquipmentLocatorProvider.cpp
	Locator/EquipmentLocatorProvider.h
	Locator/Locator.cpp
	Locator/Locator.h
	Locator/LocatorEditControl.cpp
	Locator/LocatorEditControl.h
	Locator/LocatorListWidget.cpp
	Locator/LocatorListWidget.h
	Locator/LocatorProvider.cpp
	Locator/LocatorProvider.h
	Locator/SchemaLocatorProvider.cpp
	Locator/SchemaLocatorProvider.h
	LoginDialog.cpp
	LoginDialog.h
	Main.cpp
	MainTabPage.cpp
	MainTabPage.h
	MainWindow.cpp
	MainWindow.h
	PasswordService.cpp
	PasswordService.h
	ProjectsTabPage.cpp
	ProjectsTabPage.h
	Reports/DialogReportPageSetup.cpp
	Reports/DialogReportPageSetup.h
	Reports/DialogSchemasExport.cpp
	Reports/DialogSchemasExport.h
	Reports/DialogSchemasReport.cpp
	Reports/DialogSchemasReport.h
	Reports/ProjectDiffReport.cpp
	Reports/ProjectDiffReport.h
	Reports/ReportPropertyEditor.cpp
	Reports/ReportPropertyEditor.h
	Reports/SchemasReport.cpp
	Reports/SchemasReport.h
	SchemaEditor/AutoFblItemConnection.cpp
	SchemaEditor/AutoFblItemConnection.h
	SchemaEditor/CreateSchemaDialog.cpp
	SchemaEditor/CreateSchemaDialog.h
	SchemaEditor/CreateSignalDialog.cpp
	SchemaEditor/CreateSignalDialog.h
	SchemaEditor/EditConnectionLine.cpp
	SchemaEditor/EditConnectionLine.h
	SchemaEditor/EditSchemaSignalProvider.cpp
	SchemaEditor/EditSchemaSignalProvider.h
	SchemaEditor/EditSchemaTabPage.cpp
	SchemaEditor/EditSchemaTabPage.h
	SchemaEditor/EditSchemaTypes.cpp
	SchemaEditor/EditSchemaTypes.h
	SchemaEditor/EditSchemaView.cpp
	SchemaEditor/EditSchemaView.h
	SchemaEditor/EditSchemaWidget.cpp
	SchemaEditor/EditSchemaWidget.h
	SchemaEditor/SchemaControlTabPage.cpp
	SchemaEditor/SchemaControlTabPage.h
	SchemaEditor/SchemaItemPropertiesDialog.cpp
	SchemaEditor/SchemaItemPropertiesDialog.h
	SchemaEditor/SchemaLayersDialog.cpp
	SchemaEditor/SchemaLayersDialog.h
	SchemaEditor/SchemaPropertiesDialog.cpp
	SchemaEditor/SchemaPropertiesDialog.h
	SchemaEditor/SchemasTabPage.cpp
	SchemaEditor/SchemasTabPage.h
	Settings.cpp
	Settings.h
	SignalHistoryDialog.cpp
	SignalHistoryDialog.h
	SignalPropertiesDialog.cpp
	SignalPropertiesDialog.h
	SignalsModels.cpp
	SignalsModels.h
	SignalsTabPage.cpp
	SignalsTabPage.h
	Simulator/ScriptSimApplication.cpp
	Simulator/ScriptSimApplication.h
	Simulator/SimBasePage.cpp
	Simulator/SimBasePage.h
	Simulator/SimCodePage.cpp
	Simulator/SimCodePage.h
	Simulator/SimConnectionPage.cpp
	Simulator/SimConnectionPage.h
	Simulator/SimIdeSimulator.cpp
	Simulator/SimIdeSimulator.h
	Simulator/SimLogicModulePage.cpp
	Simulator/SimLogicModulePage.h
	Simulator/SimMemoryWidget.cpp
	Simulator/SimMemoryWidget.h
	Simulator/SimOutputWidget.cpp
	Simulator/SimOutputWidget.h
	Simulator/SimOverridePane.cpp
	Simulator/SimOverridePane.h
	Simulator/SimOverrideValueWidget.cpp
	Simulator/SimOverrideValueWidget.h
	Simulator/SimProfileEditor.cpp
	Simulator/SimProfileEditor.h
	Simulator/SimProjectWidget.cpp
	Simulator/SimProjectWidget.h
	Simulator/SimSchemaManager.cpp
	Simulator/SimSchemaManager.h
	Simulator/SimSchemaPage.cpp
	Simulator/SimSchemaPage.h
	Simulator/SimSchemaView.cpp
	Simulator/SimSchemaView.h
	Simulator/SimSchemaWidget.cpp
	Simulator/SimSchemaWidget.h
	Simulator/SimSelectBuildDialog.cpp
	Simulator/SimSelectBuildDialog.h
	Simulator/SimSelectSchemaPage.cpp
	Simulator/SimSelectSchemaPage.h
	Simulator/SimSignalInfo.cpp
	Simulator/SimSignalInfo.h
	Simulator/SimSignalSnapshot.cpp
	Simulator/SimSignalSnapshot.h
	Simulator/SimTrend/SimTrends.cpp
	Simulator/SimTrend/SimTrends.h
	Simulator/SimWidget.cpp
	Simulator/SimWidget.h
	SimulatorTabPage.cpp
	SimulatorTabPage.h
	SpecificPropertiesEditor.cpp
	SpecificPropertiesEditor.h
	SvgEditor.cpp
	SvgEditor.h
	TestsTabPage.cpp
	TestsTabPage.h
	TuningConnectionStub.cpp
	TuningConnectionStub.h
	UndoSignalsDialog.cpp
	UndoSignalsDialog.h
	UploadTabPage.cpp
	UploadTabPage.h
	UserManagementDialog.cpp
	UserManagementDialog.h
)

set (U7_RESOURCES
	Resources.qrc
	../Etc/Logo/RadiyLogo.qrc
	../DatabaseUpgrade/DatabaseUpgrade.qrc
)

set (U7_FORMS
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/Ui/DialogSignalInfo.ui
	CreateProjectDialog.ui
	CreateUserDialogDialog.ui
	DialogSettings.ui
	EquipmentEditor/DialogImportPreset.ui
	Forms/DialogProjectDiff.ui
	DialogTagsEditor.ui
	LoginDialog.ui
	Simulator/SimSelectBuildDialog.ui
	UserManagementDialog.ui
	CheckInDialog.ui
	DialogSubsystemListEditor.ui
	Forms/ChooseAfbDialog.ui
	Reports/DialogSchemasExport.ui
	EquipmentEditor/EquipmentVcsDialog.ui
	SchemaEditor/CreateSchemaDialog.ui
	SchemaEditor/SchemaLayersDialog.ui
	SchemaEditor/SchemaPropertiesDialog.ui
	SchemaEditor/SchemaItemPropertiesDialog.ui
	EquipmentEditor/DialogChoosePreset.ui
	DialogSettingsConfigurator.ui
	Forms/ChooseUfbDialog.ui
	Forms/SelectChangesetDialog.ui
	Forms/FileHistoryDialog.ui
	Forms/ChangesetDetailsDialog.ui
	Forms/CompareDialog.ui
	Forms/ComparePropertyObjectDialog.ui
	DialogTuningClients.ui
)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set(TARGET u7)

add_executable(${TARGET} WIN32
	${U7_FORMS}
	${U7_SOURCES}
	${U7_RESOURCES}
	${WINDOWS_RESOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	<cassert>
	<cfloat>
	<cstdint>

	<algorithm>
	<array>
	<functional>
	<iterator>
	<limits>
	<list>
	<map>
	<memory>
	<optional>
	<set>
	<type_traits>
	<utility>
	<vector>
	<ranges>

	<QtDebug>
	<QtGui>
	<QtNetwork>
	<QtQml>
	<QtSerialPort>
	<QtSql>
	<QtSvg>
	<QtWidgets>
	<QtXml>
	<QtPrintSupport>

	"../DbLib/DbController.h"
	"../CommonLib/DebugInstCounter.h"
	"../CommonLib/Factory.h"
	"../CommonLib/Types.h"
	"../Proto/ProtoSerialization.h"
	"../Proto/serialization.pb.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Qml	Qt6::Svg
	Qt6::SerialPort Qt6::Xml Qt6::PrintSupport Qt6::Test Qt6::Concurrent Qt6::DBus)

target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib HardwareLib DbLib OnlineLib
	TrendView Simulator qt6keychain Builder VFrame30 ReportLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

target_compile_definitions(${TARGET} PRIVATE QTKEYCHAIN_NO_EXPORT)
