#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../TestSuiteLib/TestSuite.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"
#include "../TestSuiteLib/TestScriptsStorage.h"
#include "../lib/Ui/DialogTcpStatistics.h"
#include "../lib/Ui/DialogAlert.h"
#include "../OnlineLib/TcpClientStatistics.h"

#include "TestSuiteLog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TestSuiteMainWindow; }
QT_END_NAMESPACE

class TestSuiteMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget *parent = nullptr);
	~TestSuiteMainWindow();

private:
	// Initialization
	//
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


	// Processing slots
	//
	void onConfigurationArrived();
	void onTestingFinished(int result);

	// Logging slots
	//
	void onAppLogError(const QString &errMsg);
	void onAppLogWarning(const QString &msg);
	void onAppLogMessage(const QString &msg);
	void onAppLogText(const QString &msg);

	void onTestLogError(const QString &errMsg);
	void onTestLogWarning(const QString &msg);
	void onTestLogMessage(const QString &msg);


private:
	// Ui
	Ui::TestSuiteMainWindow *ui;
	DialogAlert m_dialogAlert;

	QAction* m_pExitAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pStatisticsAction = nullptr;
	QAction* m_pAppLogAction = nullptr;
	QAction* m_aboutQtAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	// Main objects
	TestSuiteLogFile m_appLog;						// Must be initialized first
	TestSuiteTestLog m_testLog;

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
