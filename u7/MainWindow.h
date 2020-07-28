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
class TestsTabPage;
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

	CentralWidget* getCentralWidget();

	// Commands
	//
protected slots:
	void exit();

	void userManagement();
	void showLog();
	void showSettings();
	void showShortcuts();

	void showRpctUserManual();
	void showRpctUserManualAppendixA();
	void showAfblReference();
	void showScriptHelp();
	void showMatsUserManual();
	void showTuningUserManual();

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
	void projectAboutToBeClosed();
	void projectClosed();

	void buildStarted();
	void buildFinished(int errorCount);

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

	QAction* m_manualRpctAction = nullptr;
	QAction* m_manualRpctAppendixAAction = nullptr;
	QAction* m_manualAfblAction = nullptr;
	QAction* m_scriptHelpAction = nullptr;
	QAction* m_manualMatsAction = nullptr;
	QAction* m_manualTuningAction = nullptr;

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
	TestsTabPage* m_testsTabPage = nullptr;

	DbController* m_dbController = nullptr;

	DialogShortcuts* m_dialogShortcuts = nullptr;

	int m_filesTabPageIndex = 0;
	FilesTabPage* m_filesTabPage = nullptr;

#ifdef Q_OS_WINDOWS
	QWinTaskbarButton* m_taskBarButton = nullptr;
	int m_timerId = -1;
#endif
};

