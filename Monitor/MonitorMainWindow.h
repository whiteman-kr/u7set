#ifndef MONITORMAINWINDOW_H
#define MONITORMAINWINDOW_H

#include "MonitorConfigController.h"
#include "SchemaManager.h"
#include "TcpSignalClient.h"
#include "TcpSignalRecents.h"

class MonitorCentralWidget;
class MonitorToolBar;
class SchemaListWidget;
class QLabel;
class QComboBox;

class MonitorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MonitorMainWindow(QWidget* parent = nullptr);
	~MonitorMainWindow();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void showEvent(QShowEvent*) override;

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

	MonitorCentralWidget* monitorCentralWidget();

	// Commands
	//
protected slots:
	void exit();

	void showLog();
	void showSettings();

	void showAbout();
	void debug();

	// slots
protected:
	void slot_archive();
	void slot_trends();

	void slot_signalSnapshot();
	void slot_findSignal();
	void slot_historyChanged(bool enableBack, bool enableForward);

	void tcpSignalClient_signalParamAndUnitsArrived();
	void tcpSignalClient_connectionReset();

	void checkMonitorSingleInstance();

signals:
	void signalParamAndUnitsArrived();
	void connectionReset();

	// Properties
	//
public:
	MonitorConfigController* configController();
	const MonitorConfigController* configController() const;

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
	SchemaManager m_schemaManager;

	TcpSignalClient* m_tcpSignalClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

	TcpSignalRecents* m_tcpSignalRecents = nullptr;
	SimpleThread* m_tcpRecentsThread = nullptr;

	// File menu
	//
	QAction* m_pExitAction = nullptr;

	// Tools menu
	//
	QAction* m_pSettingsAction = nullptr;

	// ? menu
	//
	QAction* m_pDebugAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

	QAction* m_zoomInAction = nullptr;
	QAction* m_zoomOutAction = nullptr;
	QAction* m_zoom100Action = nullptr;

	QAction* m_historyBack = nullptr;
	QAction* m_historyForward = nullptr;

	QAction* m_archiveAction = nullptr;
	QAction* m_trendsAction = nullptr;

	QAction* m_signalSnapshotAction = nullptr;
	QAction* m_findSignalAction = nullptr;

	// Logo
	//
	QLabel* m_logoLabel = nullptr;
	QWidget* m_spacer = nullptr;

	// Controls
	//
	MonitorToolBar* m_toolBar = nullptr;

	SchemaListWidget* m_schemaListWidget = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;

	int m_updateStatusBarTimerId = -1;
};

class SchemaListWidget : public QWidget
{
	Q_OBJECT

public:
	SchemaListWidget(MonitorConfigController* configController, MonitorCentralWidget* centralWidget);
	virtual ~SchemaListWidget();

signals:
	void selectionChanged(QString schemaId);

protected slots:
	void slot_configurationArrived(ConfigSettings);
	void slot_schemaChanged(QString strId);
	void slot_indexChanged(int index);

private:
	MonitorConfigController* m_configController = nullptr;
	MonitorCentralWidget* m_centraWidget = nullptr;

	QComboBox* m_comboBox = nullptr;
};

class MonitorToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit MonitorToolBar(const QString &tittle, QWidget* parent = Q_NULLPTR);

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dropEvent(QDropEvent* event) override;

};

extern MonitorMainWindow* theMonitorMainWindow;

#endif // MONITORMAINWINDOW_H
