#pragma once
#include "../include/DbStruct.h"

class CentralWidget;
class DbController;
class SchemesTabPage;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
	MainWindow(DbController* dbcontroller, QWidget* parent);
    ~MainWindow();

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

	CentralWidget* getCentralWidget();

	// Commands
	//
protected slots:
	void exit();

	void userManagement();
	void showLog();
	void showSettings();

	void runConfigurator();
    void runAfblEditor();
	void runSubsystemListEditor();
    void showAbout();
	void debug();

private slots:
	void projectOpened(DbProject project);
	void projectClosed();

	// Properties
	//
protected:
	DbController* dbController();
    
	// Data
	//
private:
	QAction* m_pExitAction;

	QAction* m_pUsersAction;
	QAction* m_pLogAction;
	QAction* m_pSettingsAction;

	QAction* m_pConfiguratorAction;
    QAction* m_pAfblEditorAction;
	QAction* m_pSubsystemListEditorAction;
    QAction* m_pAboutAction;
	QAction* m_pDebugAction;

	QLabel* m_pStatusBarInfo;
	QLabel* m_pStatusBarConnectionStatistics;
	QLabel* m_pStatusBarConnectionState;

	SchemesTabPage* m_logicScheme = nullptr;
	SchemesTabPage* m_workflowScheme = nullptr;
	SchemesTabPage* m_diagScheme = nullptr;

	DbController* m_dbController;
};

