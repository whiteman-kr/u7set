#ifndef MONITORMAINWINDOW_H
#define MONITORMAINWINDOW_H

#include "MonitorConfigController.h"
#include "SchemaManager.h"
#include "TcpSignalClient.h"

class MonitorCentralWidget;

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

	// Controls
	//
	QToolBar* m_toolBar = nullptr;
	QLabel* m_pStatusBarInfo = nullptr;
	QLabel* m_pStatusBarConnectionStatistics = nullptr;
	QLabel* m_pStatusBarConnectionState = nullptr;
};

#endif // MONITORMAINWINDOW_H
