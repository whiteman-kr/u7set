#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>

#include "TuningWorkspace.h"
#include "SchemasWorkspace.h"
#include "ConfigController.h"
#include "../lib/LogFile.h"
#include "UserManager.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"
#include "SchemaStorage.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = 0);
	~MainWindow();

private:
	void createActions();
	void createMenu();
	void createStatusBar();


private:

	TuningSignalManager m_tuningSignalManager;

	TuningClientTcpClient* m_tcpClient = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

	TuningClientFilterStorage m_filterStorage;

	ConfigController m_configController;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	SchemasWorkspace* m_schemasWorkspace = nullptr;

	int m_mainWindowTimerId_250ms = -1;
	int m_mainWindowTimerId_500ms = -1;

private slots:
	void slot_configurationArrived();
	void slot_projectFiltersUpdated(QByteArray data);
	void slot_schemasDetailsUpdated(QByteArray data);
	void slot_signalsUpdated(QByteArray data);
	void slot_schemasGlobalScriptArrived(QByteArray data);

public slots:
	void exit();
	void runPresetEditor();
	void runUsersEditor();
	void showSettings();
	void showTuningSources();
	void showLog();
	void showAbout();

private:

	virtual void timerEvent(QTimerEvent* event) override;

	void createWorkspace();

signals:
	void timerTick500();

private:

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pUsersAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;

	QString m_globalScript;
};

// Global definitions

extern MainWindow* theMainWindow;

extern Log::LogFile* theLogFile;

extern UserManager theUserManager;

#endif // MAINWINDOW_H
