#pragma once

#include "SimSchemaManager.h"
#include <ILogFile.h>

#include <SimulatorLib/SimControlStatus.h>
#include <SimulatorUi/ISimPropertyStorage.h>
#include <SimulatorUi/SimIdeSimulator.h>
#include <VFrame30/AppSignalController.h>

#include <QDoubleValidator>
#include <QMainWindow>
#include <QToolBar>

class QDockWidget;
class QLineEdit;
class QComboBox;
class QLabel;

namespace UiLib
{
	class TabWidgetEx;
}

namespace SimUi
{
	class SimToolBar;
	class SimProjectWidget;
	class SimOutputWidget;
	class DbProjectStateNotifier;
	class NotificationPanel;

	class SimWidgetPrivate : public QMainWindow
	{
		Q_OBJECT

	public:
		SimWidgetPrivate(std::shared_ptr<ILogFile> ideLogFile,
						 std::shared_ptr<SimIdeSimulator> simulator,
						 std::function<QString(QWidget*)> getProjectPathFunc,
						 ISimPropertyStorage& propertyStorage,
						 DbProjectStateNotifier* dbProjectStateNotifier,
						 QWidget* parent = nullptr,
						 Qt::WindowType windowType = Qt::Window,
						 bool slaveWindow = false,                  // Cannot have output pane, do not stores its state
						 SimWidgetPrivate* masterWindow = nullptr); // If slaveWindow is true, then masterWindow must be not null
		virtual ~SimWidgetPrivate();

	public:
		void startTrends(const std::vector<AppSignalParam>& appSignals);

	public slots:
		void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
		void signalInfo(QString appSignalId);
		void openSchemaTabPage(QString schemaId, QStringList highlightIds);

	protected:
		void createToolBar();
		void createDocks();

		void showEvent(QShowEvent* e) override;
		void timerEvent(QTimerEvent* e) override;

	signals:
		void needUpdateActions();
		void needCloseChildWindows();

	protected slots:
		void projectOpened();
		void projectClosed();
		void openBuild();
		void closeBuild();
		void refreshBuild();

		void aboutToQuit();

		void controlStateChanged(Sim::SimControlState state);
		void updateTimeIndicator(Sim::ControlStatus state);
		void updateActions();


		void runSimulation();
		void pauseSimulation();
		void stopSimulation(bool stopSimulationThread = false);

		void allowLanCommToggled(bool state);
		void profileComboTextChanged(QString text);

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

	public:
		SimSchemaManager& schemaManager();
		const SimSchemaManager& schemaManager() const;

	private:
		bool m_slaveWindow = false;                            // Cannot have output pane, do not stores its state
		SimWidgetPrivate* m_masterWindow = nullptr;            // If slaveWindow is true, then masterWindow must be not null

		std::function<QString(QWidget*)> m_getProjectPathFunc; // This function is called to get project path (action openProject)
		ISimPropertyStorage& m_propertyStorage;
		DbProjectStateNotifier* m_dbProjectStateNotifier = nullptr;

		bool m_showEventFired =
			false; // Save of widget state possible only after showEvent, otherwise state will be starge, even can hide all child widgets.
		UiLib::TabWidgetEx* m_tabWidget = nullptr;

		SimToolBar* m_toolBar = nullptr;
		QLabel* m_timeIndicator = nullptr; // Widget on toolbar to show current simulation time

		SimOutputWidget* m_outputWidget = nullptr;
		SimProjectWidget* m_projectWidget = nullptr;

		std::shared_ptr<ILogFile> m_ideLogFile;
		std::shared_ptr<SimIdeSimulator> m_simulator;

		QDockWidget* m_overridePaneDock = nullptr;
		QDockWidget* m_outputPaneDock = nullptr;

		NotificationPanel* m_notificationPanel = nullptr;

		// --
		//
		SimSchemaManager m_schemaManager;

		VFrame30::AppSignalController* m_appSignalController = nullptr;

		// Actions
		//
		QAction* m_openProjectAction = nullptr;
		QAction* m_closeProjectAction = nullptr;
		QAction* m_refreshProjectAction = nullptr;

		QAction* m_addWindowAction = nullptr;

		QLocale m_simulationTimeLocale{QLocale::C};
		QDoubleValidator m_simulationTimeEditValidator;
		QLineEdit* m_simulationTimeEdit = nullptr;

		QComboBox* m_speedComboBox = nullptr;
		static std::vector<QComboBox*> s_speedComboBoxes; // registered m_speedComboBox, for state synchronization

		QAction* m_runAction = nullptr;
		QAction* m_pauseAction = nullptr;
		QAction* m_stopAction = nullptr;

		QAction* m_allowLanComm = nullptr;
		QComboBox* m_profilesComboBox = nullptr;
		static std::vector<QComboBox*> s_profilesComboBoxes; // registered m_profilesComboBox, for state synchronization

		QAction* m_schemaListAction = nullptr;
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

	protected:
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;
	};
} // namespace SimUi