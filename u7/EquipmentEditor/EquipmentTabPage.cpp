#include "EquipmentTabPage.h"
#include "../AppSettings.h"
#include "../DialogConnections.h"
#include "../EquipmentEditor/DialogImportPreset.h"
#include "../Forms/ComparePropertyObjectDialog.h"
#include "../Forms/DialogDiagSignalTypes.h"
#include "EquipmentModel.h"
#include "EquipmentVcsDialog.h"
#include "EquipmentView.h"
#include "IdePropertyEditor.h"

#include <HardwareLib/DeviceController.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>

//
//
// EquipmentTabPage
//
//
EquipmentTabPage::EquipmentTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	//
	// Controls
	//

	// Equipment View
	//
	m_equipmentView = new EquipmentView(dbcontroller);
	m_equipmentModel = new EquipmentModel(dbcontroller, this, this);
	m_equipmentView->setModel(m_equipmentModel);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	m_equipmentView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_equipmentView, &EquipmentView::customContextMenuRequested, this, &EquipmentTabPage::onEquipmentViewContextMenuRequested);

	// -----------------
	//
	m_addObjectMenu->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));

	m_addObjectMenu->addAction(m_addSystemAction);
	m_addObjectMenu->addAction(m_addRackAction);
	m_addObjectMenu->addAction(m_addChassisAction);
	m_addObjectMenu->addAction(m_addModuleAction);
	m_addObjectMenu->addAction(m_addControllerAction);
	m_addObjectMenu->addAction(m_addAppSignalAction);
	m_addObjectMenu->addAction(m_addDiagSignalAction);
	m_addObjectMenu->addAction(m_addDiagSignalReflectionAction);
	m_addObjectMenu->addAction(m_addWorkstationAction);
	m_addObjectMenu->addAction(m_addSoftwareAction);

	// -----------------
	//
	m_addPresetMenu->setIcon(QIcon(":/Images/Images/AddPreset.svg"));

	m_addPresetMenu->addAction(m_addPresetRackAction);
	m_addPresetMenu->addAction(m_addPresetChassisAction);
	m_addPresetMenu->addAction(m_addPresetModuleAction);
	m_addPresetMenu->addAction(m_addPresetControllerAction);
	m_addPresetMenu->addAction(m_addPresetWorkstationAction);
	m_addPresetMenu->addAction(m_addPresetSoftwareAction);

	// -----------------

	// These actions have shortcuts, so they are added to m_equipmentView
	//
	m_equipmentView->addAction(m_copyObjectAction);
	m_equipmentView->addAction(m_pasteObjectAction);
	m_equipmentView->addAction(m_findAction);
	m_equipmentView->addAction(m_deleteObjectAction);
	m_equipmentView->addAction(m_refreshAction);

	// Property View
	//

	// Splitter
	//
	m_splitter = new QSplitter(this);

	m_propertyEditor = new IdePropertyEditor(m_splitter, dbcontroller);
	m_propertyTable = new IdePropertyTable(this, dbcontroller);

	QTabWidget* tabWidget = new QTabWidget();
	tabWidget->addTab(m_propertyEditor, "Tree view");
	tabWidget->addTab(m_propertyTable, "Table view");

	tabWidget->setTabPosition(QTabWidget::South);

	m_splitter->addWidget(m_equipmentView);
	m_splitter->addWidget(tabWidget);

	m_splitter->setStretchFactor(0, 2);
	m_splitter->setStretchFactor(1, 1);

	// ToolBar
	//
	m_toolBar = new QToolBar(this);
	m_toolBar->setStyleSheet("QToolButton { padding-top: 3px; padding-bottom: 3px; padding-left: 3px; padding-right: 3px;}");
	m_toolBar->setIconSize(m_toolBar->iconSize() * 0.9);

	m_toolBar->addAction(m_addObjectButton);
	m_toolBar->addAction(m_addFromPresetAction);
	m_toolBar->addAction(m_addNewPresetAction);
	m_toolBar->addAction(m_replaceAction);

	m_separatorActionA = new QAction(tr("Preset"), this);
	m_separatorActionA->setSeparator(true);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_checkOutAction);
	m_toolBar->addAction(m_checkInAction);
	m_toolBar->addAction(m_undoChangesRecursivelyAction);
	m_toolBar->addAction(m_undoChangesAction);
	m_toolBar->addAction(m_historyAction);

	m_toolBar->addAction(m_separatorActionA);
	m_toolBar->addAction(m_refreshAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_switchModeAction);
	m_toolBar->addAction(m_showConnections);

	m_toolBar->addAction(m_separatorPresetExportImport);
	m_toolBar->addAction(m_exportPresetAction);
	m_toolBar->addAction(m_importPresetAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_diagSignalTypesAction);

	// m_toolBar->addAction(m_pendingChangesAction);		// Not implemented, removed to be consistent with User Manual

	//
	// Layouts
	//
	QGridLayout* layout = new QGridLayout();

	auto margins = layout->contentsMargins();
	margins.setTop(0);
	layout->setContentsMargins(margins);

	layout->setMenuBar(m_toolBar); // Set ToolBar here as menu, so no gaps and margins
	layout->addWidget(m_splitter);

	setLayout(layout);

	// Restore state
	//
	{
		QSettings s;

		m_equipmentView->header()->restoreState(s.value("EquipmentTabPage/m_equipmentView/header").toByteArray());
		m_splitter->restoreState(s.value("EquipmentTabPage/m_splitter").toByteArray());

		m_propertyEditor->setSplitterPosition(s.value("EquipmentTabPage/m_propertyEditor/splitterPosition", 150).toInt());
		m_propertyTable->setPropertyFilter(s.value("EquipmentTabPage/m_propertyTable/propertyFilter").toString());
		m_propertyTable->setColumnsWidth(s.value("EquipmentTabPage/m_propertyTable/getColumnsWidth").value<QMap<QString, int>>());
		m_propertyTable->setGroupByCategory(s.value("EquipmentTabPage/m_propertyTable/groupByCategory").toBool());
	}

	// --
	//
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &EquipmentTabPage::clipboardChanged);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &EquipmentTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &EquipmentTabPage::projectClosed);

	connect(m_equipmentView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EquipmentTabPage::selectionChanged);

	// connect(m_equipmentModel, &EquipmentModel::dataChanged, this, &EquipmentTabPage::modelDataChanged);
	connect(m_equipmentView, &EquipmentView::updateState, this, &EquipmentTabPage::setActionState);

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &EquipmentTabPage::propertiesChanged);
	connect(m_propertyTable, &ExtWidgets::PropertyTable::propertiesChanged, this, &EquipmentTabPage::propertiesChanged);

	connect(m_equipmentModel, &EquipmentModel::objectVcsStateChanged, this, &EquipmentTabPage::objectVcsStateChanged);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::findDeviceObject, this, &EquipmentTabPage::findDeviceObject);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &EquipmentTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

