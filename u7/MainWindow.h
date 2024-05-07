#pragma once
#include <DbLib/DbStruct.h>
#include <UiLib/ClickableLabel.h>
#include "./Locator/EquipmentLocatorProvider.h"
#include "./Locator/ConnectionLocatorProvider.h"
#include "./Locator/SchemaLocatorProvider.h"
#include "./Locator/AppSignalLocatorProvider.h"
#include "./Locator/LocatorListWidget.h"
#include "./Locator/LocatorEditControl.h"

class CentralWidget;
class DbController;
class ProjectsTabPage;
class SchemasTabPage;
class SchemasTabPage;
class EquipmentTabPage;
class SignalsTabPage;
class FilesTabPage;
class BuildTabPage;
class UploadTabPage;
class SimulatorTabPage;
class TestsTabPage;
class DialogShortcuts;
class AppSignalSetProvider;
class ProjectDiffGenerator;

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

	bool preCloseConditions();

	// Private methods
	//
private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	CentralWidget* getCentralWidget();

public slots:
    void onMiniDumpCreated(QString dumpFilePath, bool result);


protected slots:
	void currentTabChanged(int tabIndex);

	// Commands
	//
	void exit();

	void userManagement();
	void showSettings();
	void showShortcuts();
	void showRpctUserManual();
	void showRpctInstallManual();
	void showRpctQuickStart();
	void showRpctUserManualAppendixA();
	void showRpctUserManualAppendixB();
	void showAfblReference();
	void showScriptHelp();
	void showMatsUserManual();
	void showTuningUserManual();

	void runConfigurator();
	void runSubsystemListEditor();
    void runConnectionsEditor();
	void runDiagSignalTypesEditor();
	void runBusEditor();
	void runTagsEditor();
	void runSimulationProfilesEditor();
	void runMatsUserEditor();
	void updateUfbsAfbsBusses();
	void afbLibraryCheck();
    void showAbout();
	void showAboutQt();
	void debug();
	void startBuild();
	void projectHistory();
	void projectProperties();
	void projectDifference();
	void createSchemasAlbums();
	void pendingChanges();

private slots:
	void projectOpened(DbProject project);
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
	QAction* m_settingsAction = nullptr;
	QAction* m_shortcutsAction = nullptr;

	QAction* m_manualRpctAction = nullptr;
	QAction* m_installRpctAction = nullptr;
	QAction* m_rpctQuickStartAction = nullptr;
	QAction* m_manualRpctAppendixAAction = nullptr;
	QAction* m_manualRpctAppendixBAction = nullptr;
	QAction* m_manualAfblAction = nullptr;
	QAction* m_scriptHelpAction = nullptr;
	QAction* m_manualMatsAction = nullptr;
	QAction* m_manualTuningAction = nullptr;

	QAction* m_subsystemListEditorAction = nullptr;
    QAction* m_connectionsEditorAction = nullptr;
	QAction* m_diagSignalTypesEditorAction = nullptr;
	QAction* m_busEditorAction = nullptr;
	QAction* m_tagsEditorAction = nullptr;
	QAction* m_matsUsersEditorAction = nullptr;
	QAction* m_simProfilesEditorAction = nullptr;
	QAction* m_updateUfbsAfbs = nullptr;
	QAction* m_AfbLibraryCheck = nullptr;
	QAction* m_aboutAction = nullptr;
	QAction* m_aboutQtAction = nullptr;
	QAction* m_debugAction = nullptr;
	QAction* m_startBuildAction = nullptr;
	QAction* m_projectHistoryAction = nullptr;
	QAction* m_projectPropertiesAction = nullptr;
	QAction* m_projectDifferenceAction = nullptr;
	QAction* m_schemasAlbumAction = nullptr;
	QAction* m_pendingChangesAction = nullptr;

	QAction* m_locatorAction = nullptr;

	Locator::EquipmentLocatorProvider m_equipmentLocatorProvider;
	Locator::ConnectionLocatorProvider m_connectionLocatorProvider;
	Locator::SchemaLocatorProvider m_schemaLocatorProvider;
	Locator::AppSignalLocatorProvider m_appSignalLocatorProvider;
	Locator::LocatorListWidget m_locatorListWidget;
	Locator::Locator m_locator{m_locatorListWidget};
	Locator::LocatorEditControl* m_locatorEditControl = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	UiLib::ClickableLabel* m_statusBarSchemaLayerLabel = nullptr;		// Specific information for schemas tab
	UiLib::ClickableLabel* m_statusBarSchemaZoomLabel = nullptr;		// Specific information for schemas tab
	QLabel* m_statusBarConnectionState = nullptr;

	ProjectsTabPage* m_projectsTab = nullptr;
	EquipmentTabPage* m_equipmentTab = nullptr;
	SignalsTabPage* m_signalsTab = nullptr;
	SchemasTabPage* m_schemaTabPage = nullptr;
	BuildTabPage* m_buildTabPage = nullptr;
	UploadTabPage* m_uploadTabPage = nullptr;
	SimulatorTabPage* m_simulatorTabPage = nullptr;
	TestsTabPage* m_testsTabPage = nullptr;

	DbController* m_dbController = nullptr;
	AppSignalSetProvider* m_signalSetProvider = nullptr;

	DialogShortcuts* m_dialogShortcuts = nullptr;

	int m_filesTabPageIndex = 0;
	FilesTabPage* m_filesTabPage = nullptr;

//#ifdef Q_OS_WINDOWS		class QWinTaskbarButton removed from qt6
//	QWinTaskbarButton* m_taskBarButton = nullptr;
//	int m_timerId = -1;
//#endif

	int m_visibleTimerId = -1;
};

