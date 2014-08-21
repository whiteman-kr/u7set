#pragma once

#include "MainTabPage.h"
#include "..\include\DeviceObject.h"

class DbController;
/*
class EquipmentModel : public QAbstractItemModel
{
public:
	EquipmentModel(std::shared_ptr<DeviceRoot> root, QObject* parent = 0);
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
	std::shared_ptr<DeviceRoot> m_root;

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

protected slots:
	void addSystem();
	void addCase();
	void addSubblock();
	void addBlock();

	// Data
	//
private:
	QAction* m_addSystemAction;
	QAction* m_addCaseAction;
	QAction* m_addSubblockAction;
	QAction* m_addBlockAction;

	std::shared_ptr<DeviceRoot> m_root;
	EquipmentModel* m_equipmentModel;

	QTreeView* m_equipmentView;
	QTextEdit* m_propertyView;
	QSplitter* m_splitter;
};

*/

