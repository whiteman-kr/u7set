#pragma once


#include "MonitorConfigController.h"
#include "MonitorSignalManager.h"
#include "SelectSchemaWidget.h"
#include "InstanceResolver.h"
#include "SchemaDrawStatistics.h"
//#include "../VFrame30/ClientSchemaView.h"
#include "../ClientLib/ClientTranslator.h"
#include "../ClientLib/AdsConnection.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../ClientLib/TuningUserManager.h"
#include "../ClientLib/TuningLog.h"
#include "../VFrame30/AppSignalController.h"
#include "../UtilsLib/LogFile.h"
#include "../lib/Ui/DialogAlert.h"
#include "../lib/Ui/DialogTcpStatistics.h"

class MonitorCentralWidget;
class MonitorToolBar;
class QLabel;
class QComboBox;

class MonitorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MonitorMainWindow(InstanceResolver& instanceResolver,  const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
	~MonitorMainWindow() = default;

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent* event) override;
	virtual void timerEvent(QTimerEvent* event) override;

	virtual void showEvent(QShowEvent* event) override;
	virtual bool eventFilter(QObject* object, QEvent* event) override;

	// Public methods
	//
public:
	static QString getInstanceKey();

	void showTrends(const std::vector<AppSignalParam>& appSignals);

	// Protected methods
	//
protected:
	void saveWindowState();
	void restoreWindowState();

	void showTuningLoginControls();
	void showLogo();
	void showZoomControls();

	// Private methods
	//
private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

public:
	MonitorCentralWidget* monitorCentralWidget();

private:
	void updateStatusBar();
	void showSoftwareConnection(const QString& caption,
								const std::vector<Tcp::ConnectionState>& connectionStates,
								QLabel* label);
	// Commands
	//
protected slots:
	void exit();

	void schemaTreeListToggled(bool checked);

	void showLog();
    void showTuningLog();
	void showDataSources();
	void showSettings();
	void showStatistics();

	void showAboutQt();
	void showAbout();
	void showMatsUserManual();
	void devTools();
	void debug();

	// Slots
	//
public slots:
	void slot_archive();
	void slot_archive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType);

	void slot_trends();

	void slot_signalSnapshot();
	void slot_signalSnapshot(QStringList signalsList);
	void slot_signalSnapshotByMask(QStringList masks);
	void slot_signalSnapshotByTag(QStringList tags);

	void slot_findSignal();
	void slot_historyChanged(bool enableBack, bool enableForward);
	void slot_updateActions(bool schemaWidgetSelected);

	void slot_configurationArrived(ConfigSettings configuration);
	void slot_configurationError(QString error);

	//void checkMonitorSingleInstance();
	void activateRequested();

	void toggleSchemaTree();
	void setVisibleSchemaTree(bool visible);
	void setVisibleTabBar(bool visible);
	void setVisibleToolBar(bool visible);
	void setVisibleStatusBar(bool visible);
	void setVisibleMenu(bool visible);
	void setFullScreen(bool value);

	void slot_login();
	void slot_reLogin();

	void slot_loggedIn();
	void slot_loggedOut();

private slots:
	void slot_tuningSignalsArrived(QByteArray data);

	// Properties
	//
public:
	MonitorConfigController& configController();
	const MonitorConfigController& configController() const;

	MonitorSignalManager& signalManager();
	const MonitorSignalManager& signalManager() const;

	ClientLib::TuningUserManager& userManager();
	const ClientLib::TuningUserManager& userManager() const;

	TuningSignalManager& tuningSignalManager();
	const TuningSignalManager& tuningSignalManager() const;

	ClientLib::TuningConnection& tuningConnection();
	const ClientLib::TuningConnection& tuningConnection() const;

	ITuningAuthorization& tuningAuthorization();
	const ITuningAuthorization& tuningAuthorization() const;

protected:

	// Data
	//
private:
	Log::LogFile m_LogFile;						// Must be initialized first
	ClientLib::TuningLog m_tuningLogFile;
	InstanceResolver& m_instanceResolver;

	MonitorConfigController m_configController;
	MonitorSignalManager m_signalManager;
	TuningSignalManager m_tuningSignalManager;

	MonitorSchemaManager m_schemaManager;

	std::unique_ptr<VFrame30::AppSignalController> m_appSignalController;
	std::unique_ptr<VFrame30::LogController> m_logController;

	ClientLib::AdsConnection m_adsConnection{m_signalManager, &m_signalManager, &m_LogFile};
	ClientLib::TuningConnection m_tuningConnection{m_tuningSignalManager, m_tuningSignalManager, m_tuningSignalManager, m_tuningUserManager, &m_LogFile, &m_tuningLogFile};

	DialogAlert m_dialogAlert;

	SchemaDrawStatistics m_schemaStats;

	// File menu
	//
	QAction* m_pExportAction = nullptr;
	QAction* m_pExitAction = nullptr;

	// Tools menu
	//
	QAction* m_pDataSourcesAction = nullptr;
	QAction* m_pStatisticsAction = nullptr;
	QAction* m_pSettingsAction = nullptr;

	// ? menu
	//
	QAction* m_pDevToolsAction = nullptr;
	QAction* m_pDebugAction = nullptr;
	QAction* m_pLogAction = nullptr;
    QAction* m_pTuningLogAction = nullptr;
	QAction* m_pAboutQtAction = nullptr;
	QAction* m_pAboutAction = nullptr;
	QAction* m_manualMatsAction = nullptr;

	QAction* m_schemaListAction = nullptr;
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

	QAction* m_zoomToolBarSeparator = nullptr;
	QAction* m_zoomInAction = nullptr;
	QAction* m_zoomOutAction = nullptr;
	QAction* m_zoom100Action = nullptr;
	QAction* m_zoomToFitAction = nullptr;

	QAction* m_historyBack = nullptr;
	QAction* m_historyForward = nullptr;

	QAction* m_archiveAction = nullptr;
	QAction* m_trendsAction = nullptr;

	QAction* m_signalSnapshotAction = nullptr;
	QAction* m_findSignalAction = nullptr;

	QAction* m_loginAction = nullptr;
	QAction* m_loginUserTimeoutAction = nullptr;
	QAction* m_logoSeparator = nullptr;

	// Logo
	//
	QLabel* m_logoLabel = nullptr;
	QPixmap m_logoImage;

	QWidget* m_spacer = nullptr;

	// Controls
	//
	MonitorToolBar* m_toolBar = nullptr;
	QDockWidget* m_schemaListDock = nullptr;

	SelectSchemaWidget* m_selectSchemaWidget = nullptr;

	// Status bar
	//
	QLabel* m_statusBarInfo = nullptr;

	QLabel* m_statusBarConfigConnection	= nullptr;
	QLabel* m_statusBarAppDataConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;

	QLabel* m_statusBarProjectInfo = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;

	int m_updateStatusBarTimerId = -1;

	int m_logErrorsCounter = -1;
	int m_logWarningsCounter = -1;

	// Translator
	//
	ClientLib::ClientTranslator m_translator;

	// --
	//
	DialogTcpStatistics* m_dialogStatistics = nullptr;

	ClientLib::TuningUserManager m_tuningUserManager;
};


class MonitorToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit MonitorToolBar(const QString& tittle, QWidget* parent = Q_NULLPTR);

public:
	void addAction(QAction* action);

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dropEvent(QDropEvent* event) override;
};


