#ifndef MONITORMAINWINDOW_H
#define MONITORMAINWINDOW_H

#include "MonitorConfigController.h"
#include "SchemaManager.h"

class MonitorCentralWidget;

class MonitorMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MonitorMainWindow(MonitorConfigController* configController, QWidget* parent = nullptr);
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

	MonitorConfigController* m_configController = nullptr;
	SchemaManager m_schemaManager;

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

	// Controls
	//
	QLabel* m_pStatusBarInfo = nullptr;
	QLabel* m_pStatusBarConnectionStatistics = nullptr;
	QLabel* m_pStatusBarConnectionState = nullptr;
};

#endif // MONITORMAINWINDOW_H
