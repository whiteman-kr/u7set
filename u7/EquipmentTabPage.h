#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/DeviceObject.h"
#include "IdePropertyEditor.h"

class DbController;

//
//
// EquipmentModel
//
//
class EquipmentModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	EquipmentModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent);
	virtual ~EquipmentModel();

	QModelIndex index(int row, const QModelIndex& parentIndex) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual bool canFetchMore(const QModelIndex& parent) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

	void sortDeviceObject(Hardware::DeviceObject* object, int column, Qt::SortOrder order);
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	// --
	//
public:
	bool insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex);
	void deleteDeviceObject(const QModelIndexList& rowList);

private:
	void updateRowFuncOnCheckIn(QModelIndex modelIndex, const std::map<int, DbFileInfo>& updateFiles, std::set<void*>& updatedModelIndexes);
public:
	void checkInDeviceObject(QModelIndexList& rowList);

	void checkOutDeviceObject(QModelIndexList& rowList);
	void undoChangesDeviceObject(QModelIndexList& undowRowList);

	void refreshDeviceObject(QModelIndexList& rowList);
	void updateDeviceObject(QModelIndexList& rowList);

	Hardware::DeviceObject* deviceObject(QModelIndex& index);
	const Hardware::DeviceObject* deviceObject(const QModelIndex& index) const;

	std::shared_ptr<Hardware::DeviceObject> deviceObjectSharedPtr(QModelIndex& index);

	QString usernameById(int userId) const;

	void reset();

public slots:
	void projectOpened();
	void projectClosed();

	void switchMode();

	void updateUserList();

signals:
	void objectVcsStateChanged();

	// Properties
	//
public:
	DbController* dbController();
	DbController* dbController() const;

	bool isPresetMode() const;
	bool isConfigurationMode() const;

public:
	enum Columns
	{
		ObjectNameColumn,
		ObjectEquipmentIdColumn,
		ObjectPlaceColumn,
		ObjectStateColumn,
		ObjectUserColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

	// Data
	//
private:
	DbController* m_dbController;
	QWidget* m_parentWidget;

	std::shared_ptr<Hardware::DeviceObject> m_root;
	std::shared_ptr<Hardware::DeviceObject> m_configuration;
	std::shared_ptr<Hardware::DeviceObject> m_preset;

	int m_sortColumn = ObjectPlaceColumn ;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	std::map<int, QString> m_users;
};


//
//
// EquipmentView
//
//
class EquipmentView : public QTreeView
{
	Q_OBJECT

public:
	EquipmentView() = delete;
	explicit EquipmentView(DbController* dbcontroller);
	virtual ~EquipmentView();

	bool isPresetMode() const;
	bool isConfigurationMode() const;

signals:
	void updateState();

public slots:
	void addSystem();
	void addRack();
	void addChassis();
	void addModule();
	void addController();
	void addSignal();
	void addWorkstation();
	void addSoftware();

	void addPreset();
	void replaceObject();

	void addPresetRack();
	void addPresetChassis();
	void addPresetModule();
	void addPresetController();
	void addPresetWorkstation();
	void addPresetSoftware();

	void choosePreset(Hardware::DeviceType type);

	std::shared_ptr<Hardware::DeviceObject> addPresetToConfiguration(const DbFileInfo& fileInfo, bool addToEquipment);
	QModelIndex addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentModelIndex, bool clearPrevSelection);

	void addInOutsToSignals();
	void showAppSignals(bool refreshSignalList = false);			// Show application signals for this object
	void addAppSignal();

	void addLogicSchemaToLm();
	void showLogicSchemaForLm();

	void addOptoConnection();
	void showModuleOptoConnections();

	void copySelectedDevices();
	void pasteDevices();
	bool canPaste() const;

	void deleteSelectedDevices();
	void checkInSelectedDevices();
	void checkOutSelectedDevices();
	void undoChangesSelectedDevices();
	void showHistory();
	void compare();
	void refreshSelectedDevices();

	void updateSelectedDevices();

	void updateFromPreset();
	bool updateDeviceFromPreset(std::shared_ptr<Hardware::DeviceObject> device,
								std::shared_ptr<Hardware::DeviceObject> preset,
								const QStringList& forceUpdateProperties,
								const QStringList& presetsToUpdate,
								std::vector<std::shared_ptr<Hardware::DeviceObject>>* updateDeviceList,
								std::vector<Hardware::DeviceObject*>* deleteDeviceList,
								std::vector<std::pair<int, int>>* addDeviceList,
								QVector<Hardware::DeviceSignal*>* deviceSignalsToUpdateAppSignals);

	// Events
	//
protected:
	virtual void focusInEvent(QFocusEvent* event) override;
	virtual void focusOutEvent(QFocusEvent* event) override;

	// Properties
	//
protected:
	EquipmentModel* equipmentModel();
	EquipmentModel* equipmentModel() const;
	DbController* db();

	// Data
	//
private:
	DbController* m_dbController;

public:
	static const char* mimeType;					// = "application/x-deviceobjecs";
	static const char* mimeTypeShortDescription;	// = "application/x-deviceobjecs-sd";
};

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

protected:
	void CreateActions();

	bool isPresetMode() const;
	bool isConfigurationMode() const;

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

	void propertiesModeTabChanged(int index);

	//void moduleConfiguration();

	void setProperties();

	void propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects);

protected slots:
	void addObjectTriggered();
	void addNewPresetTriggered();

	void pendingChanges();

	void objectVcsStateChanged();

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
private:
	QMenu* m_addObjectMenu = nullptr;
	QAction* m_addObjectAction = nullptr;
		QAction* m_addSystemAction = nullptr;
		QAction* m_addRackAction = nullptr;
		QAction* m_addChassisAction = nullptr;
		QAction* m_addModuleAction = nullptr;
		QAction* m_addControllerAction = nullptr;
		QAction* m_addSignalAction = nullptr;
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
	QAction* m_inOutsToSignals = nullptr;
	QAction* m_showAppSignals = nullptr;
	QAction* m_addAppSignal = nullptr;
	//----------------------------------
	QAction* m_separatorSchemaLogic = nullptr;
	QAction* m_addLogicSchemaToLm = nullptr;
	QAction* m_showLmsLogicSchemas = nullptr;
	//----------------------------------
	QAction* m_separatorOptoConnection = nullptr;
	QAction* m_addOptoConnection = nullptr;
	QAction* m_showModuleOptoConnections = nullptr;
	//----------------------------------
	QAction* m_separatorAction01 = nullptr;
	QAction* m_copyObjectAction = nullptr;
	QAction* m_pasteObjectAction = nullptr;
	//----------------------------------
	QAction* m_separatorAction1 = nullptr;
	QAction* m_deleteObjectAction = nullptr;
	//----------------------------------
	QAction* m_separatorAction2 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_historyAction = nullptr;
	QAction* m_compareAction = nullptr;
	QAction* m_refreshAction = nullptr;
	//----------------------------------
	QAction* m_separatorAction3 = nullptr;
	QAction* m_updateFromPresetAction = nullptr;
	QAction* m_switchModeAction = nullptr;
    QAction* m_connectionsAction = nullptr;
	QAction* m_pendingChangesAction = nullptr;
	QAction* m_SeparatorAction4 = nullptr;

	//--
	//
	EquipmentModel* m_equipmentModel = nullptr;
	EquipmentView* m_equipmentView = nullptr;

	QSplitter* m_splitter = nullptr;
	QToolBar* m_toolBar = nullptr;

	IdePropertyEditor* m_propertyEditor = nullptr;
	IdePropertyTable* m_propertyTable = nullptr;
};


