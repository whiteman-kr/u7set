#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../UtilsLib/LogFile.h"
#include "../TestSuiteLib/TestLibrary.h"
#include "../TestSuiteLib/TestSuiteConfigController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TestSuiteMainWindow; }
QT_END_NAMESPACE

class TestSuiteMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	TestSuiteMainWindow(const SoftwareInfo &softwareInfo, QWidget *parent = nullptr);
	~TestSuiteMainWindow();

private:
	void createActions();
	void createMenu();
	void createStatusBar();

	void fillTestsTree();

private slots:
	void exit();
	void on_m_run_clicked();
	void newLogItem(const TestLogItem& item);
	void testFinished(int result);

	void showSettings();

	void slot_configurationArrived();

	void slot_logMessage(const QString &msg);
	void slot_logError(const QString &errMsg);

	void on_m_stop_clicked();

private:
	// Ui
	Ui::TestSuiteMainWindow *ui;

	QAction* m_pExitAction = nullptr;
	QAction* m_pSettingsAction = nullptr;

	// Main objects
	Log::LogFile m_LogFile;						// Must be initialized first

	TestLibrary m_testLibrary;

};

extern TestSuiteMainWindow* theMainWindow;

#endif // TESTSUITEMAINWINDOW_H