EquipmentTabPage::~EquipmentTabPage()
{
	QSettings s;
	s.setValue("EquipmentTabPage/m_equipmentView/header", m_equipmentView->header()->saveState());
	s.setValue("EquipmentTabPage/m_splitter", m_splitter->saveState());
	s.setValue("EquipmentTabPage/m_propertyEditor/splitterPosition", m_propertyEditor->splitterPosition());
	s.setValue("EquipmentTabPage/m_propertyTable/propertyFilter", m_propertyTable->propertyFilter());
	s.setValue("EquipmentTabPage/m_propertyTable/getColumnsWidth", QVariant::fromValue(m_propertyTable->getColumnsWidth()));
	s.setValue("EquipmentTabPage/m_propertyTable/groupByCategory", m_propertyTable->groupByCategory());

	return;
}

void EquipmentTabPage::saveSession() const
{
	m_equipmentView->saveSession();
	return;
}

std::shared_ptr<Hardware::DeviceObject> EquipmentTabPage::deviceObject(QString equipmentId) const
{
	return std::as_const(m_equipmentView)->deviceObject(equipmentId);
}

std::vector<std::shared_ptr<Hardware::DeviceObject>> EquipmentTabPage::deviceObjects(QString equipmentId) const
{
	return std::as_const(m_equipmentView)->deviceObjects(equipmentId);
}


