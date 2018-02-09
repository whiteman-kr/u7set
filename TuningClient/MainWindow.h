#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>

#include "TuningWorkspace.h"
#include "SchemasWorkspace.h"
#include "ConfigController.h"
#include "../lib/LogFile.h"
#include "../lib/Tuning/TuningLog.h"
#include "UserManager.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"
#include "SchemaStorage.h"
#include "DialogAlert.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = 0);
	~MainWindow();

	UserManager* userManager();

private:
	void createActions();
	void createMenu();
	void createStatusBar();

	void closeEvent(QCloseEvent *event) override;


private:

	TuningSignalManager m_tuningSignalManager;

	TuningClientTcpClient* m_tcpClient = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

	TuningClientFilterStorage m_filterStorage;

	ConfigController m_configController;

	QVBoxLayout* m_mainLayout = nullptr;

	LogonWorkspace* m_logonWorkspace = nullptr;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	SchemasWorkspace* m_schemasWorkspace = nullptr;

	UserManager m_userManager;
public:

	int m_mainWindowTimerId_250ms = -1;
	int m_mainWindowTimerId_500ms = -1;

	DialogAlert* m_dialogAlert = nullptr;

private slots:
	void slot_configurationArrived();
	void slot_projectFiltersUpdated(QByteArray data);
	void slot_schemasDetailsUpdated(QByteArray data);
	void slot_signalsUpdated(QByteArray data);
	void slot_schemasGlobalScriptArrived(QByteArray data);

public slots:
	void exit();
	void runPresetEditor();
	void showSettings();
	void showTuningSources();
	void showLog();
	void showAbout();

private:

	virtual void timerEvent(QTimerEvent* event) override;

	void createWorkspace();

private:
	bool eventFilter(QObject *object, QEvent *event) override;

signals:
	void timerTick500();

private:

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusDiscreteCount = nullptr;
	QLabel* m_statusBarLmErrors = nullptr;
	QLabel* m_statusBarSor = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;

	QString m_globalScript;

	TuningLog::TuningLog* m_tuningLog = nullptr;
};

// Global definitions

extern MainWindow* theMainWindow;

extern Log::LogFile* theLogFile;

#endif // MAINWINDOW_H

