#pragma once

#include "TuningWorkspace.h"
#include "SchemasWorkspace.h"
#include "TuningConfigController.h"
#include "LogonWorkspace.h"
#include "TuningSignalListSet.h"
#include "DialogTuningSources.h"
#include <ClientLib/ClientTranslator.h>
#include <ClientLib/TuningConnection.h>
#include <ClientLib/TuningUserManager.h>
#include <ClientLib/TuningLog.h>
#include <TuningLib/TuningUiItem.h>
#include <SchemaClientLib/DialogTcpStatistics.h>
#include "../UtilsLib/LogFile.h"

namespace UiLib
{
	class DialogAlert;
}

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = 0);
	~MainWindow();

	ClientLib::TuningSignalManager& tuningSignalManager();
	const ClientLib::TuningSignalManager& tuningSignalManager() const;

	ClientLib::TuningConnection& tuningConnection();
	const ClientLib::TuningConnection& tuningConnection() const;

	ITuningAuthorization& tuningAuthorization();
	const ITuningAuthorization& tuningAuthorization() const;

private:
	void createActions();
	void createMenu();
	void createStatusBar();
	void loadSignalLists();

private slots:
	void slot_configurationArrived(TuningClientConfigSettings configuration);
	void slot_uiUpdated(QByteArray data);
	void slot_signalsUpdated(QByteArray data);

	void slot_configurationError(QString error);

public slots:
	void exit();
	void showSettings();
	void showTuningSources();
	void showStatistics();
	void showAppLog();
	void showSignalsLog();
	void showAboutQt();
	void showAbout();
	void showTuningUserManual();
	void showAppSignalListEditor();
	void slot_userFiltersChanged();

private:

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void closeEvent(QCloseEvent *event) override;
	virtual void timerEvent(QTimerEvent* event) override;

	void createWorkspace();
	void deleteWorkspace();


private:
	bool eventFilter(QObject *object, QEvent *event) override;

	void updateStatusBar();
	void showSoftwareConnection(const QString& caption,
								const std::vector<Tcp::ConnectionState>& connectionStates,
								QLabel* label);


signals:
	void timerTick500();

public:
	int m_mainWindowTimerId_500ms = -1;

private:
	// Logs, must be initialized first
	//
	Log::LogFile m_logFile;
	ClientLib::TuningLog m_tuningLog;

	// Base objects
	//
	TuningConfigController m_configController;
	ClientLib::TuningSignalManager m_tuningSignalManager;
	ClientLib::TuningUserManager m_userManager;

	// Connections
	//
	ClientLib::TuningConnection m_tuningConnection;

	// AppSignalLists
	//
	TuningSignalListSet m_appSignalListSet;

	// Ui
	//
	TuningClientUiStorage m_tuningUi;

	// Workspace items
	//
	TuningWorkspace* m_tuningWorkspace = nullptr;
	std::vector<SchemasWorkspace*> m_schemasWorkspaces;

	DialogTuningSources* m_dialogTuningSources = nullptr;
	SchemaClientLib::DialogTcpStatistics* m_dialogStatistics = nullptr;
	UiLib::DialogAlert* m_dialogAlert = nullptr;

	// User interface
	//
	ClientLib::ClientTranslator m_translator;

	QLabel* m_noWorkspaceLabel = nullptr;

	QVBoxLayout* m_mainLayout = nullptr;
	QTabWidget* m_tabWidget = nullptr;

	QAction* m_pExitAction = nullptr;
	QAction* m_pAppSignalListsAction = nullptr;
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

	// Login controls
	//
	LogonWidget* m_logonWidget = nullptr;

	// Status bar counters
	//
	int m_discreteCounter = -1;
	QString m_sorStatus;
	int m_logErrorsCounter = -1;
	int m_logWarningsCounter = -1;

	QString m_sorTooltipText;
};