void EquipmentTabPage::CreateActions()
{
	//-------------------------------
	m_addObjectButton = new QAction(tr("Add Object"), this);
	m_addObjectButton->setIcon(QIcon(":/Images/Images/SchemaAddFile.svg"));
	m_addObjectButton->setEnabled(true);

	connect(m_addObjectButton, &QAction::triggered, this, &EquipmentTabPage::addObjectTriggered);

	m_addSystemAction = new QAction(tr("System"), this);
	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
	m_addSystemAction->setEnabled(false);
	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	m_addRackAction = new QAction(tr("Rack"), this);
	m_addRackAction->setStatusTip(tr("Add rack to the configuration..."));
	m_addRackAction->setEnabled(false);
	connect(m_addRackAction, &QAction::triggered, m_equipmentView, &EquipmentView::addRack);

	m_addChassisAction = new QAction(tr("Chassis"), this);
	m_addChassisAction->setStatusTip(tr("Add chassis to the configuration..."));
	m_addChassisAction->setEnabled(false);
	connect(m_addChassisAction, &QAction::triggered, m_equipmentView, &EquipmentView::addChassis);

	m_addModuleAction = new QAction(tr("Module"), this);
	m_addModuleAction->setStatusTip(tr("Add module to the configuration..."));
	m_addModuleAction->setEnabled(false);
	connect(m_addModuleAction, &QAction::triggered, m_equipmentView, &EquipmentView::addModule);

	m_addControllerAction = new QAction(tr("Controller"), this);
	m_addControllerAction->setStatusTip(tr("Add controller to the configuration..."));
	m_addControllerAction->setEnabled(false);
	connect(m_addControllerAction, &QAction::triggered, m_equipmentView, &EquipmentView::addController);

	m_addAppSignalAction = new QAction(tr("AppSignal Port"), this);
	m_addAppSignalAction->setStatusTip(
		tr("Add application signal port to the configuration, AppSignal can be created from this equipment AppSignal Port ..."));
	m_addAppSignalAction->setEnabled(false);
	connect(m_addAppSignalAction, &QAction::triggered, m_equipmentView, &EquipmentView::addAppSignalPort);

	m_addDiagSignalAction = new QAction(tr("DiagSignal"), this);
	m_addDiagSignalAction->setStatusTip(tr("Add diagnostics signal to the configuration..."));
	m_addDiagSignalAction->setEnabled(false);
	connect(m_addDiagSignalAction, &QAction::triggered, m_equipmentView, &EquipmentView::addDiagSignal);

	m_addDiagSignalReflectionAction = new QAction(tr("DiagSignal Reflection"), this);
	m_addDiagSignalReflectionAction->setStatusTip(tr("Add reflection to diagnostics signal to the configuration..."));
	m_addDiagSignalReflectionAction->setEnabled(false);
	connect(m_addDiagSignalReflectionAction, &QAction::triggered, m_equipmentView, &EquipmentView::addDiagSignalReflection);

	m_addWorkstationAction = new QAction(tr("Workstation"), this);
	m_addWorkstationAction->setStatusTip(tr("Add workstation to the configuration..."));
	m_addWorkstationAction->setEnabled(false);
	connect(m_addWorkstationAction, &QAction::triggered, m_equipmentView, &EquipmentView::addWorkstation);

	m_addSoftwareAction = new QAction(tr("Software"), this);
	m_addSoftwareAction->setStatusTip(tr("Add software to the configuration..."));
	m_addSoftwareAction->setEnabled(false);
	connect(m_addSoftwareAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSoftware);


	//----------------------------------
	m_addFromPresetAction = new QAction(tr("Add From Preset..."), this);
	m_addFromPresetAction->setStatusTip(tr("Add preset to the configuration..."));
	m_addFromPresetAction->setIcon(QIcon{":/Images/Images/AddPreset.svg"});
	m_addFromPresetAction->setEnabled(true);
	connect(m_addFromPresetAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPreset);

	m_replaceAction = new QAction(tr("Replace with..."), this);
	m_replaceAction->setStatusTip(tr("Replace selected object with selected one"));
	m_replaceAction->setIcon(QIcon{":/Images/Images/EquipEditorReplace.svg"});
	m_replaceAction->setEnabled(false);
	connect(m_replaceAction, &QAction::triggered, m_equipmentView, &EquipmentView::replaceObject);

	//----------------------------------
	m_addPresetMenu = new QMenu(tr("Create Preset"), this);

	m_addNewPresetAction = new QAction(tr("Create Preset"), this);
	m_addNewPresetAction->setEnabled(true);
	m_addNewPresetAction->setIcon(QIcon{":/Images/Images/AddPreset.svg"});

	connect(m_addNewPresetAction, &QAction::triggered, this, &EquipmentTabPage::addNewPresetTriggered);

	m_addPresetRackAction = new QAction(tr("Preset Rack"), this);
	m_addPresetRackAction->setStatusTip(tr("Add rack to the preset..."));
	m_addPresetRackAction->setEnabled(false);
	connect(m_addPresetRackAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetRack);

	m_addPresetChassisAction = new QAction(tr("Preset Chassis"), this);
	m_addPresetChassisAction->setStatusTip(tr("Add chassis to the preset..."));
	m_addPresetChassisAction->setEnabled(false);
	connect(m_addPresetChassisAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetChassis);

	m_addPresetModuleAction = new QAction(tr("Preset Module"), this);
	m_addPresetModuleAction->setStatusTip(tr("Add module to the preset..."));
	m_addPresetModuleAction->setEnabled(false);
	connect(m_addPresetModuleAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetModule);

	m_addPresetControllerAction = new QAction(tr("Preset Controller"), this);
	m_addPresetControllerAction->setStatusTip(tr("Add controller to the preset..."));
	m_addPresetControllerAction->setEnabled(false);
	connect(m_addPresetControllerAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetController);

	m_addPresetWorkstationAction = new QAction(tr("Preset Workstation"), this);
	m_addPresetWorkstationAction->setStatusTip(tr("Add workstation to the preset..."));
	m_addPresetWorkstationAction->setEnabled(false);
	connect(m_addPresetWorkstationAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetWorkstation);

	m_addPresetSoftwareAction = new QAction(tr("Preset Software"), this);
	m_addPresetSoftwareAction->setStatusTip(tr("Add software to the preset..."));
	m_addPresetSoftwareAction->setEnabled(false);
	connect(m_addPresetSoftwareAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetSoftware);

	//-----------------------------------
	m_separatorAction0 = new QAction(tr("Application Signals"), this);
	m_separatorAction0->setSeparator(true);

	m_createInOutsToSignals = new QAction(tr("Create In/Outs AppSignals"), this);
	m_createInOutsToSignals->setStatusTip(tr("Add inputs/outputs to application logic signals..."));
	m_createInOutsToSignals->setEnabled(false);
	// m_inOutsToSignals->setVisible(false);
	connect(m_createInOutsToSignals, &QAction::triggered, m_equipmentView, QOverload<>::of(&EquipmentView::createInOutsToSignals));

	m_createInternalAppSignal = new QAction(tr("Create Internal AppSignal"), this);
	m_createInternalAppSignal->setStatusTip(tr("Add new internal application signal to device"));
	m_createInternalAppSignal->setEnabled(false);
	connect(m_createInternalAppSignal, &QAction::triggered, m_equipmentView, &EquipmentView::createInternalAppSignal);

	m_showAppSignals = new QAction(tr("Show AppSignals"), this);
	m_showAppSignals->setStatusTip(tr("Show application signals for object and all its children"));
	m_showAppSignals->setEnabled(false);
	connect(m_showAppSignals,
			&QAction::triggered,
			m_equipmentView,
			[ev = m_equipmentView]()
			{
				ev->showAppSignals(false, false);
			});

	//-----------------------------------
	m_separatorSchemaLogic = new QAction(tr("Application Logic"), this);
	m_separatorSchemaLogic->setSeparator(true);

	m_addLogicSchemaToLm = new QAction(tr("Create AppLogic Schema..."), this);
	m_addLogicSchemaToLm->setStatusTip(tr("Create Application Logic Schema for selected module"));
	m_addLogicSchemaToLm->setIcon(QIcon{":/Images/Images/SimAppLogicSchemas.svg"});
	m_addLogicSchemaToLm->setEnabled(false);
	// m_addLogicSchemaToLm->setVisible(false);
	connect(m_addLogicSchemaToLm, &QAction::triggered, m_equipmentView, &EquipmentView::addLogicSchemaToLm);

	m_showLmsLogicSchemas = new QAction(tr("Show AppLogic Schemas..."), this);
	m_showLmsLogicSchemas->setStatusTip(tr("Show Application Logic Schema for selected module"));
	m_showLmsLogicSchemas->setIcon(QIcon{":/Images/Images/SimAppLogicSchemas.svg"});
	m_showLmsLogicSchemas->setEnabled(false);
	// m_showLmsLogicSchemas->setVisible(false);
	connect(m_showLmsLogicSchemas, &QAction::triggered, m_equipmentView, &EquipmentView::showLogicSchemaForLm);

	//-----------------------------------
	m_separatorOptoConnection = new QAction(tr("Connections"), this);
	m_separatorOptoConnection->setSeparator(true);

	m_createConnection = new QAction(tr("Create Connection..."), this);
	m_createConnection->setStatusTip(tr("Create connection for selected port(s)"));
	m_createConnection->setIcon(QIcon{":/Images/Images/SimConnectionIcon.svg"});
	m_createConnection->setEnabled(false);
	// m_addOptoConnection->setVisible(false);
	connect(m_createConnection, &QAction::triggered, m_equipmentView, &EquipmentView::createConnection);


	m_showObjectConnections = new QAction(tr("Show Object Connections..."), this);
	m_showObjectConnections->setStatusTip(tr("Show module or opto-port connections"));
	m_showObjectConnections->setIcon(QIcon{":/Images/Images/SimConnectionIcon.svg"});
	m_showObjectConnections->setEnabled(false);
	// m_showObjectConnections->setVisible(false);
	connect(m_showObjectConnections, &QAction::triggered, m_equipmentView, &EquipmentView::showObjectConnections);

	m_showConnections = new QAction(tr("Connections..."), this);
	m_showConnections->setStatusTip(tr("Edit connections"));
	m_showConnections->setIcon(QIcon{":/Images/Images/SimConnectionIcon.svg"});
	m_showConnections->setEnabled(true);
	connect(m_showConnections, &QAction::triggered, this, &EquipmentTabPage::showConnections);

	//-----------------------------------

	m_copyObjectAction = new QAction(tr("Copy"), this);
	m_copyObjectAction->setStatusTip(tr("Copy equipment to the clipboard..."));
	m_copyObjectAction->setEnabled(false);
	m_copyObjectAction->setShortcut(QKeySequence::Copy);
	m_copyObjectAction->setObjectName("I_am_a_Copy_Action");
	connect(m_copyObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::copySelectedDevices);

	m_pasteObjectAction = new QAction(tr("Paste"), this);
	m_pasteObjectAction->setStatusTip(tr("Paste equipment from the clipboard..."));
	m_pasteObjectAction->setEnabled(false);
	m_pasteObjectAction->setShortcut(QKeySequence::Paste);
	m_pasteObjectAction->setObjectName("I_am_a_Paste_Action");
	connect(m_pasteObjectAction, &QAction::triggered, m_equipmentView, qOverload<>(&EquipmentView::pasteDevices));

	//-----------------------------------

	m_findAction = new QAction(tr("Find..."), this);
	m_findAction->setStatusTip(tr("Find object by EquipmentID"));
	m_findAction->setShortcut(QKeySequence::Find);
	m_findAction->setObjectName("I_am_a_Find_Action");
	connect(m_findAction, &QAction::triggered, m_equipmentView, qOverload<>(&EquipmentView::findObject));

	//-----------------------------------

	m_deleteObjectAction = new QAction(tr("Delete"), this);
	m_deleteObjectAction->setStatusTip(tr("Delete equipment object..."));
	m_deleteObjectAction->setIcon(QIcon{":/Images/Images/SchemaDelete.svg"});
	m_deleteObjectAction->setEnabled(false);
	m_deleteObjectAction->setShortcut(QKeySequence::Delete);
	connect(m_deleteObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::deleteSelectedDevices);

	//-----------------------------------

	m_checkOutAction = new QAction(tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out device for edit"));
	m_checkOutAction->setIcon(QIcon{":/Images/Images/SchemaCheckOut.svg"});
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_equipmentView, &EquipmentView::checkOutSelectedDevices);

	m_checkInAction = new QAction(tr("CheckIn..."), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setIcon(QIcon{":/Images/Images/SchemaCheckIn.svg"});
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_equipmentView, &EquipmentView::checkInSelectedDevices);

	m_undoChangesRecursivelyAction = new QAction(tr("Undo Tree Changes..."), this);
	m_undoChangesRecursivelyAction->setStatusTip(
		tr("Roll back all modifications made to this object and its child objects, if any, reverting them to their previous state. Object "
		   "is removed if it was not checked in before."));
	m_undoChangesRecursivelyAction->setIcon(QIcon{":/Images/Images/SchemaUndoTree.svg"});
	m_undoChangesRecursivelyAction->setEnabled(false);
	connect(m_undoChangesRecursivelyAction, &QAction::triggered, m_equipmentView, &EquipmentView::undoChangesRecursively);

	m_undoChangesAction = new QAction(tr("Undo Object Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setIcon(QIcon{":/Images/Images/SchemaUndo.svg"});
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_equipmentView, &EquipmentView::undoChangesSelectedDevices);

	m_historyAction = new QAction(tr("History..."), this);
	m_historyAction->setStatusTip(tr("Show check in history"));
	m_historyAction->setIcon(QIcon{":/Images/Images/SchemaHistory.svg"});
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, m_equipmentView, &EquipmentView::showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare file"));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, m_equipmentView, &EquipmentView::compare);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setIcon(QIcon{":/Images/Images/SchemaRefresh.svg"});
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, m_equipmentView, &EquipmentView::refreshSelectedDevices);
	addAction(m_refreshAction);

	//-----------------------------------

	m_updateFromPresetAction = new QAction(tr("Update from Preset"), this);
	m_updateFromPresetAction->setStatusTip(tr("Update from all object from preset"));
	m_updateFromPresetAction->setEnabled(true);
	connect(m_updateFromPresetAction, &QAction::triggered, m_equipmentView, &EquipmentView::updateFromPreset);

	m_switchModeAction = new QAction(tr("Switch to Preset"), this);
	m_switchModeAction->setStatusTip(tr("Switch to preset/configuration mode"));
	m_switchModeAction->setEnabled(true);
	connect(m_switchModeAction, &QAction::triggered, m_equipmentModel, &EquipmentModel::switchMode);
	connect(m_switchModeAction, &QAction::triggered, this, &EquipmentTabPage::modeSwitched);

	m_pendingChangesAction = new QAction(tr("Pending Changes..."), this);
	m_pendingChangesAction->setStatusTip(tr("Show pending changes"));
	m_pendingChangesAction->setEnabled(true);
	// connect(m_pendingChangesAction, &QAction::triggered, m_equipmentModel, &EquipmentModel::pendingChanges);
	connect(m_pendingChangesAction, &QAction::triggered, this, &EquipmentTabPage::pendingChanges);

	//-----------------------------------
	m_SeparatorAction4 = new QAction(this);
	m_SeparatorAction4->setSeparator(true);

	m_separatorPresetExportImport = new QAction{this};
	m_separatorPresetExportImport->setSeparator(true);
	m_separatorPresetExportImport->setVisible(false);

	m_exportPresetAction = new QAction{tr("Export Preset..."), this};
	m_exportPresetAction->setStatusTip(tr("Export selected preset to file"));
	m_exportPresetAction->setIcon(QIcon{":/Images/Images/ExportPreset.svg"});
	m_exportPresetAction->setVisible(false);
	connect(m_exportPresetAction, &QAction::triggered, this, &EquipmentTabPage::exportPreset);

	m_importPresetAction = new QAction{tr("Import Preset..."), this};
	m_importPresetAction->setStatusTip(tr("Import preset from file"));
	m_importPresetAction->setIcon(QIcon{":/Images/Images/ImportPreset.svg"});
	m_importPresetAction->setVisible(false);
	connect(m_importPresetAction, &QAction::triggered, this, &EquipmentTabPage::importPreset);

	m_diagSignalTypesAction = new QAction{tr("Diagnostic Signal Types"), this};
	m_diagSignalTypesAction->setStatusTip(tr("Run Diagnostic Signal Types Editor"));
	m_diagSignalTypesAction->setIcon(QIcon{":/Images/Images/DiagSignalTypes.svg"});
	connect(m_diagSignalTypesAction, &QAction::triggered, this, &EquipmentTabPage::diagSignalTypes);

	return;
}

