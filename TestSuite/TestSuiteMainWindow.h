#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../TestSuiteLib/TestSuite.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"

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
	void createActions();
	void createMenu();
	void createStatusBar();

	void fillTestsTree();

private slots:
	void exit();
	void on_m_run_clicked();
	void newLogItem(const TestSuite::TestLogItem& item);
	void onTestingFinished(int result);

	void showSettings();

	void onConfigurationArrived();

	void onLogError(const QString &errMsg);
	void onLogWarning(const QString &msg);
	void onLogMessage(const QString &msg);
	void onLogText(const QString &msg);

	void onOutputLogError(const QString &errMsg);
	void onOutputLogWarning(const QString &msg);
	void onOutputLogMessage(const QString &msg);

	void on_m_stop_clicked();

private:
	// Ui
	Ui::TestSuiteMainWindow *ui;

	QAction* m_pExitAction = nullptr;
	QAction* m_pSettingsAction = nullptr;

	// Main objects
	TestSuiteLogFile m_logFile;						// Must be initialized first

	TestSuiteOutputLog m_outputLog;

	TestSuite::TestSuite m_testSuite;
};

extern TestSuiteMainWindow* theMainWindow;

#endif // TESTSUITEMAINWINDOW_H
