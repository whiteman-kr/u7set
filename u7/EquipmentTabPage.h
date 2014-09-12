#pragma once

#include "MainTabPage.h"
#include "../include/DeviceObject.h"

class DbController;

class EquipmentModel : public QAbstractItemModel
{
public:
	EquipmentModel(std::shared_ptr<Hardware::DeviceRoot> root, QObject* parent = 0);
	virtual ~EquipmentModel();

	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	// Data
	//
private:
	std::shared_ptr<Hardware::DeviceRoot> m_root;

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

	std::shared_ptr<Hardware::DeviceRoot> m_root;
	EquipmentModel* m_equipmentModel = nullptr;
	EquipmentView* m_equipmentView = nullptr;

	QTextEdit* m_propertyView = nullptr;
	QSplitter* m_splitter = nullptr;
};