bool EquipmentTabPage::isPresetMode() const
{
	return m_equipmentModel->isPresetMode();
}

bool EquipmentTabPage::isConfigurationMode() const
{
	return m_equipmentModel->isConfigurationMode();
}

void EquipmentTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void EquipmentTabPage::projectOpened()
{
	this->setEnabled(true);
	selectionChanged(QItemSelection(), QItemSelection());
	return;
}

void EquipmentTabPage::projectClosed()
{
	this->setEnabled(false);
	m_propertyEditor->clear();
	m_propertyTable->clear();
	return;
}

void EquipmentTabPage::selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	setActionState();
	setProperties();
	return;
}

void EquipmentTabPage::modelDataChanged(const QModelIndex& /*topLeft*/,
										const QModelIndex& /*bottomRight*/,
										const QVector<int>& /*roles = QVector<int>()*/)
{
	return setActionState();
}

void EquipmentTabPage::setActionState()
{
	assert(m_addFromPresetAction);
	assert(m_replaceAction);
	assert(m_addNewPresetAction);
	assert(m_addSystemAction);
	assert(m_addSystemAction);
	assert(m_addChassisAction);
	assert(m_addModuleAction);
	assert(m_addControllerAction);
	assert(m_addAppSignalAction);
	assert(m_addDiagSignalAction);
	assert(m_addDiagSignalReflectionAction);
	assert(m_addWorkstationAction);
	assert(m_addSoftwareAction);
	assert(m_copyObjectAction);
	assert(m_pasteObjectAction);
	assert(m_deleteObjectAction);
	assert(m_checkOutAction);
	assert(m_checkInAction);
	assert(m_undoChangesRecursivelyAction);
	assert(m_undoChangesAction);
	assert(m_historyAction);
	assert(m_compareAction);
	assert(m_refreshAction);
	assert(m_addPresetRackAction);
	assert(m_addPresetChassisAction);
	assert(m_addPresetModuleAction);
	assert(m_addPresetControllerAction);
	assert(m_addPresetWorkstationAction);
	assert(m_addPresetSoftwareAction);
	assert(m_createInOutsToSignals);
	assert(m_showAppSignals);
	assert(m_createInternalAppSignal);
	assert(m_addLogicSchemaToLm);
	assert(m_showLmsLogicSchemas);
	assert(m_createConnection);
	assert(m_showObjectConnections);
	assert(m_exportPresetAction);
	assert(m_importPresetAction);
	assert(m_diagSignalTypesAction);

	// Check in is always true, as we perform check in is performed for the tree, and there is no information
	// about does parent have any checked out files
	//
	m_checkInAction->setEnabled(true);
	m_deleteObjectAction->setEnabled(true); // Allow to TRY to delete always. Even part of preset in editConfigurationMode,
											// It can be useful to delete preset with all it's children, especially if it was
											// just created, the it will remove from the DB all records.

	if (isPresetMode() == true)
	{
		m_addNewPresetAction->setVisible(true);
		m_addFromPresetAction->setVisible(false);
		m_replaceAction->setVisible(false);
	}
	else
	{
		m_addNewPresetAction->setVisible(false);
		m_addFromPresetAction->setVisible(true);
		m_replaceAction->setVisible(true);
	}

	// Disable all
	//
	m_addSystemAction->setEnabled(false);
	m_addRackAction->setEnabled(false);
	m_addChassisAction->setEnabled(false);
	m_addModuleAction->setEnabled(false);
	m_addControllerAction->setEnabled(false);
	m_addAppSignalAction->setEnabled(false);
	m_addDiagSignalAction->setEnabled(false);
	m_addDiagSignalReflectionAction->setEnabled(false);

	m_addWorkstationAction->setEnabled(false);
	m_addSoftwareAction->setEnabled(false);

	m_checkOutAction->setEnabled(false);
	// m_checkInAction->setEnabled(false);			// Check in is always true, as we perform check in is performed for the tree, and there
	// is no information
	m_undoChangesRecursivelyAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_historyAction->setEnabled(false);
	m_compareAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	m_addPresetRackAction->setEnabled(false);
	m_addPresetChassisAction->setEnabled(false);
	m_addPresetModuleAction->setEnabled(false);
	m_addPresetControllerAction->setEnabled(false);
	m_addPresetWorkstationAction->setEnabled(false);
	m_addPresetSoftwareAction->setEnabled(false);

	m_replaceAction->setEnabled(false);

	m_createInOutsToSignals->setEnabled(false);
	m_showAppSignals->setEnabled(false);
	m_createInternalAppSignal->setEnabled(false);
	m_addLogicSchemaToLm->setEnabled(false);

	m_showLmsLogicSchemas->setEnabled(false);
	m_showLmsLogicSchemas->setVisible(false);

	m_createConnection->setEnabled(false);
	m_showObjectConnections->setEnabled(false);

	m_copyObjectAction->setEnabled(false);

	m_exportPresetAction->setEnabled(false);
	m_importPresetAction->setEnabled(false);

	m_diagSignalTypesAction->setEnabled(false);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	m_addFromPresetAction->setEnabled(selectedIndexList.size() == 1);

	// Refresh
	//
	m_refreshAction->setEnabled(true);

	// Add inputs/outputs to signals
	//
	if (isConfigurationMode() == true && selectedIndexList.size() == 1)
	{
		// Allow to add signals for all module
		//
		auto device = m_equipmentModel->deviceObject(selectedIndexList.front());
		assert(device);

		if (device->isModule() == true)
		{
			auto module = device->toModule();
			assert(module);

			// Allow to add signals for any module
			// it comes from MUM, it is not IO module, it is control
			//
			m_createInOutsToSignals->setEnabled(true);
			m_createInOutsToSignals->setVisible(true);

			if (module->isLogicModule() == true)
			{
				m_createInternalAppSignal->setEnabled(true);
				m_createInternalAppSignal->setVisible(true);
			}
		}

		if (device->presetRoot() == true)
		{
			m_replaceAction->setEnabled(true);
		}
	}

	if (m_createInOutsToSignals->isEnabled() == false && // Could be already enabled in prev condition
		isConfigurationMode() == true && selectedIndexList.isEmpty() == false)
	{
		// Allow to add selected signals
		//
		bool allSelectedAreAppSignals = true;

		for (auto& si : selectedIndexList)
		{
			auto device = m_equipmentModel->deviceObject(si);
			assert(device);

			if (device->isAppSignal() == false)
			{
				allSelectedAreAppSignals = false;
				break;
			}
		}

		m_createInOutsToSignals->setEnabled(allSelectedAreAppSignals);
		m_createInOutsToSignals->setVisible(true);
	}

	// Add AppLogic Schema to LM
	//
	if (isConfigurationMode() == true && selectedIndexList.size() >= 1)
	{
		std::optional<QString> lmDescriptionFile;

		bool allSelectedHaveSameLmDescriptionFile =
			std::all_of(selectedIndexList.begin(),
						selectedIndexList.end(),
						[this, &lmDescriptionFile](const QModelIndex& mi)
						{
							auto device = m_equipmentModel->deviceObject(mi);
							if (device == nullptr || device->isModule() == false)
							{
								assert(device);
								return false;
							}

							QString thisLmDescriptionFile = device->propertyValue(Hardware::PropertyNames::lmDescriptionFile).toString();
							if (lmDescriptionFile.has_value() == false)
							{
								lmDescriptionFile = thisLmDescriptionFile;
							}

							return thisLmDescriptionFile.isEmpty() == false && lmDescriptionFile.value() == thisLmDescriptionFile;
						});

		bool allSelectedAreLMs = allSelectedHaveSameLmDescriptionFile == true && lmDescriptionFile.has_value() == true &&
								 lmDescriptionFile.value().isEmpty() == false;

		m_addLogicSchemaToLm->setEnabled(allSelectedAreLMs);
		m_addLogicSchemaToLm->setVisible(allSelectedAreLMs);
	}

	// Show logic schemas for selected LM
	//
	if (isConfigurationMode() == true && selectedIndexList.size() == 1)
	{
		auto device = m_equipmentModel->deviceObject(selectedIndexList.front());
		assert(device);

		if (device->isModule() == true)
		{
			if (auto module = device->toModule(); module->isFSCConfigurationModule() == true)
			{
				m_showLmsLogicSchemas->setEnabled(true);
				m_showLmsLogicSchemas->setVisible(true);
			}
		}
	}

	// Add Opto Connection with TWO selected Opto Ports
	//
	if (isConfigurationMode() == true && selectedIndexList.size() == 1)
	{
		auto controller1 = m_equipmentModel->deviceObject(selectedIndexList.front())->toController();

		if (controller1 != nullptr)
		{
			// If parent is LM or OCM
			//
			if (controller1->parent()->isModule() == true)
			{
				auto parent1 = controller1->parent()->toModule();
				assert(parent1);

				if (parent1->isLogicModule() == true || parent1->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM)
				{
					// If id ends with _OPTOPORTXX
					//
					QString id1 = controller1->equipmentIdTemplate();
					if (id1.size() > 10)
					{
						id1.replace(id1.size() - 2, 2, QLatin1String("##"));
					}

					if (id1.endsWith("_OPTOPORT##") == true)
					{
						m_createConnection->setEnabled(true);
						m_createConnection->setVisible(true);
					}
				}
			}
		}
	}

	if (isConfigurationMode() == true && selectedIndexList.size() == 2)
	{
		auto controller1 = m_equipmentModel->deviceObject(selectedIndexList.front())->toController();
		auto controller2 = m_equipmentModel->deviceObject(selectedIndexList.back())->toController();

		if (controller1 != nullptr && controller2 != nullptr)
		{
			// If parent is LM or OCM
			//
			auto parent1 = controller1->parent()->toModule();
			auto parent2 = controller2->parent()->toModule();

			if (parent1 != nullptr && parent2 != nullptr &&
				(parent1->isLogicModule() == true || parent1->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM) &&
				(parent2->isLogicModule() == true || parent2->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM))
			{
				// If id ends with _OPTOPORTXX
				//
				QString id1 = controller1->equipmentIdTemplate();
				if (id1.size() > 10)
				{
					id1.replace(id1.size() - 2, 2, QLatin1String("##"));
				}

				QString id2 = controller2->equipmentIdTemplate();
				if (id2.size() > 10)
				{
					id2.replace(id2.size() - 2, 2, QLatin1String("##"));
				}

				if (id1.endsWith("_OPTOPORT##") == true && id2.endsWith("_OPTOPORT##") == true)
				{
					m_createConnection->setEnabled(true);
					m_createConnection->setVisible(true);
				}
			}
		}
	}

	// Show opto connections for selected LM/OCM or selected opto port
	//
	if (isConfigurationMode() == true && selectedIndexList.size() > 0)
	{
		m_showObjectConnections->setEnabled(true);
		m_showObjectConnections->setVisible(true);
	}

	// Show Application Logic Signal for current object
	//
	if (selectedIndexList.size() > 0)
	{
		m_showAppSignals->setEnabled(true);
	}

	// Export/import preset
	//
	// Show logic schemas for selected LM
	//
	if (isPresetMode() == true)
	{
		m_importPresetAction->setEnabled(db()->currentUser().isDisabled() == false && db()->currentUser().isReadonly() == false);

		// Enable if all selected are preset roots
		//
		bool selectedArePresetRoots = selectedIndexList.size() > 0;

		for (auto sli : selectedIndexList)
		{
			auto device = m_equipmentModel->deviceObject(sli);
			assert(device);

			if (device == nullptr || device->isPreset() != true || device->presetRoot() != true)
			{
				selectedArePresetRoots = false;
				break;
			}
		}

		m_exportPresetAction->setEnabled(selectedArePresetRoots);
	}

	// CheckIn, CheckOut
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		auto device = m_equipmentModel->deviceObject(mi);
		assert(device);

		const DbFileInfo* fileInfo = device->data();
		Q_ASSERT(fileInfo);

		if (fileInfo->state() == E::VcsState::CheckedOut &&
			(fileInfo->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			canAnyBeCheckedIn = true;
		}

		if (fileInfo->state() == E::VcsState::CheckedIn)
		{
			canAnyBeCheckedOut = true;
		}

		// Don't need to go further
		//
		if (canAnyBeCheckedIn == true && canAnyBeCheckedOut == true)
		{
			break;
		}
	}

	// m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesRecursivelyAction->setEnabled(selectedIndexList.size() == 1);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1);
	m_compareAction->setEnabled(selectedIndexList.size() == 1);

	// Enable possible creation items;
	//
	if (selectedIndexList.size() > 1)
	{
		// Don't know after which item possible to insert new Device
		//
	}
	else
	{
		if (selectedIndexList.empty() == true)
		{
			m_addSystemAction->setEnabled(true);
		}
		else
		{
			QModelIndex singleSelectedIndex = selectedIndexList[0];

			auto selectedObject = m_equipmentModel->deviceObject(singleSelectedIndex);
			assert(selectedObject != nullptr);

			if (isPresetMode() == true && selectedObject->isPreset() == false)
			{
				assert(false);
				return;
			}

			m_addSystemAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::System));
			m_addRackAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Rack));
			m_addChassisAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Chassis));
			m_addModuleAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Module));
			m_addWorkstationAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Workstation));
			m_addSoftwareAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Software));
			m_addControllerAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Controller));
			m_addAppSignalAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::AppSignal));
			m_addDiagSignalAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::DiagSignal));
			m_addDiagSignalReflectionAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::DiagSignal));

			m_addPresetRackAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Rack));
			m_addPresetChassisAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Chassis));
			m_addPresetModuleAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Module));
			m_addPresetControllerAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Controller));
			m_addPresetWorkstationAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Workstation));
			m_addPresetSoftwareAction->setEnabled(selectedObject->canAddChild(Hardware::DeviceType::Software));
		}
	}

	// Enable elements in preset mode
	//
	if (isPresetMode() == true)
	{
		m_addSystemAction->setEnabled(false);

		m_addPresetRackAction->setEnabled(true);
		m_addPresetChassisAction->setEnabled(true);
		m_addPresetModuleAction->setEnabled(true);
		m_addPresetControllerAction->setEnabled(true);
		m_addPresetWorkstationAction->setEnabled(true);
		m_addPresetSoftwareAction->setEnabled(true);
	}

	// Copy to the clipboard
	//
	if (selectedIndexList.empty() == false)
	{
		bool allowCopyToClipboard = true; // allow copy if all selected objects are the same type

		auto firstSelectedDevice = m_equipmentModel->deviceObject(selectedIndexList.first());
		Q_ASSERT(firstSelectedDevice);

		Hardware::DeviceType type = firstSelectedDevice->deviceType();

		for (const QModelIndex& mi : selectedIndexList)
		{
			auto device = m_equipmentModel->deviceObject(mi);
			Q_ASSERT(device);

			// System can be very big so forbid it's copying
			//
			if (device->isSystem() == true)
			{
				allowCopyToClipboard = false;
				break;
			}

			// all selected objects must be the same type
			//
			if (type != device->deviceType())
			{
				allowCopyToClipboard = false;
				break;
			}

			// In ConfigurationMode it is possible to copy only root items of preset items
			//
			if (isConfigurationMode() == true && device->isPreset() == true && device->presetRoot() == false)
			{
				allowCopyToClipboard = false;
				break;
			}
		}

		m_copyObjectAction->setEnabled(allowCopyToClipboard);
	}

	// Update paste
	//
	bool enablePaste = m_equipmentView->canPaste();
	m_pasteObjectAction->setEnabled(enablePaste);

	// Always enable to edit diag signa types.
	//
	m_diagSignalTypesAction->setEnabled(true);

	return;
}

