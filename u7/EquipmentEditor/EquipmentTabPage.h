#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"


class IdePropertyEditor;
class IdePropertyTable;
class DbController;
class EquipmentModel;
class EquipmentView;

namespace Hardware
{
	class DeviceObject;
}

//
//
// EquipmentTabPage
//
//
class EquipmentTabPage : public MainTabPage
{
	Q_OBJECT

public:
	EquipmentTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~EquipmentTabPage();

	void saveSession() const;

	std::shared_ptr<Hardware::DeviceObject> deviceObject(QString equipmentId) const;
	std::vector<std::shared_ptr<Hardware::DeviceObject>> deviceObjects(QString equipmentId) const;

protected:
	void CreateActions();

	[[nodiscard]] bool isPresetMode() const;
	[[nodiscard]] bool isConfigurationMode() const;

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());

	void setActionState();
	void clipboardChanged();

	void modeSwitched();
	void showConnections();

	void setProperties();

	void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

protected slots:
	void addObjectTriggered();
	void addNewPresetTriggered();

	void pendingChanges();
	void objectVcsStateChanged();

	void compareObject(DbChangesetObject object, CompareData compareData);

	void exportPreset();
	void importPreset();

	void diagSignalTypes();

	void onEquipmentViewContextMenuRequested(const QPoint& pos);

	void findDeviceObject(QString equipmentId);

	// Data
	//
private:
	QMenu* m_addObjectMenu = new QMenu(tr("Add Object"), this);
	QAction* m_addObjectButton = nullptr;
	QAction* m_addSystemAction = nullptr;
	QAction* m_addRackAction = nullptr;
	QAction* m_addChassisAction = nullptr;
	QAction* m_addModuleAction = nullptr;
	QAction* m_addControllerAction = nullptr;
	QAction* m_addAppSignalAction = nullptr;
	QAction* m_addDiagSignalAction = nullptr;
	QAction* m_addDiagSignalReflectionAction = nullptr;
	QAction* m_addWorkstationAction = nullptr;
	QAction* m_addSoftwareAction = nullptr;

	QAction* m_addFromPresetAction = nullptr;
	QAction* m_replaceAction = nullptr;

	//----------------------------------
	QMenu* m_addPresetMenu = nullptr;
	QAction* m_addNewPresetAction = nullptr;
	QAction* m_addPresetRackAction = nullptr;
	QAction* m_addPresetChassisAction = nullptr;
	QAction* m_addPresetModuleAction = nullptr;
	QAction* m_addPresetControllerAction = nullptr;
	QAction* m_addPresetWorkstationAction = nullptr;
	QAction* m_addPresetSoftwareAction = nullptr;

	QAction* m_separatorActionA = nullptr;

	//----------------------------------
	QAction* m_separatorAction0 = nullptr;
	QAction* m_createInOutsToSignals = nullptr;
	QAction* m_createInternalAppSignal = nullptr;
	QAction* m_showAppSignals = nullptr;
	//----------------------------------
	QAction* m_separatorSchemaLogic = nullptr;
	QAction* m_addLogicSchemaToLm = nullptr;
	QAction* m_showLmsLogicSchemas = nullptr;
	//----------------------------------
	QAction* m_separatorOptoConnection = nullptr;
	QAction* m_createConnection = nullptr;
	QAction* m_showObjectConnections = nullptr;
	QAction* m_showConnections = nullptr;

	//----------------------------------
	QAction* m_copyObjectAction = nullptr;
	QAction* m_pasteObjectAction = nullptr;
	//----------------------------------
	QAction* m_findAction = nullptr;
	//----------------------------------
	QAction* m_deleteObjectAction = nullptr;
	//----------------------------------
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesRecursivelyAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_refreshAction = nullptr;
	//----------------------------------
	QAction* m_updateFromPresetAction = nullptr;
	QAction* m_switchModeAction = nullptr;
	QAction* m_pendingChangesAction = nullptr;
	QAction* m_SeparatorAction4 = nullptr;

	// --
	//
	QAction* m_separatorPresetExportImport = nullptr;
	QAction* m_exportPresetAction = nullptr;
	QAction* m_importPresetAction = nullptr;

	// --
	//
	QAction* m_separatorAction4 = nullptr;
	QAction* m_diagSignalTypesAction = nullptr;

	//--
	//
	EquipmentModel* m_equipmentModel = nullptr;
	EquipmentView* m_equipmentView = nullptr;

	QSplitter* m_splitter = nullptr;
	QToolBar* m_toolBar = nullptr;

	IdePropertyEditor* m_propertyEditor = nullptr;
	IdePropertyTable* m_propertyTable = nullptr;
};

