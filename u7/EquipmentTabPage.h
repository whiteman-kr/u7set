#pragma once

#include "MainTabPage.h"
#include "../include/DeviceObject.h"

class DbController;

class EquipmentModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	EquipmentModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent);
	virtual ~EquipmentModel();

	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual bool canFetchMore(const QModelIndex& parent) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

	// Get data
public:
	bool insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex);

protected:
	Hardware::DeviceObject* deviceObject(QModelIndex& index);
	const Hardware::DeviceObject* deviceObject(const QModelIndex& index) const;

public slots:
	void projectOpened();
	void projectClosed();

	// Properties
public:
	DbController* dbController();
	DbController* dbController() const;

	// Data
	//
private:
	DbController* m_dbController;
	QWidget* m_parentWidget;

	std::shared_ptr<Hardware::DeviceObject> m_root;

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

public slots:
	void addSystem();
	void addCase();
	void addSubblock();
	void addBlock();

	// Properties
	//
protected:
	EquipmentModel* equipmentModel();
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

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();

	// Data
	//
private:
	QAction* m_addSystemAction = nullptr;
	QAction* m_addCaseAction = nullptr;
	QAction* m_addSubblockAction = nullptr;
	QAction* m_addBlockAction = nullptr;

	EquipmentModel* m_equipmentModel = nullptr;
	EquipmentView* m_equipmentView = nullptr;

	QTextEdit* m_propertyView = nullptr;
	QSplitter* m_splitter = nullptr;
};