void EquipmentTabPage::clipboardChanged()
{
	bool enablePaste = m_equipmentView->canPaste();
	m_pasteObjectAction->setEnabled(enablePaste);

	return;
}


void EquipmentTabPage::modeSwitched()
{
	if (m_equipmentModel->isPresetMode() == true)
	{
		m_switchModeAction->setText(tr("Switch to Configuration"));

		m_updateFromPresetAction->setEnabled(false);
	}
	else
	{
		m_switchModeAction->setText(tr("Switch to Preset"));

		m_updateFromPresetAction->setEnabled(true);
	}

	setActionState();

	// Show/hide some actions
	//
	const bool visibleEditModeActions = isConfigurationMode();

	m_separatorAction0->setVisible(visibleEditModeActions);
	m_createInOutsToSignals->setVisible(visibleEditModeActions);
	m_showAppSignals->setVisible(visibleEditModeActions);
	m_createInternalAppSignal->setVisible(visibleEditModeActions);

	m_separatorAction0->setVisible(visibleEditModeActions);
	m_createInOutsToSignals->setVisible(visibleEditModeActions);
	m_showAppSignals->setVisible(visibleEditModeActions);
	m_createInternalAppSignal->setVisible(visibleEditModeActions);

	m_separatorSchemaLogic->setVisible(visibleEditModeActions);
	m_addLogicSchemaToLm->setVisible(visibleEditModeActions);
	m_showLmsLogicSchemas->setVisible(visibleEditModeActions);

	m_separatorOptoConnection->setVisible(visibleEditModeActions);
	m_createConnection->setVisible(visibleEditModeActions);
	m_showObjectConnections->setVisible(visibleEditModeActions);
	m_showConnections->setVisible(visibleEditModeActions);

	m_separatorPresetExportImport->setVisible(isPresetMode());
	m_exportPresetAction->setVisible(isPresetMode());
	m_importPresetAction->setVisible(isPresetMode());

	return;
}

