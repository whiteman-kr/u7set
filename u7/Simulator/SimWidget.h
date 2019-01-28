#ifndef SIMULATORWIDGET_H
#define SIMULATORWIDGET_H

#include "../lib/DbController.h"
#include "SimIdeSimulator.h"
#include "SimSchemaManager.h"
#include "../Simulator/SimAppSignalManager.h"
#include "SimTuningTcpClient.h"
#include "../../VFrame30/AppSignalController.h"
#include "../../VFrame30/TuningController.h"


class SimProjectWidget;
class SimOutputWidget;
class SimMemoryWidget;
class SimToolBar;


class SimWidget : public QMainWindow, HasDbController
{
	Q_OBJECT
public:
	SimWidget(std::shared_ptr<SimIdeSimulator> simulator,
			  DbController* db,
			  QWidget* parent = nullptr,
			  Qt::WindowType windowType = Qt::Window,
			  bool slaveWindow = false);		// Cannot have output pane, do not stores its state
	virtual ~SimWidget();

protected:
	void createToolBar();
	void createDocks();
	QDockWidget* createMemoryDock(QString caption);

	virtual void showEvent(QShowEvent*) override;

signals:
	void needUpdateActions();

protected slots:
	void controlStateChanged(Sim::SimControlState state);
	void updateActions();

	void projectOpened(DbProject project);
	void openBuild();
	void closeBuild();
	void refreshBuild();

	void runSimulation();
	void pauseSimulation();
	void stopSimulation();

	bool loadBuild(QString buildPath);

	void addNewWindow();

	void openControlTabPage(QString lmEquipmentId);
	void openLogicSchemaTabPage(QString schemaId);

	void openSchemaTabPage(QString fileName);
	void openCodeTabPage(QString lmEquipmentId);

	void tabCloseRequest(int index);
	void tabBarContextMenuRequest(const QPoint& pos);

private:
	bool m_slaveWindow = false;				// Cannot have output pane, do not stores its state
	QTabWidget* m_tabWidget = nullptr;
	SimToolBar* m_toolBar = nullptr;
	SimProjectWidget* m_projectWidget = nullptr;
	std::vector<SimMemoryWidget*> m_memoryWidgets;

	std::shared_ptr<SimIdeSimulator> m_simulator;

	// --
	//
	SimSchemaManager m_schemaManager;

	VFrame30::AppSignalController* m_appSignalController = nullptr;

	SimTuningTcpClient m_tuningTcpClient;
	VFrame30::TuningController* m_tuningController = nullptr;

	// Actions
	//
	QAction* m_openProjectAction = nullptr;
	QAction* m_closeProjectAction = nullptr;
	QAction* m_refreshProjectAction = nullptr;

	QAction* m_addWindowAction = nullptr;

	QAction* m_runAction = nullptr;
	QAction* m_pauseAction = nullptr;
	QAction* m_stopAction = nullptr;
};


class SimToolBar : public QToolBar
{
	Q_OBJECT
public:
	explicit SimToolBar(const QString& title, QWidget* parent = nullptr);
	virtual ~SimToolBar();

private:
};

#endif // SIMULATORWIDGET_H
