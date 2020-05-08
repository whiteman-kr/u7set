#ifndef SIMULATORPROJECTWIDGET_H
#define SIMULATORPROJECTWIDGET_H
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

	void openControlTabPage();
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
	QTreeWidget* m_equipmentTree = nullptr;

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


#endif // SIMULATORPROJECTWIDGET_H