void EquipmentTabPage::showConnections()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	if (theDialogConnections == nullptr)
	{
		theDialogConnections = new DialogConnections(dbController(), this);
		theDialogConnections->show();
	}
	else
	{
		theDialogConnections->activateWindow();
	}

	return;
}

void EquipmentTabPage::setProperties()
{
	assert(m_propertyEditor != nullptr);
	assert(m_propertyTable != nullptr);

	// Get objects from the selected rows
	//
	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	if (selectedIndexList.empty() == true)
	{
		m_propertyEditor->clear();
		m_propertyTable->clear();
		return;
	}

	QList<std::shared_ptr<PropertyObject>> checkedInList;
	QList<std::shared_ptr<PropertyObject>> checkedOutList;

	for (QModelIndex& mi : selectedIndexList)
	{
		std::shared_ptr<Hardware::DeviceObject> device = m_equipmentModel->deviceObject(mi);
		assert(device);

		const DbFileInfo* deviceFileInfo = device->data();
		assert(deviceFileInfo);

		if (deviceFileInfo->state() == E::VcsState::CheckedOut)
		{
			checkedOutList << device;
		}
		else
		{
			checkedInList << device;
		}
	}

	m_propertyEditor->setExpertMode(isPresetMode() || theAppSettings.isExpertMode());
	m_propertyEditor->setReadOnly(checkedOutList.isEmpty() == true);

	m_propertyTable->setExpertMode(isPresetMode() || theAppSettings.isExpertMode());
	m_propertyTable->setReadOnly(checkedOutList.isEmpty() == true);

	// Set objects to the PropertyEditor
	//
	if (checkedOutList.isEmpty() == false)
	{
		m_propertyEditor->setObjects(checkedOutList);
		m_propertyTable->setObjects(checkedOutList);
	}
	else
	{
		m_propertyEditor->setObjects(checkedInList);
		m_propertyTable->setObjects(checkedInList);
	}

	return;
}

