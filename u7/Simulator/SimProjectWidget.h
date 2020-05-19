#pragma once
#include "SimIdeSimulator.h"

//
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

	void updateModuleStates(Sim::ControlStatus state);

protected:
	void fillEquipmentTree();

signals:
	void signal_openLogicModuleTabPage(QString equipmentId);
	void signal_openCodeTabPage(QString equipmentID);
	void signal_openConnectionTabPage(QString connectionId);
	void signal_openAppSchemasTabPage();

public:
	const SimIdeSimulator* simulator() const;
	SimIdeSimulator* simulator();

private:
	SimIdeSimulator* m_simulator = nullptr;

	QLabel* m_buildLabel = nullptr;
	QTreeWidget* m_treeWidget = nullptr;
};


namespace SimProjectTreeItems
{
	enum EquipmentTreeColumns
	{
		EquipmentID,
		Info,
		State,
		Count
	};


	class BaseTreeItem : public QTreeWidgetItem
	{
	public:
		BaseTreeItem(QTreeWidgetItem* parent, const QStringList& strings);

		virtual void updateState(SimProjectWidget* simProjectWidget, Sim::ControlStatus state);
		virtual void doubleClick(SimProjectWidget* simProjectWidget);
		virtual void contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos);
	};


	class LogicModuleTreeItem : public BaseTreeItem
	{
	public:
		LogicModuleTreeItem(QTreeWidgetItem* parent, std::shared_ptr<Sim::LogicModule> lm);

		virtual void updateState(SimProjectWidget* simProjectWidget, Sim::ControlStatus state) override;
		virtual void doubleClick(SimProjectWidget* simProjectWidget) override;
		virtual void contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos) override;

	public:
		QString m_equipmentId;
	};


	class ConnectionTreeItem : public BaseTreeItem
	{
	public:
		ConnectionTreeItem(QTreeWidgetItem* parent, const Sim::ConnectionPtr& connection);

		virtual void updateState(SimProjectWidget* simProjectWidget, Sim::ControlStatus state) override;
		virtual void doubleClick(SimProjectWidget* simProjectWidget) override;
		virtual void contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos) override;

	public:
		QString m_connectionId;
	};


//	class ConnectionPortTreeItem : public BaseTreeItem
//	{
//	public:
//		ConnectionPortTreeItem(ConnectionTreeItem* parent, const Sim::ConnectionPortPtr& port);

//		virtual void doubleClick(SimProjectWidget* simProjectWidget) override;

//	public:
//		QString m_connectionPortId;
//	};


	class AppLogicSchemasTreeItem : public BaseTreeItem
	{
	public:
		AppLogicSchemasTreeItem(QTreeWidgetItem* parent);

		virtual void updateState(SimProjectWidget* simProjectWidget, Sim::ControlStatus state) override;
		virtual void doubleClick(SimProjectWidget* simProjectWidget) override;
		virtual void contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos) override;

	public:
	};



}

