#pragma once
#include "SimIdeSimulator.h"

// Widget for selection build and module
//
class SimProjectWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SimProjectWidget(SimIdeSimulator* simulator, QWidget* parent = nullptr);
	virtual ~SimProjectWidget();

protected:
	void createActions();

protected slots:
	void projectUpdated();
	void treeContextMenu(const QPoint& pos);
	void treeDoubleClicked(const QModelIndex &index);

	void openModuleTabPage();
	void openCodeTabPage();

	void updateModuleStates(Sim::ControlStatus state);

protected:
	void fillEquipmentTree();

signals:
	void signal_openControlTabPage(QString equipmentID);
	void signal_openCodeTabPage(QString equipmentID);

private:
	SimIdeSimulator* m_simulator = nullptr;

	QLabel* m_buildLabel = nullptr;
	QTreeWidget* m_treeWidget = nullptr;

	enum EquipmentTreeColumns
	{
		EquipmentID,
		Info,
		State,
		Count
	};

	QAction* m_openLmControlPageAction = nullptr;
	QAction* m_openLmCodePageAction = nullptr;
};

namespace SimProjectTreeItems
{
	enum SimProjectTreeTypes
	{
		LogicModule = 1010,
		Connection = 1020,
		ConnectionPort = 1021,
	};


	class BaseTreeItem : public QTreeWidgetItem
	{
	public:
		BaseTreeItem(QTreeWidgetItem* parent, const QStringList& strings, int type = Type);
	};


	class LogicModuleTreeItem : public BaseTreeItem
	{
	public:
		LogicModuleTreeItem(QTreeWidgetItem* parent, std::shared_ptr<Sim::LogicModule> lm);

	public:
		QString m_equipmentId;
	};


	class ConnectionTreeItem : public BaseTreeItem
	{
	public:
		ConnectionTreeItem(QTreeWidgetItem* parent, const Sim::ConnectionPtr& connection);

	public:
		QString m_connectionId;
	};


	class ConnectionPortTreeItem : public BaseTreeItem
	{
	public:
		ConnectionPortTreeItem(ConnectionTreeItem* parent, const Sim::ConnectionPortPtr& port);

	public:
		QString m_connectionPortId;
	};



}

