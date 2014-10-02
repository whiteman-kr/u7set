#pragma once

#include "MainTabPage.h"
#include "../include/DeviceObject.h"


class DbController;

class QtTreePropertyBrowser;
//class QtProperty;
//class QtStringPropertyManager;
//class QtEnumPropertyManager;
//class QtIntPropertyManager;
//class QtDoublePropertyManager;


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

	// --
	//
public:
	bool insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex);
	void deleteDeviceObject(QModelIndexList& rowList);

	void checkInDeviceObject(QModelIndexList& rowList);
	void checkOutDeviceObject(QModelIndexList& rowList);
	void undoChangesDeviceObject(QModelIndexList& rowList);
	void refreshDeviceObject(QModelIndexList& rowList);

	Hardware::DeviceObject* deviceObject(QModelIndex& index);
	const Hardware::DeviceObject* deviceObject(const QModelIndex& index) const;

public slots:
	void projectOpened();
	void projectClosed();

	void switchMode();

	// Properties
public:
	DbController* dbController();
	DbController* dbController() const;

	bool isPresetMode() const;
	bool isConfigurationMode() const;

	// Data
	//
private:
	DbController* m_dbController;
	QWidget* m_parentWidget;

	std::shared_ptr<Hardware::DeviceObject> m_root;
	std::shared_ptr<Hardware::DeviceObject> m_configuration;
	std::shared_ptr<Hardware::DeviceObject> m_preset;

	enum Columns
	{
		ObjectNameColumn,
		ObjectStrIdColumn,
		ObjectStateColumn,
		ObjectUserColumn,

		// Add other column befor this line
		//
		ColumnCount
	};
};


class EquipmentView : public QTreeView
{
	Q_OBJECT

private:
	EquipmentView();

public:
	EquipmentView(DbController* dbcontroller);
	virtual ~EquipmentView();

	bool isPresetMode() const;
	bool isConfigurationMode() const;

public slots:
	void addSystem();
	void addRack();
	void addChassis();
	void addModule();

	void addPresetRack();
	void addPresetChassis();
	void addPresetModule();

	void choosePreset(Hardware::DeviceType type);

	void addPresetToConfiguration(const DbFileInfo& fileInfo);
	void addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object);

	void deleteSelectedDevices();
	void checkInSelectedDevices();
	void checkOutSelectedDevices();
	void undoChangesSelectedDevices();
	void refreshSelectedDevices();

	// Properties
	//
protected:
	EquipmentModel* equipmentModel();
	EquipmentModel* equipmentModel() const;
	DbController* dbController();

	// Data
	//
private:
	DbController* m_dbController;
};


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
	void modeSwitched();

	// Data
	//
private:
	QMenu* m_addObjectMenu = nullptr;
	QAction* m_addObjectAction = nullptr;
		QAction* m_addSystemAction = nullptr;
		QAction* m_addRackAction = nullptr;
		QAction* m_addChassisAction = nullptr;
		QAction* m_addModuleAction = nullptr;
	//----------------------------------
	QMenu* m_addPresetMenu = nullptr;
	QAction* m_addPresetAction = nullptr;
		QAction* m_addPresetRackAction = nullptr;
		QAction* m_addPresetChassisAction = nullptr;
		QAction* m_addPresetModuleAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction1 = nullptr;
	QAction* m_deleteObjectAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction2 = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoChangesAction = nullptr;
	QAction* m_refreshAction = nullptr;
	//----------------------------------
	QAction* m_SeparatorAction3 = nullptr;
	QAction* m_switchMode = nullptr;

	//--
	//
	EquipmentModel* m_equipmentModel = nullptr;
	EquipmentView* m_equipmentView = nullptr;

	QtTreePropertyBrowser* m_propertyBrowser = nullptr;
	QSplitter* m_splitter = nullptr;
};


