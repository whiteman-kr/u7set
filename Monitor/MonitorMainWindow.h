#pragma once

#include "MonitorConfigController.h"
#include "MonitorSchemaManager.h"
#include "TcpSignalClient.h"
#include "TcpSignalRecents.h"
#include "TcpAppSourcesState.h"
#include "SelectSchemaWidget.h"
#include "MonitorTuningTcpClient.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"
#include "../UtilsLib/LogFile.h"
#include "../lib/Ui/DialogAlert.h"
#include "../lib/TcpClientsStatistics.h"

class MonitorCentralWidget;
class MonitorToolBar;
class QLabel;
class QComboBox;
class DialogDataSources;

class MonitorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MonitorMainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
	~MonitorMainWindow();

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

	void showLogo();

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
	void showSoftwareConnection(const QString& caption, const QString& shortCaption, Tcp::ConnectionState connectionState, HostAddressPort portPrimary, HostAddressPort portSecondary, QLabel* label);

	// Commands
	//
protected slots:
	void exit();

	void schemaTreeListToggled(bool checked);

	void showLog();
	void showDataSources();
	void showSettings();
	void showStatistics();

	void showAboutQt();
	void showAbout();
	void showMatsUserManual();
	void debug();

	// slots
protected:
	void slot_archive();
	void slot_trends();

	void slot_signalSnapshot();
	void slot_findSignal();
	void slot_historyChanged(bool enableBack, bool enableForward);
	void slot_updateActions(bool schemaWidgetSelected);

	void slot_configurationArrived(ConfigSettings configuration);
	void slot_unknownClient();

	void checkMonitorSingleInstance();

	// Properties
	//
public:
	MonitorConfigController* configController();
	const MonitorConfigController* configController() const;

	TcpSignalClient* tcpSignalClient();
	const TcpSignalClient* tcpSignalClient() const;

protected:

	// Data
	//
private:
	// Monitor instance key value
	//
	static const QString m_monitorSingleInstanceKey;

	QSharedMemory m_appInstanceSharedMemory;

	// Instance checker timer
	//
	QTimer* m_instanceTimer = nullptr;

	MonitorConfigController m_configController;
	MonitorSchemaManager m_schemaManager;

	std::unique_ptr<VFrame30::AppSignalController> m_appSignalController;
	std::unique_ptr<VFrame30::TuningController> m_tuningController;
	std::unique_ptr<VFrame30::LogController> m_logController;

	TcpSignalClient* m_tcpSignalClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;			// +

	TcpSignalRecents* m_tcpSignalRecents = nullptr;
	SimpleThread* m_tcpRecentsThread = nullptr;			// +

	MonitorTuningTcpClient* m_tuningTcpClient = nullptr;
	SimpleThread* m_tuningTcpClientThread = nullptr;	// +

	TcpAppSourcesState* m_tcpSourcesStateClient = nullptr;
	SimpleThread* m_sourcesStateClientThread = nullptr;	// +

	Log::LogFile m_LogFile;

	DialogAlert m_dialogAlert;

	// File menu
	//
	QAction* m_pExitAction = nullptr;

	// Tools menu
	//
	QAction* m_pDataSourcesAction = nullptr;
	QAction* m_pStatisticsAction = nullptr;
	QAction* m_pSettingsAction = nullptr;

	// ? menu
	//
	QAction* m_pDebugAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutQtAction = nullptr;
	QAction* m_pAboutAction = nullptr;
	QAction* m_manualMatsAction = nullptr;

	QAction* m_schemaListAction = nullptr;
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

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

	// Logo
	//
	QLabel* m_logoLabel = nullptr;
	QImage m_logoImage;

	QWidget* m_spacer = nullptr;

	// Controls
	//
	MonitorToolBar* m_toolBar = nullptr;

	QDockWidget* m_schemaListDock = nullptr;

	SelectSchemaWidget* m_selectSchemaWidget = nullptr;

	QLabel* m_statusBarInfo = nullptr;

	QLabel* m_statusBarConfigConnection	= nullptr;
	QLabel* m_statusBarAppDataConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;

	QLabel* m_statusBarProjectInfo = nullptr;
	QLabel* m_statusBarLogAlerts = nullptr;

	int m_updateStatusBarTimerId = -1;

	int m_logErrorsCounter = -1;
	int m_logWarningsCounter = -1;

	DialogDataSources* m_dialogDataSources = nullptr;
	DialogStatistics* m_dialogStatistics = nullptr;
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

extern MonitorMainWindow* theMonitorMainWindow;

