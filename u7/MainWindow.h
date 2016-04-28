#pragma once
#include "../include/DbStruct.h"

class CentralWidget;
class DbController;
class ProjectsTabPage;
class SchemasTabPage;
class EquipmentTabPage;
class SignalsTabPage;
class FilesTabPage;
class BuildTabPage;

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
	void startBuild();

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
	QAction* m_startBuildAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarSchemaZoom = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;

	ProjectsTabPage* m_projectsTab = nullptr;
	EquipmentTabPage* m_equipmentTab = nullptr;
	SignalsTabPage* m_signalsTab = nullptr;
	SchemasTabPage* m_logicSchema = nullptr;
	SchemasTabPage* m_monitorSchema = nullptr;
	BuildTabPage* m_buildTabPage = nullptr;
	//SchemasTabPage* m_diagSchema = nullptr;

	DbController* m_dbController = nullptr;

	int m_filesTabPageIndex = 0;
	FilesTabPage* m_filesTabPage = nullptr;
};

