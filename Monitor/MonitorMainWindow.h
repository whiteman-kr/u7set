#ifndef MONITORMAINWINDOW_H
#define MONITORMAINWINDOW_H

#include "MonitorConfigController.h"
#include "SchemaManager.h"
#include "TcpSignalClient.h"

class MonitorCentralWidget;
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

	// Public methods
	//
public:

	// Protected methods
	//
protected:
	void saveWindowState();
	void restoreWindowState();

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
	void slot_findSignal();

	// Properties
	//
protected:

	// Data
	//
private:
	MonitorConfigController m_configController;
	SchemaManager m_schemaManager;

	TcpSignalClient* m_tcpSignalClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

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

	QAction* m_findSignalAction = nullptr;

	// Controls
	//
	QToolBar* m_toolBar = nullptr;

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
	void slot_configurationArrived(ConfigSettings configuration);
	void slot_schemaChanged(QString strId);
	void slot_indexChanged(int index);

private:
	MonitorConfigController* m_configController = nullptr;
	MonitorCentralWidget* m_centraWidget = nullptr;

	QLabel* m_label = nullptr;
	QComboBox* m_comboBox = nullptr;
};

extern MonitorMainWindow* theMonitorMainWindow;

#endif // MONITORMAINWINDOW_H
