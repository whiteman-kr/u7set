#ifndef SIMULATORWIDGET_H
#define SIMULATORWIDGET_H

#include <QMainWindow>
#include <QtWidgets>
#include "../lib/DbController.h"
#include "Simulator.h"

class SimulatorProjectWidget;
class SimulatorOutputWidget;
class SimulatorMemoryWidget;
class SimulatorToolBar;

class SimulatorWidget : public QMainWindow, HasDbController
{
	Q_OBJECT
public:
	SimulatorWidget(std::shared_ptr<Sim::Simulator> simulator,
					DbController* db,
					QWidget* parent = nullptr,
					Qt::WindowType windowType = Qt::Window,
					bool slaveWindow = false);		// Cannot have output pane, do not stores its state
	virtual ~SimulatorWidget();

protected:
	void createToolBar();
	void createDocks();
	QDockWidget* createMemoryDock(QString caption);

	virtual void showEvent(QShowEvent*) override;

signals:
	void needUpdateActions();

protected slots:
	void updateActions();

	void projectOpened(DbProject project);
	void openBuild();
	void closeBuild();
	void refreshBuild();

	bool loadBuild(QString buildPath);

	void addNewWindow();

	void openControlTabPage(QString lmEquipmentId);
	void tabCloseRequest(int index);
	void tabBarContextMenuRequest(const QPoint& pos);

private:
	bool m_slaveWindow = false;				// Cannot have output pane, do not stores its state
	QTabWidget* m_tabWidget = nullptr;
	SimulatorToolBar* m_toolBar = nullptr;
	SimulatorProjectWidget* m_projectWidget = nullptr;
	std::vector<SimulatorMemoryWidget*> m_memoryWidgets;

	std::shared_ptr<Sim::Simulator> m_simulator;

	// Actions
	//
	QAction* m_openProjectAction = nullptr;
	QAction* m_closeProjectAction = nullptr;
	QAction* m_refreshProjectAction = nullptr;

	QAction* m_addWindowAction = nullptr;

//	QAction* m_runAction = nullptr;
//	QAction* m_stopAction = nullptr;
//	QAction* m_pauseAction = nullptr;
};


class SimulatorToolBar : public QToolBar
{
	Q_OBJECT
public:
	explicit SimulatorToolBar(const QString& title, QWidget* parent = nullptr);
	virtual ~SimulatorToolBar();

private:
};

#endif // SIMULATORWIDGET_H