void EquipmentTabPage::propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	// Refresh property "EquipmentID", as it depends on "EquipmentIDTemplate"
	//
	if (m_propertyEditor != nullptr)
	{
		m_propertyEditor->updatePropertyValue("EquipmentID");
	}

	// --
	//
	std::vector<std::shared_ptr<DbFile>> files;

	for (auto& o : objects)
	{
		Hardware::DeviceObject* device = dynamic_cast<Hardware::DeviceObject*>(o.get());
		if (device == nullptr)
		{
			assert(device != nullptr);
			return;
		}

		const DbFileInfo* deviceFileInfo = device->data();
		if (deviceFileInfo == nullptr)
		{
			assert(deviceFileInfo != nullptr);
			return;
		}

		QByteArray data;
		bool ok = device->saveToByteArray(&data);

		if (ok == false)
		{
			assert(false);
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		file->operator=(*deviceFileInfo);
		file->setData(std::move(data));
		file->setDetails(device->details());

		files.push_back(file);
	}

	// qDebug() << "Update Properties in the Database";

	bool ok = dbController()->setWorkcopy(files, this);

	if (ok == true)
	{
		// Refresh selected items
		//
		m_equipmentView->updateSelectedDevices();
	}

	return;
}

void EquipmentTabPage::addObjectTriggered()
{
	assert(m_addObjectButton);

	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_addObjectButton);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.ry() += w->height();

	m_addObjectMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EquipmentTabPage::addNewPresetTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_addNewPresetAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.ry() += w->height();

	m_addPresetMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EquipmentTabPage::pendingChanges()
{
	EquipmentVcsDialog d(db(), this);
	d.exec();
}

void EquipmentTabPage::objectVcsStateChanged()
{
	setActionState();
	setProperties();
	return;
}

void EquipmentTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	if (isVisible() == false)
	{
		return;
	}

	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension
	//
	bool extFound = false;
	QString fileName = object.name();

	for (const QString& ext : Hardware::DeviceObjectExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			extFound = true;
			break;
		}
	}

	if (extFound == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<Hardware::DeviceObject> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceChangeset, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceDate, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<Hardware::DeviceObject> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetChangeset, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetDate, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}

