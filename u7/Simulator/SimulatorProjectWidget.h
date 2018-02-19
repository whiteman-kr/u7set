#ifndef SIMULATORPROJECTWIDGET_H
#define SIMULATORPROJECTWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QAction>
#include <QTreeWidget>
#include "../Simulator/Simulator.h"

// Widget for selection build and module
//
class SimulatorProjectWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SimulatorProjectWidget(std::shared_ptr<Sim::Simulator> simulator, QWidget* parent = nullptr);
	virtual ~SimulatorProjectWidget();

protected:
	void createActions();

protected slots:
	void projectUpdated();
	void treeContextMenu(const QPoint& pos);
	void treeDoubleClicked(const QModelIndex &index);
	void openControlTabPage();

protected:
	void fillEquipmentTree();

signals:
	void signal_openControlTabPage(QString equipmentID);

private:
	std::shared_ptr<Sim::Simulator> m_simulator;

	QLabel* m_buildLabel = nullptr;
	QTreeWidget* m_equipmentTree = nullptr;

	enum EquipmentTreeColumns
	{
		EquipmentID,
		Info,
		State,
		Count
	};

	QAction* m_openLmControlPageAction = nullptr;
};


#endif // SIMULATORPROJECTWIDGET_H
