#pragma once
#include "../include/DbStruct.h"

class CentralWidget;
class DbController;
class SchemasTabPage;
class FilesTabPage;

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
    void runConnectionsEditor();
	void runRS232SignalListEditor();
    void showAbout();
	void debug();

private slots:
	void projectOpened(DbProject project);
	void projectClosed();

	// Properties
	//
protected:
	DbController* dbController();
	DbController* db();
    
	// Data
	//
private:
	QAction* m_exitAction = nullptr;

	QAction* m_usersAction = nullptr;
	QAction* m_logAction = nullptr;
	QAction* m_settingsAction = nullptr;

	QAction* m_configuratorAction = nullptr;
	QAction* m_afblEditorAction = nullptr;
	QAction* m_subsystemListEditorAction = nullptr;
    QAction* m_connectionsEditorAction = nullptr;
	QAction* m_rs232SignalListEditorAction = nullptr;
	QAction* m_aboutAction = nullptr;
	QAction* m_debugAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarSchemaZoom = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;

	SchemasTabPage* m_logicSchema = nullptr;
	SchemasTabPage* m_monitorSchema = nullptr;
	//SchemasTabPage* m_diagSchema = nullptr;

	DbController* m_dbController = nullptr;

	int m_filesTabPageIndex = 0;
	FilesTabPage* m_filesTabPage = nullptr;
};

