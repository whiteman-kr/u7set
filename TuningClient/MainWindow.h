#pragma once

#include <QMainWindow>
#include <QTranslator>

#include "TuningWorkspace.h"
#include "SchemasWorkspace.h"
#include "ConfigController.h"
#include "UserManager.h"
#include "DialogTuningSources.h"
#include "../lib/Ui/DialogTcpStatistics.h"

class TuningTcpClient;
class DialogAlert;

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

private:

	TuningSignalManager m_tuningSignalManager;

	TuningClientTcpClient* m_tcpClient = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

	TuningClientFilterStorage m_filterStorage;

	ConfigController m_configController;

	QVBoxLayout* m_mainLayout = nullptr;

	QTabWidget* m_tabWidget = nullptr;

	LogonWorkspace* m_logonWorkspace = nullptr;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	std::vector<SchemasWorkspace*> m_schemasWorkspaces;

	QLabel* m_noWorkspaceLabel = nullptr;

	UserManager m_userManager;
public:

	int m_mainWindowTimerId_250ms = -1;
	int m_mainWindowTimerId_500ms = -1;

	DialogAlert* m_dialogAlert = nullptr;

private slots:
	void slot_configurationArrived();
	void slot_projectFiltersUpdated(QByteArray data);
	void slot_signalsUpdated(QByteArray data);

public slots:
	void exit();
	void runPresetEditor();
	void showSettings();
	void showTuningSources();
	void showStatistics();
	void showAppLog();
	void showSignalsLog();
	void showAboutQt();
	void showAbout();
	void showTuningUserManual();

	void slot_userFiltersChanged();

private:

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void closeEvent(QCloseEvent *event) override;
	virtual void timerEvent(QTimerEvent* event) override;

	void createAndCheckFiltersHashes(bool userFiltersOnly);

	void createWorkspace();

private:
	bool eventFilter(QObject *object, QEvent *event) override;

	void updateStatusBar();

signals:
	void timerTick500();

private:

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pStatisticsAction = nullptr;
	QAction* m_pAppLogAction = nullptr;
	QAction* m_pSignalLogAction = nullptr;
	QAction* m_aboutQtAction = nullptr;
	QAction* m_pAboutAction = nullptr;
	QAction* m_manualTuningAction = nullptr;

	QLabel* m_statusBarBuildInfo = nullptr;
	QLabel* m_statusBarLmControlMode = nullptr;
	std::vector<QLabel*> m_statusDiscreteCount;
	QLabel* m_statusBarLmErrors = nullptr;
	QLabel* m_statusBarSor = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;

	TuningLog::TuningLog* m_tuningLog = nullptr;

	bool m_singleLmControlMode = true;
	QString m_activeClientId;
	QString m_activeClientIp;
	int m_discreteCounter = -1;
	int m_lmErrorsCounter = -1;
	QString m_sorStatus;
	int m_logErrorsCounter = -1;
	int m_logWarningsCounter = -1;

	QString m_singleLmControlModeText;
	QString m_multipleLmControlModeText;

	DialogTuningSources* m_dialogTuningSources = nullptr;
	DialogTcpStatistics* m_dialogStatistics = nullptr;
};

// Global definitions

extern MainWindow* theMainWindow;

extern Log::LogFile* theLogFile;


