#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../TestSuiteLib/TestSuite.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"
#include "../TestSuiteLib/TestScriptsStorage.h"
#include "../lib/Ui/DialogTcpStatistics.h"
#include "../lib/Ui/DialogAlert.h"
#include "../OnlineLib/TcpClientStatistics.h"

#include "AppLogOutputWidget.h"
#include "TestListWidget.h"

#include "TestSuiteLog.h"

class TabWidgetEx;
class TestLogTabPage;

class TestSuiteMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget *parent = nullptr);
	~TestSuiteMainWindow();

private:
	// Initialization
	//
	void createDocks();
	void createToolbar();
	void createActions();
	void createMenu();
	void createStatusBar();

	void updateStatusBar();
	void showSoftwareConnection(const QString& caption, const QString& nameFilter,
								const std::vector<TcpClientStatistics::Statistics>& connectionStatistics,
								QLabel* label);

	void loadScriptsFromConfiguration();
	void loadScriptsFromLocalPath();

	void clearTestsTree();
	void fillTestsTree();
	void updateActionsState();
	void updateTimeIndicator(const TestSuite::ControlStatus& state);

	bool eventFilter(QObject *object, QEvent *event) override;
	void timerEvent(QTimerEvent* event) override;

private slots:
	// Interface slots
	//
	void onExit();
	void on_m_run_clicked();
	void on_m_stop_clicked();
	void onSettings();
	void showStatistics();
	void showAppLog();
	void showAboutQt();
	void showAbout();
	void onTestsRefresh();
	void onShowTestContents(const QString& testName);
	void onTabCloseRequested(int index);

	// Processing slots
	//
	void onConfigurationArrived();
	void onTestingFinished(int result);

private:
	// Ui
	//Ui::TestSuiteMainWindow *ui;
	DialogAlert m_dialogAlert;

	QAction* m_pExitAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pStatisticsAction = nullptr;
	QAction* m_pAppLogAction = nullptr;
	QAction* m_aboutQtAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QAction* m_refreshTestsAction = nullptr;
	QAction* m_runAction = nullptr;
	//QAction* m_pauseAction = nullptr;
	QAction* m_stopAction = nullptr;

	TabWidgetEx* m_tabWidget = nullptr;

	QToolBar* m_toolBar = nullptr;
	QLabel* m_timeIndicator = nullptr;	// Widget on toolbar to show current simulation time


	TestListWidget* m_testListWidget = nullptr;
	TestLogTabPage* m_testLogTabPage = nullptr;

	AppLogOutputWidget* m_appLogoutputWidget = nullptr;
	QDockWidget* m_appLogPaneDock = nullptr;

	// Main objects
	TestSuiteLogFile m_appLog;						// Must be initialized first
	TestSuiteTestLogOutput m_testLogOutput;

	TestSuite::TestSuiteConfigController m_configController;
	TestSuite::TestSuite m_testSuite;
	TestSuite::TestScriptsStorage m_testScriptsStorage;

	// Status bar and statistics
	//
	DialogTcpStatistics* m_dialogStatistics = nullptr;

	QLabel* m_statusBarProjectInfo = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarAppDataConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;

	int m_mainWindowTimerId_250ms = -1;

};

extern TestSuiteMainWindow* theMainWindow;

#endif // TESTSUITEMAINWINDOW_H
