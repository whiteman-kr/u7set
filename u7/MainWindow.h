#pragma once
#include "../lib/DbStruct.h"

class CentralWidget;
class DbController;
class ProjectsTabPage;
class SchemasTabPage;
class SchemasTabPageEx;
class EquipmentTabPage;
class SignalsTabPage;
class FilesTabPage;
class BuildTabPage;
class UploadTabPage;
class SimulatorTabPage;
class DialogShortcuts;

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
	virtual void showEvent(QShowEvent* event) override;

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
	void showShortcuts();

	void runConfigurator();
	void runSubsystemListEditor();
    void runConnectionsEditor();
	void runBusEditor();
	void updateUfbsAfbsBusses();
	void afbLibraryCheck();
    void showAbout();
	void debug();
	void startBuild();
	void projectHistory();
	void projectProperties();
	void pendingChanges();

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
	QAction* m_shortcutsAction = nullptr;

	QAction* m_subsystemListEditorAction = nullptr;
    QAction* m_connectionsEditorAction = nullptr;
	QAction* m_busEditorAction = nullptr;
	QAction* m_updateUfbsAfbs = nullptr;
	QAction* m_AfbLibraryCheck = nullptr;
	QAction* m_aboutAction = nullptr;
	QAction* m_debugAction = nullptr;
	QAction* m_startBuildAction = nullptr;
	QAction* m_projectHistoryAction = nullptr;
	QAction* m_projectPropertiesAction = nullptr;
	QAction* m_pendingChangesAction = nullptr;
	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarSchemaZoom = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;

	ProjectsTabPage* m_projectsTab = nullptr;
	EquipmentTabPage* m_equipmentTab = nullptr;
	SignalsTabPage* m_signalsTab = nullptr;
	SchemasTabPageEx* m_editSchemaTabPage = nullptr;
	BuildTabPage* m_buildTabPage = nullptr;
	UploadTabPage* m_uploadTabPage = nullptr;
	SimulatorTabPage* m_simulatorTabPage = nullptr;

	DbController* m_dbController = nullptr;

	DialogShortcuts* m_dialogShortcuts = nullptr;

	int m_filesTabPageIndex = 0;
	FilesTabPage* m_filesTabPage = nullptr;
};

