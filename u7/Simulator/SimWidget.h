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
class TabWidgetEx;


class SimWidget : public QMainWindow, HasDbController, protected Sim::Output
{
	Q_OBJECT

public:
	SimWidget(std::shared_ptr<SimIdeSimulator> simulator,
			  DbController* db,
			  QWidget* parent = nullptr,
			  Qt::WindowType windowType = Qt::Window,
			  bool slaveWindow = false);		// Cannot have output pane, do not stores its state
	virtual ~SimWidget();

public:
	void startTrends(const std::vector<AppSignalParam>& appSignals);

public slots:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);
	void openSchemaTabPage(QString schemaId, QStringList highlightIds);

protected:
	void createToolBar();
	void createDocks();
	QDockWidget* createMemoryDock(QString caption);

	virtual void showEvent(QShowEvent* e) override;

signals:
	void needUpdateActions();
	void needCloseChildWindows();

protected slots:
	void aboutToQuit();

	void controlStateChanged(Sim::SimControlState state);
	void updateTimeIndicator(Sim::ControlStatus state);
	void updateActions();

	void projectOpened(DbProject project);
	void projectClosed();
	void openBuild();
	void closeBuild();
	void refreshBuild();

	void runSimulation();
	void pauseSimulation();
	void stopSimulation(bool stopSimulationThread = false);

	void allowRegDataToggled(bool state);

	void showSnapshot();
	void showFindSignal();
	void showTrends();

	bool loadBuild(QString buildPath);

	void addNewWindow();

	void openLogicModuleTabPage(QString lmEquipmentId);
	void openCodeTabPage(QString lmEquipmentId);
	void openConnectionTabPage(QString connectionId);
	void openAppSchemasTabPage();

	void tabCloseRequest(int index);
	void tabCurrentChanged(int index);
	void tabBarContextMenuRequest(const QPoint& pos);

private:
	bool m_slaveWindow = false;				// Cannot have output pane, do not stores its state
	bool m_showEventFired = false;			// Save of widget state possible only after showEvent, otherwise stae will be starge, even can hide all child widgets.
	TabWidgetEx* m_tabWidget = nullptr;

	SimToolBar* m_toolBar = nullptr;
	QLabel* m_timeIndicator = nullptr;	// Widget on toolbar to show current simulation time

	SimOutputWidget* m_outputWidget = nullptr;
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

	QAction* m_allowRegData = nullptr;

	QAction* m_snapshotAction = nullptr;
	QAction* m_findSignalAction = nullptr;
	QAction* m_trendsAction = nullptr;

	QAction* m_showControlTabAccelerator = nullptr;
};


class SimToolBar : public QToolBar
{
	Q_OBJECT
public:
	explicit SimToolBar(const QString& title, QWidget* parent = nullptr);
	virtual ~SimToolBar();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;
};

#endif // SIMULATORWIDGET_H