void EquipmentTabPage::exportPreset()
{
	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	if (isPresetMode() == false || selectedIndexList.isEmpty() == true)
	{
		assert(isPresetMode());
		assert(selectedIndexList.isEmpty() == false);
		return;
	}

	// --
	//
	bool allObjectsArePresetRoots = true;
	QString firstDeviceName;

	std::vector<const Hardware::DeviceObject*> devices;
	devices.reserve(selectedIndexList.size());

	for (const QModelIndex& mi : selectedIndexList)
	{
		auto device = m_equipmentModel->deviceObject(mi);

		if (device == nullptr || device->isPreset() != true || device->presetRoot() != true)
		{
			assert(device);
			assert(false);
			return;
		}

		if (firstDeviceName.isEmpty() == true)
		{
			int presetVersion = device->propertyValue(Hardware::PropertyNames::presetVersion).toInt();

			firstDeviceName = QString{"%1_v%2.u7devp"}.arg(device->caption()).arg(presetVersion);
		}

		allObjectsArePresetRoots &= device->presetRoot() && device->isPreset();

		devices.push_back(device.get());
	}

	// --
	//
	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													path + QDir::separator() + firstDeviceName,
													tr("Device Presets (*.u7devp);;All Files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	// Read devices from the project database
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> latestDevices;
	latestDevices.reserve(devices.size());

	for (const Hardware::DeviceObject* device : devices)
	{
		const DbFileInfo* deviceFileInfo = device->data();
		if (deviceFileInfo == nullptr)
		{
			Q_ASSERT(deviceFileInfo);
			return;
		}

		std::shared_ptr<Hardware::DeviceObject> out;

		bool result = db()->getDeviceTreeLatestVersion(*deviceFileInfo, &out, this);
		if (result == false)
		{
			return;
		}

		assert(out);
		latestDevices.push_back(out);
	}

	// Save devices to the clipboard
	//
	::Proto::ExportedDevicePreset2 message;

	::Proto::EnvelopeSet* setMessage = message.mutable_items();
	::Proto::EnvelopeSetShortDescription* descriptionMessage = message.mutable_description();

	descriptionMessage->set_projectname(db()->currentProject().projectName().toStdString());
	descriptionMessage->set_username(db()->currentUser().username().toStdString());
	descriptionMessage->set_exporttime(QDateTime::currentDateTime().toSecsSinceEpoch());
	descriptionMessage->set_projectdbversion(DbController::databaseVersion());
	descriptionMessage->set_equipmenteditor(isConfigurationMode());
	descriptionMessage->set_preseteditor(isPresetMode());
	descriptionMessage->set_presetroot(allObjectsArePresetRoots);

	for (std::shared_ptr<Hardware::DeviceObject>& device : latestDevices)
	{
		::Proto::Envelope* protoDevice = setMessage->add_items();
		device->SaveObjectTree(protoDevice);

		descriptionMessage->add_classnamehash(protoDevice->classnamehash());
	}

	// Compress and save to file
	//
	{
		std::string data = message.SerializeAsString();
		QByteArray compressedData = qCompress(QByteArray::fromRawData(data.data(), static_cast<int>(data.size())));

		::Proto::ExportedDevicePreset2 compressedMessage;
		compressedMessage.mutable_description()->operator=(*descriptionMessage);
		compressedMessage.set_compressedthis(compressedData.toStdString());

		std::fstream output(std::filesystem::path(fileName.toStdWString()), std::ios::out | std::ios::binary);

		if (output.is_open() == false || output.bad() == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Write file %1 error.").arg(fileName));
			return;
		}

		bool ok = compressedMessage.SerializeToOstream(&output);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Write file %1 error.").arg(fileName));
			return;
		}
	}

	return;
}

void EquipmentTabPage::importPreset()
{
	static QString path{"."};
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Device Preset"), path, tr("Device Presets (*.u7devp);;All Files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	std::fstream input(std::filesystem::path(fileName.toStdWString()), std::ios::in | std::ios::binary);
	if (input.is_open() == false || input.bad() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Load file %1 error.").arg(fileName));
		return;
	}

	// --
	//
	::Proto::ExportedDevicePreset2 fileMessage;

	bool ok = fileMessage.ParseFromIstream(&input);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Parse file %1 error. File may be corrupted.").arg(fileName));
		return;
	}

	// Check if data was compressed then uncompress it
	//
	::Proto::ExportedDevicePreset2 message;

	if (fileMessage.has_compressedthis() == true)
	{
		QByteArray compressedData =
			QByteArray::fromRawData(fileMessage.compressedthis().data(), static_cast<int>(fileMessage.compressedthis().size()));
		QByteArray uncompressedData = qUncompress(compressedData);
		message.ParseFromArray(uncompressedData.constData(), static_cast<int>(uncompressedData.size()));
	}
	else
	{
		message = fileMessage;
	}

	// Save data to clipboard
	//
	bool canPaste = m_equipmentView->canPaste(message.description());
	if (canPaste == false)
	{
		// Something wrong, cannot paste
		//
		QMessageBox::critical(this, qAppName(), tr("Cannot paste file %1.").arg(fileName));
		return;
	}

	DialogImportPreset d(message, this);
	if (d.exec() == QDialog::Accepted)
	{
		::Proto::EnvelopeSet chosenItems = d.chosenItems();
		if (chosenItems.items_size() != 0)
		{
			m_equipmentView->pasteDevices(chosenItems, message.description(), false);
		}
	}

	return;
}

void EquipmentTabPage::diagSignalTypes()
{
	DialogDiagSignalTypes::showDialog(dbController(), this);
	return;
}

void EquipmentTabPage::onEquipmentViewContextMenuRequested(const QPoint& /*pos*/)
{
	QMenu contextMenu{this};

	if (isConfigurationMode() == true)
	{
		contextMenu.addMenu(m_addObjectMenu);
		contextMenu.addAction(m_addFromPresetAction);
		contextMenu.addAction(m_replaceAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_createInOutsToSignals);
		contextMenu.addAction(m_createInternalAppSignal);
		contextMenu.addAction(m_showAppSignals);

		contextMenu.addSeparator();
		contextMenu.addAction(m_addLogicSchemaToLm);
		contextMenu.addAction(m_showLmsLogicSchemas);

		contextMenu.addSeparator();
		contextMenu.addAction(m_createConnection);
		contextMenu.addAction(m_showObjectConnections);
		contextMenu.addAction(m_showConnections);

		contextMenu.addSeparator();
		contextMenu.addAction(m_copyObjectAction);
		contextMenu.addAction(m_pasteObjectAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_findAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_deleteObjectAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_checkOutAction);
		contextMenu.addAction(m_checkInAction);
		contextMenu.addAction(m_undoChangesRecursivelyAction);
		contextMenu.addAction(m_undoChangesAction);
		contextMenu.addAction(m_historyAction);
		contextMenu.addAction(m_compareAction);
		contextMenu.addAction(m_refreshAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_updateFromPresetAction);
		contextMenu.addAction(m_switchModeAction);
	}
	else
	{
		contextMenu.addMenu(m_addObjectMenu);
		contextMenu.addMenu(m_addPresetMenu);

		contextMenu.addSeparator();
		contextMenu.addAction(m_copyObjectAction);
		contextMenu.addAction(m_pasteObjectAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_findAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_deleteObjectAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_checkOutAction);
		contextMenu.addAction(m_checkInAction);
		contextMenu.addAction(m_undoChangesRecursivelyAction);
		contextMenu.addAction(m_undoChangesAction);
		contextMenu.addAction(m_historyAction);
		contextMenu.addAction(m_compareAction);
		contextMenu.addAction(m_refreshAction);

		contextMenu.addSeparator();
		contextMenu.addAction(m_updateFromPresetAction);
		contextMenu.addAction(m_switchModeAction);
	}

	contextMenu.exec(QCursor::pos());

	return;
}

void EquipmentTabPage::findDeviceObject(QString equipmentId)
{
	GlobalMessanger::instance().changeCurrentTab(this);
	m_equipmentView->findObject(equipmentId);
	return;
}
