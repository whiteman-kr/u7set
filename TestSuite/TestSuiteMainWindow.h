#ifndef TESTSUITEMAINWINDOW_H
#define TESTSUITEMAINWINDOW_H

#include <QMainWindow>
#include "../UtilsLib/LogFile.h"
#include "../TestSuiteLib/TestEngine.h"
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

	TestSuiteConfigController& configController();
	const TestSuiteConfigController& configController() const;

private:
	void createActions();
	void createMenu();
	void createStatusBar();

private slots:
	void exit();
	void on_m_run_clicked();
	void newLogItem(const TestLogItem& item);
	void testFinished(int result);

	void showSettings();

	void slot_configurationArrived(ConfigSettings configuration);
	void slot_unknownClient(QString errMsg);
	void slot_wrongClientHostname(QString errMsg);


private:
	Log::LogFile m_LogFile;						// Must be initialized first

	Ui::TestSuiteMainWindow *ui;

	QAction* m_pExitAction = nullptr;
	QAction* m_pSettingsAction = nullptr;

	TestEngine* m_testEngine = nullptr;

	TestSuiteConfigController m_configController;
};

extern TestSuiteMainWindow* theMainWindow;

#endif // TESTSUITEMAINWINDOW_H
