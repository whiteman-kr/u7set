#include "MainWindow.h"

#include "./EquipmentEditor/EquipmentTabPage.h"
#include "./Forms/DialogDiagSignalTypes.h"
#include "./Forms/DialogProjectDiff.h"
#include "./Forms/FileHistoryDialog.h"
#include "./Forms/PendingChangesDialog.h"
#include "./Forms/ProjectPropertiesForm.h"
#include "./ProjectsTabPage/ProjectsTabPage.h"
#include "./Reports/SchemasReport.h"
#include "./SchemaEditor/EditSchemaWidget.h"
#include "./SchemaEditor/F2KeyForSchemaItem.h"
#include "./SchemaEditor/SchemasTabPage.h"
#include "./Simulator/SimProfileEditor.h"
#include "AppSignalSetProvider.h"
#include "BuildTabPage.h"
#include "CentralWidget.h"
#include "DialogAfbLibraryCheck.h"
#include "DialogAppSignalLists.h"
#include "DialogBusEditor.h"
#include "DialogConnections.h"
#include "DialogMatsUsersEditor.h"
#include "DialogSettings.h"
#include "DialogShortcuts.h"
#include "DialogSubsystemListEditor.h"
#include "DialogTagsEditor.h"
#include "FilesTabPage.h"
#include "GlobalMessanger.h"
#include "ProjectDefaults.h"
#include "Settings.h"
#include "SignalsTabPage.h"
#include "SimulatorTabPage.h"
#include "TestsTabPage.h"
#include "UploadTabPage.h"
#include "UserManagementDialog.h"

#include "../Builder/LogicModuleSet.h"

#include <LicenseLib/AppLicenser.h>
#include <UiLib/DialogAbout.h>
#include <UiLib/UiTools.h>
#include <VFrame30/ActuatorHeader.h>
#include <VFrame30/PropertyNames.h>


MainWindow::MainWindow(DbController* dbcontroller, QWidget* parent) :
	QMainWindow{parent},
	m_connectionLocatorProvider{dbcontroller},
	m_schemaLocatorProvider{dbcontroller},
	m_locatorListWidget{dbcontroller, this},
	m_dbController{dbcontroller}
{
	assert(m_dbController);

	// --
	//
	CentralWidget* central = new CentralWidget();
	setCentralWidget(central);

	connect(central, &QTabWidget::currentChanged, this, &MainWindow::currentTabChanged);

	// Create Menus, ToolBars, StatusBar
	//
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &MainWindow::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &MainWindow::projectClosed);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::changeCurrentTab, getCentralWidget(), &CentralWidget::setCurrentWidget);

	m_signalSetProvider = new AppSignalSetProvider(dbController(), this);

	// Add main tab pages
	//
	m_projectsTab = new ProjectsTabPage{dbController(),
										[this]()
										{
											return preCloseConditions();
										},
										nullptr};

	m_equipmentTab = new EquipmentTabPage(dbController(), nullptr);
	m_signalsTab =
		new SignalsTabPage(AppSignalSetProvider::getInstance(), AppSignalPropertyManager::getInstance(), dbController(), nullptr);

	m_filesTabPage = new FilesTabPage(dbController(), nullptr);
	m_filesTabPage->setWindowTitle(tr("Files"));

	getCentralWidget()->addTabPage(m_projectsTab, tr("Projects"));
	getCentralWidget()->addTabPage(m_equipmentTab, tr("Equipment"));
	getCentralWidget()->addTabPage(m_signalsTab, tr("Application Signals"));

	connect(getCentralWidget(), &QTabWidget::currentChanged, m_signalsTab, &SignalsTabPage::onTabPageChanged);

	m_filesTabPageIndex = getCentralWidget()->addTabPage(m_filesTabPage, m_filesTabPage->windowTitle());
	getCentralWidget()->removeTab(m_filesTabPageIndex); // It will be added in projectOpened slot if required

	m_schemaTabPage = new SchemasTabPage{db(), m_signalSetProvider, m_statusBarSchemaLayerLabel, m_statusBarSchemaZoomLabel, this};
	getCentralWidget()->addTabPage(m_schemaTabPage, tr("Schemas"));

	m_buildTabPage = new BuildTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_buildTabPage, tr("Build"));

	m_simulatorTabPage = new SimulatorTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_simulatorTabPage, tr("Simulator"));

	m_testsTabPage = new TestsTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_testsTabPage, tr("Tests"));

	m_uploadTabPage = new UploadTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_uploadTabPage, tr("Upload"));

	// --
	//
	connect(m_buildTabPage, &BuildTabPage::buildStarted, this, &MainWindow::buildStarted);
	connect(m_buildTabPage, &BuildTabPage::buildFinished, this, &MainWindow::buildFinished);

	// Init locators
	//
	m_equipmentLocatorProvider.setEquipmentTabPage(m_equipmentTab);
	m_appSignalLocatorProvider.setSignalProvider(m_signalSetProvider);

	m_locator.addProvider(m_equipmentLocatorProvider);
	m_locator.addProvider(m_connectionLocatorProvider);
	m_locator.addProvider(m_schemaLocatorProvider);
	m_locator.addProvider(m_appSignalLocatorProvider);

	// --
	//
	setMinimumSize(500, 300);
	restoreWindowState();

	centralWidget()->show();

	m_visibleTimerId = startTimer(50);

	return;
}

MainWindow::~MainWindow()
{
	qDebug() << Q_FUNC_INFO;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	// Cancel build
	//
	assert(m_buildTabPage);

	if (m_buildTabPage != nullptr)
	{
		m_buildTabPage->cancelBuild();
	}

	// Check if any schema or test is not saved
	//
	if (bool canBeClosed = preCloseConditions(); canBeClosed == false)
	{
		e->ignore();
	}
	else
	{
		saveWindowState(); // Save windows state and accept event to close app

		e->accept();
		qApp->closeAllWindows();
	}

	return;
}

void MainWindow::showEvent(QShowEvent*)
{
	// Ensure widget is visible
	//
	QRect screenRect = this->screen()->availableGeometry();
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false && (intersectRect.width() < size().width() || intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false && (frameGeometry().width() > screenRect.width() || frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7), static_cast<int>(screenRect.height() * 0.7));
	}

	// #ifdef Q_OS_WINDOWS
	//	m_taskBarButton = new QWinTaskbarButton(this);
	//	m_taskBarButton->setWindow(windowHandle());
	// #endif
	return;
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	// #ifdef Q_OS_WINDOWS
	//	if (m_buildTabPage->isBuildRunning() == true && m_taskBarButton != nullptr)
	//	{
	//		m_taskBarButton->progress()->setValue(m_buildTabPage->progress());
	//	}
	// #endif

	if (event->timerId() == m_visibleTimerId && isVisible() == true)
	{
		killTimer(m_visibleTimerId);

		// Refresh project list only once
		//
		Q_ASSERT(m_projectsTab);
		m_projectsTab->refreshProjectList();
	}
}

void MainWindow::saveWindowState()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theSettings.writeUserScope();
}

void MainWindow::restoreWindowState()
{
	theSettings.loadUserScope();

	move(theSettings.m_mainWindowPos);
	restoreGeometry(theSettings.m_mainWindowGeometry);
	restoreState(theSettings.m_mainWindowState);

	return;
}

bool MainWindow::preCloseConditions()
{
	if (db()->isProjectOpened() == false)
	{
		return true;
	}

	if (m_schemaTabPage == nullptr || m_testsTabPage == nullptr)
	{
		assert(m_schemaTabPage);
		assert(m_testsTabPage);
		return false;
	}

	bool unsavedSchemas = m_schemaTabPage->hasUnsavedSchemas();
	bool unsavedTests = m_testsTabPage->hasUnsavedTests();

	bool satisfies = true;

	if (int nu = (unsavedSchemas ? 2 : 0) | (unsavedTests ? 1 : 0); nu != 0)
	{
		QString message;

		switch (nu)
		{
		case 1:
			message = tr("There are unsaved files on Tests tab page.");
			break;
		case 2:
			message = tr("There are unsaved files on Schemas tab page.");
			break;
		case 3:
			message = tr("There are unsaved files on Schemas and Tests tab pages.");
			break;
		default:
			assert(false);
		}

		auto result = QMessageBox::question(this,
											QApplication::applicationName(),
											message,
											QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel,
											QMessageBox::SaveAll);
		switch (result)
		{
		case QMessageBox::Cancel:
			satisfies = false;
			break;

		case QMessageBox::SaveAll:
			m_schemaTabPage->saveUnsavedSchemas(); // It will reset modified flag
			m_testsTabPage->saveUnsavedTests();    // It will reset modified flag
			satisfies = true;
			break;

		case QMessageBox::Discard:
			// Reset modified flag for all opened files, so on closeEvent
			// for these files prompt to save them will not be shown
			m_schemaTabPage->resetModified();
			m_testsTabPage->resetModified();
			satisfies = true;
			break;

		default:
			satisfies = true;
			assert(false);
		}
	}

	if (satisfies == true)
	{
		// Save opened schema list so it can be restored on next project open event
		//
		m_schemaTabPage->saveSession();
		m_equipmentTab->saveSession();
	}

	return satisfies;
}

void MainWindow::createActions()
{
	m_exitAction = new QAction(tr("Exit"), this);
	m_exitAction->setStatusTip(tr("Quit the application"));
	m_exitAction->setShortcut(QKeySequence::Quit);
	m_exitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_exitAction->setEnabled(true);
	connect(m_exitAction, &QAction::triggered, this, &MainWindow::exit);

	m_usersAction = new QAction(tr("Users..."), this);
	m_usersAction->setStatusTip(tr("User management"));
	m_usersAction->setEnabled(false);
	connect(m_usersAction, &QAction::triggered, this, &MainWindow::userManagement);

	m_manualRpctAction = new QAction(tr("RPCT User Manual"), this);
	m_manualRpctAction->setStatusTip(tr("Show RPCT User Manual"));
	connect(m_manualRpctAction, &QAction::triggered, this, &MainWindow::showRpctUserManual);

	m_installRpctAction = new QAction(tr("Installing and configuring RPCT"), this);
	m_installRpctAction->setStatusTip(tr("Show Installing and configuring RPCT"));
	connect(m_installRpctAction, &QAction::triggered, this, &MainWindow::showRpctInstallManual);

	m_rpctQuickStartAction = new QAction(tr("RPCT Quick Start Guide"), this);
	m_rpctQuickStartAction->setStatusTip(tr("Show RPCT Quick Start Guide"));
	connect(m_rpctQuickStartAction, &QAction::triggered, this, &MainWindow::showRpctQuickStart);

	m_manualRpctAppendixAAction = new QAction(tr("Appendix A - Errors and Warnings List"), this);
	m_manualRpctAppendixAAction->setStatusTip(tr("Show Appendix A - Errors and Warnings List"));
	connect(m_manualRpctAppendixAAction, &QAction::triggered, this, &MainWindow::showRpctUserManualAppendixA);

	m_manualRpctAppendixBAction = new QAction(tr("Appendix B - Build Directory and Output Bitstream File"), this);
	m_manualRpctAppendixBAction->setStatusTip(tr("Show Appendix B - Build Directory and Output Bitstream File"));
	connect(m_manualRpctAppendixBAction, &QAction::triggered, this, &MainWindow::showRpctUserManualAppendixB);

	m_manualAfblAction = new QAction(tr("AFB Library Reference"), this);
	m_manualAfblAction->setStatusTip(tr("Show AFB Library Reference"));
	connect(m_manualAfblAction, &QAction::triggered, this, &MainWindow::showAfblReference);

	m_scriptHelpAction = new QAction(tr("Schema Scripts Reference"), this);
	m_scriptHelpAction->setStatusTip(tr("Show Schema Scripts Reference"));
	connect(m_scriptHelpAction, &QAction::triggered, this, &MainWindow::showScriptHelp);

	m_manualMatsAction = new QAction(tr("MATS User Manual"), this);
	m_manualMatsAction->setStatusTip(tr("Show MATS User Manual"));
	connect(m_manualMatsAction, &QAction::triggered, this, &MainWindow::showMatsUserManual);

	m_manualTuningAction = new QAction(tr("Tuning User Manual"), this);
	m_manualTuningAction->setStatusTip(tr("Show Tuning User Manual"));
	connect(m_manualTuningAction, &QAction::triggered, this, &MainWindow::showTuningUserManual);

	m_shortcutsAction = new QAction(tr("Shortcuts..."), this);
	m_shortcutsAction->setStatusTip(tr("Show shortcuts"));
	connect(m_shortcutsAction, &QAction::triggered, this, &MainWindow::showShortcuts);

	m_settingsAction = new QAction(tr("Settings..."), this);
	m_settingsAction->setStatusTip(tr("Change application settings"));
	m_settingsAction->setEnabled(true);
	connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_subsystemListEditorAction = new QAction(tr("Subsystems..."), this);
	m_subsystemListEditorAction->setStatusTip(tr("Run Subsystem List Editor"));
	m_subsystemListEditorAction->setEnabled(false);
	connect(m_subsystemListEditorAction, &QAction::triggered, this, &MainWindow::runSubsystemListEditor);

	m_connectionsEditorAction = new QAction(tr("Connections..."), this);
	m_connectionsEditorAction->setStatusTip(tr("Run Connections Editor"));
	m_connectionsEditorAction->setEnabled(false);
	connect(m_connectionsEditorAction, &QAction::triggered, this, &MainWindow::runConnectionsEditor);

	m_diagSignalTypesEditorAction = new QAction(tr("Diagnostics Signal Types..."), this);
	m_diagSignalTypesEditorAction->setStatusTip(tr("Run Diagnostics Signal Types Editor"));
	m_diagSignalTypesEditorAction->setEnabled(false);
	connect(m_diagSignalTypesEditorAction, &QAction::triggered, this, &MainWindow::runDiagSignalTypesEditor);

	m_appSignalListsEditorAction = new QAction(tr("Application Signal Lists..."), this);
	m_appSignalListsEditorAction->setStatusTip(tr("Run Application Signal Lists Editor"));
	m_appSignalListsEditorAction->setEnabled(false);
	connect(m_appSignalListsEditorAction, &QAction::triggered, this, &MainWindow::runAppSignalListsEditor);

	m_busEditorAction = new QAction(tr("Bus Types Editor..."), this);
	m_busEditorAction->setStatusTip(tr("Run Bus Types Editor"));
	m_busEditorAction->setEnabled(false);
	connect(m_busEditorAction, &QAction::triggered, this, &MainWindow::runBusEditor);

	m_tagsEditorAction = new QAction(tr("Tags Editor..."), this);
	m_tagsEditorAction->setStatusTip(tr("Run Tags Editor"));
	m_tagsEditorAction->setEnabled(false);
	connect(m_tagsEditorAction, &QAction::triggered, this, &MainWindow::runTagsEditor);

	m_matsUsersEditorAction = new QAction(tr("MATS Users Editor..."), this);
	m_matsUsersEditorAction->setStatusTip(tr("Run MATS Users Editor"));
	m_matsUsersEditorAction->setEnabled(false);
	connect(m_matsUsersEditorAction, &QAction::triggered, this, &MainWindow::runMatsUserEditor);

	m_simProfilesEditorAction = new QAction(tr("Simulator Profiles Editor..."), this);
	m_simProfilesEditorAction->setStatusTip(tr("Run Simulator Profiles Editor"));
	m_simProfilesEditorAction->setEnabled(false);
	connect(m_simProfilesEditorAction, &QAction::triggered, this, &MainWindow::runSimulationProfilesEditor);

	m_updateUfbsAfbs = new QAction(tr("Update AFBs/UFBs/Busses..."), this);
	m_updateUfbsAfbs->setStatusTip(tr("Update AFBs/UFBs/Busses on all schemas"));
	m_updateUfbsAfbs->setEnabled(false);
	connect(m_updateUfbsAfbs, &QAction::triggered, this, &MainWindow::updateUfbsAfbsBusses);

	m_AfbLibraryCheck = new QAction(tr("AFB Library Check..."), this);
	m_AfbLibraryCheck->setStatusTip(tr("AFB Library Check"));
	m_AfbLibraryCheck->setEnabled(false);
	connect(m_AfbLibraryCheck, &QAction::triggered, this, &MainWindow::afbLibraryCheck);

	m_aboutQtAction = new QAction(tr("About Qt..."), this);
	m_aboutQtAction->setStatusTip(tr("Show Qt information"));
	// m_pAboutAction->setEnabled(true);
	connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQt);

	m_aboutAction = new QAction(tr("About u7..."), this);
	m_aboutAction->setStatusTip(tr("Show application information"));
	// m_pAboutAction->setEnabled(true);
	connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_debugAction = new QAction(tr("Debug Mode"), this);
	m_debugAction->setStatusTip(tr("Set debug mode, some extra messages will be displayed"));
	m_debugAction->setEnabled(true);
	m_debugAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_D));
	connect(m_debugAction, &QAction::triggered, this, &MainWindow::debug);
	addAction(m_debugAction);

	m_startBuildAction = new QAction(tr("Build Project"), this);
	m_startBuildAction->setStatusTip(tr("Build opened project"));
	m_startBuildAction->setEnabled(false);
	QList<QKeySequence> bks;
	bks << QKeySequence(Qt::CTRL | Qt::Key_B);
	bks << QKeySequence(Qt::Key_F7);
	m_startBuildAction->setShortcuts(bks);
	connect(m_startBuildAction, &QAction::triggered, this, &MainWindow::startBuild);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::buildStarted,
			this,
			[this]()
			{
				m_startBuildAction->setEnabled(false);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::buildFinished,
			this,
			[this]()
			{
				m_startBuildAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_startBuildAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_startBuildAction->setEnabled(false);
			});
	addAction(m_startBuildAction);


	m_projectHistoryAction = new QAction(tr("Project History..."), this);
	m_projectHistoryAction->setStatusTip(tr("Show project history"));
	m_projectHistoryAction->setEnabled(false);
	connect(m_projectHistoryAction, &QAction::triggered, this, &MainWindow::projectHistory);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_projectHistoryAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_projectHistoryAction->setEnabled(false);
			});
	addAction(m_projectHistoryAction);

	m_projectPropertiesAction = new QAction(tr("Project Properties..."), this);
	m_projectPropertiesAction->setEnabled(false);
	connect(m_projectPropertiesAction, &QAction::triggered, this, &MainWindow::projectProperties);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_projectPropertiesAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_projectPropertiesAction->setEnabled(false);
			});

	m_projectDifferenceAction = new QAction(tr("Project Diff..."), this);
	m_projectDifferenceAction->setEnabled(false);
	connect(m_projectDifferenceAction, &QAction::triggered, this, &MainWindow::projectDifference);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_projectDifferenceAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_projectDifferenceAction->setEnabled(false);
			});

	m_schemasAlbumAction = new QAction(tr("Create Schemas Albums..."), this);
	m_schemasAlbumAction->setStatusTip(tr("Create PDF albums with all project schemas"));
	m_schemasAlbumAction->setEnabled(false);
	connect(m_schemasAlbumAction, &QAction::triggered, this, &MainWindow::createSchemasAlbums);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_schemasAlbumAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_schemasAlbumAction->setEnabled(false);
			});

	m_pendingChangesAction = new QAction(tr("Pending Changes..."), this);
	m_pendingChangesAction->setEnabled(false);
	connect(m_pendingChangesAction, &QAction::triggered, this, &MainWindow::pendingChanges);
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			this,
			[this]()
			{
				m_pendingChangesAction->setEnabled(true);
			});
	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			this,
			[this]()
			{
				m_pendingChangesAction->setEnabled(false);
			});

	// Locator
	//
	m_locatorAction = new QAction(tr("Locator..."), this);
	m_locatorAction->setEnabled(false);
	m_locatorAction->setEnabled(true);
	m_locatorAction->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_K});
	m_locatorAction->setShortcutContext(Qt::ApplicationShortcut);

	connect(m_locatorAction,
			&QAction::triggered,
			[this]()
			{
				m_locatorEditControl->setFocus();
			});

	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectOpened,
			[this]()
			{
				m_locatorAction->setEnabled(true);
				m_locatorEditControl->setEnabled(true);
			});

	connect(&GlobalMessanger::instance(),
			&GlobalMessanger::projectClosed,
			[this]()
			{
				m_locatorAction->setEnabled(false);
				m_locatorEditControl->setEnabled(false);
				m_locatorEditControl->clear();
			});

	addAction(m_locatorAction);

	// --
	//

	return;
}

void MainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_exitAction);

	// Administration
	//
	QMenu* pAdmMenu = menuBar()->addMenu(tr("&Administration"));

	pAdmMenu->addAction(m_usersAction);

	// Project
	//
	QMenu* pProjectMenu = menuBar()->addMenu(tr("Project")); // Alt+P now switching to the Projects tab page, don't use &
	pProjectMenu->addAction(m_projectHistoryAction);
	pProjectMenu->addAction(m_pendingChangesAction);
	pProjectMenu->addAction(m_projectDifferenceAction);
	pProjectMenu->addSeparator();
	pProjectMenu->addAction(m_schemasAlbumAction);
	pProjectMenu->addAction(m_startBuildAction);
	pProjectMenu->addSeparator();
	pProjectMenu->addAction(m_projectPropertiesAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_subsystemListEditorAction);
	pToolsMenu->addAction(m_connectionsEditorAction);
	pToolsMenu->addAction(m_diagSignalTypesEditorAction);
	pToolsMenu->addAction(m_appSignalListsEditorAction);
	pToolsMenu->addSeparator();

	pToolsMenu->addAction(m_busEditorAction);
	pToolsMenu->addSeparator();

	pToolsMenu->addAction(m_tagsEditorAction);
	pToolsMenu->addAction(m_matsUsersEditorAction);
	pToolsMenu->addAction(m_simProfilesEditorAction);
	pToolsMenu->addSeparator();

	pToolsMenu->addAction(m_updateUfbsAfbs);

	if (theAppSettings.isExpertMode() == true)
	{
		pToolsMenu->addAction(m_AfbLibraryCheck);
	}

	pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_settingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_manualAfblAction);
	pHelpMenu->addAction(m_manualRpctAction);

	QMenu* pRpctAppendixesMenu = pHelpMenu->addMenu(tr("RPCT User Manual Appendixes"));
	pRpctAppendixesMenu->addAction(m_manualRpctAppendixAAction);
	pRpctAppendixesMenu->addAction(m_manualRpctAppendixBAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_installRpctAction);
	pHelpMenu->addAction(m_rpctQuickStartAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_scriptHelpAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_manualMatsAction);
	pHelpMenu->addAction(m_manualTuningAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_shortcutsAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_aboutQtAction);
	pHelpMenu->addAction(m_aboutAction);

	return;
}

void MainWindow::createToolBars() {}

void MainWindow::createStatusBar()
{
	m_locatorEditControl = new Locator::LocatorEditControl{m_locator, m_locatorListWidget, this};
	m_locatorEditControl->setEnabled(false);

	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarSchemaLayerLabel = new UiLib::ClickableLabel("Layer: ");
	m_statusBarSchemaLayerLabel->setAlignment(Qt::AlignHCenter);
	m_statusBarSchemaLayerLabel->setMinimumWidth(100);
	m_statusBarSchemaLayerLabel->setVisible(false);
	m_statusBarSchemaLayerLabel->setCursor(Qt::PointingHandCursor);

	m_statusBarSchemaZoomLabel = new UiLib::ClickableLabel("Zoom: ");
	m_statusBarSchemaZoomLabel->setAlignment(Qt::AlignHCenter);
	m_statusBarSchemaZoomLabel->setMinimumWidth(80);
	m_statusBarSchemaZoomLabel->setVisible(false);
	m_statusBarSchemaZoomLabel->setCursor(Qt::PointingHandCursor);

	m_statusBarConnectionState = new QLabel();
	m_statusBarConnectionState->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionState->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_locatorEditControl, 2);
	statusBar()->addWidget(m_statusBarInfo, 7);
	statusBar()->addPermanentWidget(m_statusBarSchemaLayerLabel, 0);
	statusBar()->addPermanentWidget(m_statusBarSchemaZoomLabel, 0);
	statusBar()->addPermanentWidget(m_statusBarConnectionState, 0);

	return;
}

CentralWidget* MainWindow::getCentralWidget()
{
	CentralWidget* pCentralWidget = dynamic_cast<CentralWidget*>(QMainWindow::centralWidget());
	assert(pCentralWidget != nullptr);
	return pCentralWidget;
}

void MainWindow::onMiniDumpCreated(QString dumpFilePath, bool result)
{
	QString s;

	if (result == false)
	{
		s = QObject::tr("Application has been crashed!\nColld not save crash dump file:\n%1.").arg(dumpFilePath);
	}
	else
	{
		s = QObject::tr("Application has been crashed!\nA crash dump has been created:\n%1\nPlease send this file and program execulable "
						"file to support.")
				.arg(dumpFilePath);
	}

	QMessageBox::critical(this, qAppName(), s);
}

void MainWindow::currentTabChanged(int /*tabIndex*/)
{
	QWidget* currentTabWidget = getCentralWidget()->currentWidget();

	m_statusBarSchemaLayerLabel->setVisible(currentTabWidget == m_schemaTabPage);
	m_statusBarSchemaZoomLabel->setVisible(currentTabWidget == m_schemaTabPage);

	return;
}

void MainWindow::exit()
{
	qApp->closeAllWindows();
}

void MainWindow::userManagement()
{
	UserManagementDialog d(this, dbController());

	if (d.exec() == QDialog::Accepted) {}

	return;
}

void MainWindow::showSettings()
{
	DialogSettings d(this);
	d.setSettings(theAppSettings);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		theAppSettings = d.settings();
		theAppSettings.save();

		dbController()->setHost(theAppSettings.serverHost());
		dbController()->setPort(theAppSettings.serverPort());
		dbController()->setServerUsername(theAppSettings.serverUsername());
		dbController()->setServerPassword(theAppSettings.serverPassword());

		return;
	}

	return;
}

void MainWindow::showShortcuts()
{
	if (m_dialogShortcuts == nullptr)
	{
		m_dialogShortcuts = new DialogShortcuts(this);
		m_dialogShortcuts->show();

		auto f = [this]() -> void
		{
			m_dialogShortcuts = nullptr;
		};

		connect(m_dialogShortcuts, &DialogShortcuts::dialogClosed, this, f);
	}
	else
	{
		m_dialogShortcuts->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogShortcuts);
}

void MainWindow::showRpctUserManual()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/D11.6_FSC_ RPCT_User_Manual.pdf", this);
}

void MainWindow::showRpctInstallManual()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/Installing and configuring RPCT.pdf", this);
}

void MainWindow::showRpctQuickStart()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/RPCT Quick Start Guide.pdf", this);
}

void MainWindow::showRpctUserManualAppendixA()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/Appendixes/D11.6 RPCT User Manual Appendix A Warnings and Errors List.pdf",
					 this);
}

void MainWindow::showRpctUserManualAppendixB()
{
	UiTools::openPdf(QApplication::applicationDirPath() +
						 "/docs/Appendixes/D11.6 RPCT User Manual Appendix B Build Directory and Output Bitstream File Description.pdf",
					 this);
}

void MainWindow::showAfblReference()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/D11.5_AFBL_RM.pdf", this);
}

void MainWindow::showScriptHelp()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/scripthelp/index.html", this);
}

void MainWindow::showMatsUserManual()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/D11.8_RPCT_MATS_User_Manual.pdf", this);
}

void MainWindow::showTuningUserManual()
{
	UiTools::openPdf(QApplication::applicationDirPath() + "/docs/D11.9_FSC_Tuning_User_Manual.pdf", this);
}


void MainWindow::runConfigurator() {}

void MainWindow::runSubsystemListEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogSubsystemListEditor d(dbController(), this);
	d.exec();
}

void MainWindow::runConnectionsEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	if (theDialogConnections == nullptr)
	{
		theDialogConnections = new DialogConnections(dbController(), this);
		theDialogConnections->show();
	}
	else
	{
		theDialogConnections->activateWindow();
	}
	UiTools::adjustDialogPlacement(theDialogConnections);
}

void MainWindow::runDiagSignalTypesEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogDiagSignalTypes::showDialog(dbController(), this);

	return;
}

void MainWindow::runAppSignalListsEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogAppSignalLists::showDialog(dbController(), this);

	return;
}

void MainWindow::runBusEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	if (theDialogBusEditor == nullptr)
	{
		theDialogBusEditor = new DialogBusEditor(dbController(), this);
		theDialogBusEditor->show();
	}
	else
	{
		theDialogBusEditor->activateWindow();
	}
	UiTools::adjustDialogPlacement(theDialogBusEditor);
}

void MainWindow::runTagsEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogTagsEditor d(dbController(), this);
	d.exec();
}

void MainWindow::runSimulationProfilesEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	SimProfileEditor::run(dbController(), this);
}

void MainWindow::runMatsUserEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogMatsUsersEditor d(dbController(), this);
	d.exec();
}

void MainWindow::updateUfbsAfbsBusses()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("Update schemas AFBs/UFBs/Actuators/Busses."));
	mb.setInformativeText(tr("To prevent data loss all schemas files must be checked in."));
	mb.setIcon(QMessageBox::NoIcon);
	QPushButton* updateButton = mb.addButton(tr("Update"), QMessageBox::ActionRole);
	mb.addButton(QMessageBox::Cancel);

	mb.exec();

	if (mb.clickedButton() != updateButton)
	{
		return;
	}

	GlobalMessanger::instance().fireChangeCurrentTab(m_schemaTabPage);

	// Get all files we will need.
	//
	auto getFilesFunc = [this](DbDir dir, const QString& extension) -> std::vector<DbFileInfo>
	{
		DbFileTree filesTree;
		db()->getFileListTree(&filesTree, dir, "%", true, this);

		std::vector<DbFileInfo> schemaFileInfos = filesTree.toVectorIf(
			[extension](const DbFileInfo& file)
			{
				return file.fileName().endsWith(QLatin1String(".") + extension, Qt::CaseInsensitive) == true && file.isFolder() == false;
			});

		return schemaFileInfos;
	};

	auto ufbSchemaFileInfos = getFilesFunc(DbDir::UfblDir, File::UfbFileExtension);
	auto alSchemaFileInfos = getFilesFunc(DbDir::AppLogicDir, File::AlFileExtension);
	auto actuatorSchemaFileInfos = getFilesFunc(DbDir::ActuatorsDir, File::ActuatorFileExtension);
	auto actuatorHeadersFileInfos = getFilesFunc(DbDir::ActuatorsDir, File::ActuatorHeaderFileExtension);

	int totalSchemas = static_cast<int>(ufbSchemaFileInfos.size() + alSchemaFileInfos.size() + actuatorSchemaFileInfos.size());

	// Check checked out schemas
	//
	auto allSchemaFiles =
		std::array{std::views::all(ufbSchemaFileInfos), std::views::all(alSchemaFileInfos), std::views::all(actuatorSchemaFileInfos)} |
		std::views::join;

	QStringList checkedOutFiles;
	for (const auto& fi : allSchemaFiles)
	{
		if (fi.state() == E::VcsState::CheckedOut)
		{
			checkedOutFiles.push_back(fi.fileName());
		}
	}

	if (checkedOutFiles.empty() == false)
	{
		QMessageBox mbError(this);

		mbError.setIcon(QMessageBox::Critical);
		mbError.setText(tr("Update AFBs/UFBs/Actuators/Busses error."));
		mbError.setInformativeText(
			"There are some checked out Application Logic, Actuators and/or UFB schemas. CheckIn these files and repeat operation.");
		mbError.setDetailedText(checkedOutFiles.join(QChar::LineSeparator));

		mbError.exec();
		return;
	}

	// Get all busses
	//
	std::vector<AppSignalLib::Bus> busses;
	bool gettingBussesOk = F2KeyForSchemaItem::loadBusses(dbController(), &busses, this);
	if (gettingBussesOk == false)
	{
		return;
	}

	// Get all ActuatorHeaders
	//
	std::vector<std::shared_ptr<VFrame30::ActuatorHeader>> actuatorHeaders;

	if (actuatorHeadersFileInfos.empty() == false)
	{
		std::vector<std::shared_ptr<DbFile>> files;
		actuatorHeaders.reserve(actuatorHeadersFileInfos.size());

		bool getFilesOk = db()->getLatestVersion(actuatorHeadersFileInfos, &files, this);
		if (getFilesOk == false)
		{
			return;
		}

		for (std::shared_ptr<DbFile>& file : files)
		{
			auto actuatorHeader = VFrame30::ActuatorHeader::Create(file->data());
			if (actuatorHeader == nullptr)
			{
				QMessageBox::critical(this, qAppName(), tr("Error parsing Actuator Header %1.").arg(file->fileName()));
				return;
			}

			actuatorHeaders.push_back(actuatorHeader);
		}
	}

	// Update UFB schemas, updating AFBs, Busses and Actuators
	//
	struct UpdateItemParams
	{
		bool updateAfbItems = true;
		bool updateUfbItems = true;
		bool updateActuatorItems = true;
		bool updateBusItems = true;
		std::vector<DbFileInfo>& fileInfos;

		std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbSchemas;
		std::vector<AppSignalLib::Bus>& busses;
		std::vector<std::shared_ptr<VFrame30::ActuatorHeader>>& actuatorHeaders;
	};

	QProgressDialog progress{"Updating AFBs/UFBs/Busses/Actuators on schemas...", "Abort", 0, totalSchemas, this};
	progress.setWindowModality(Qt::WindowModal);

	int progressIndicator = 0;
	QStringList updateDetails;

	struct
	{
		int updatedAfbItems = 0;
		int updatedUfbItems = 0;
		int updatedBusItems = 0;
		int updatedActuatorItems = 0;

		int sum() const { return updatedAfbItems + updatedUfbItems + updatedBusItems + updatedActuatorItems; }
	} totals;

	LogicModuleSet logicModuleSet;

	auto updateSchemaItemFunc =
		[&logicModuleSet, this, &totals, &progress, &progressIndicator, &updateDetails](const UpdateItemParams& params,
																						QString updateCaption)
	{
		progress.setLabelText(updateCaption);

		for (DbFileInfo& fi : params.fileInfos)
		{
			progress.setValue(progressIndicator++);
			if (progress.wasCanceled() == true)
			{
				break;
			}

			std::shared_ptr<DbFile> file;
			bool getFileOk = db()->getLatestVersion(fi, &file, this);
			if (getFileOk == false || file == nullptr)
			{
				break;
			}

			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());
			if (schema == nullptr)
			{
				QMessageBox::critical(this, qAppName(), tr("Error parsing schema %1.").arg(file->fileName()));
				break;
			}

			if (schema->isUfbSchema() == false && schema->isLogicSchema() == false && schema->isActuatorSchema() == false)
			{
				assert(schema->isUfbSchema() == true || schema->isLogicSchema() == true || schema->isActuatorSchema() == true);
				QMessageBox::critical(this, qAppName(), tr("File %1 must be AppLogic, UFB, or Actuator schema.").arg(file->fileName()));
				break;
			}

			QString lmDescriptionFile;
			auto lmp = schema->propertyByCaption(VFrame30::PropertyNames::lmDescriptionFile);
			if (lmp != nullptr)
			{
				lmDescriptionFile = lmp->value().toString();
			}

			if (logicModuleSet.has(lmDescriptionFile) == false)
			{
				QString errorMessage;
				bool fileIsLoaded = logicModuleSet.loadFile(db(), lmDescriptionFile, &errorMessage);
				if (fileIsLoaded == false)
				{
					QMessageBox::critical(this, qAppName(), errorMessage);
					break;
				}
			}

			std::shared_ptr<LmDescription> logicModuleDescription = logicModuleSet.get(lmDescriptionFile);
			if (logicModuleDescription == nullptr)
			{
				assert(logicModuleDescription);
				break;
			}

			// Update AFBs  on schemas
			//
			int thisSchemaUpdatedCount = 0;

			if (params.updateAfbItems == true)
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				bool ok = schema->updateAllSchemaItemFbs(logicModuleDescription->afbElements(), &updatedCount, &updateErrorMessage);
				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totals.updatedAfbItems += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// Update Busses on schemas
			//
			if (params.updateBusItems == true)
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				bool ok = schema->updateAllSchemaItemBusses(params.busses, &updatedCount, &updateErrorMessage);
				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totals.updatedBusItems += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// Update UFBs on schemas
			//
			if (params.updateUfbItems == true)
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				bool ok = schema->updateAllSchemaItemUfb(params.ufbSchemas, &updatedCount, &updateErrorMessage);
				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totals.updatedUfbItems += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// Update ActuatorItems on schemas
			//
			if (params.updateActuatorItems == true)
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				bool ok = schema->updateAllSchemaItemActuators(params.actuatorHeaders, &updatedCount, &updateErrorMessage);
				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totals.updatedActuatorItems += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// CheckOut and save file if changes are made
			//
			if (thisSchemaUpdatedCount > 0)
			{
				bool ok = db()->checkOut(fi, this);
				if (ok == false || fi.state() != E::VcsState::CheckedOut)
				{
					break;
				}

				// Set workcopy
				//
				QByteArray savedData;
				ok = schema->saveToByteArray(&savedData);

				file->swapData(savedData);

				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), tr("Saving %1 error.").arg(file->fileName()));
					break;
				}

				ok = db()->setWorkcopy(file, this);

				if (ok == false)
				{
					break;
				}
			}

			QString schemaType;
			if (schema->isUfbSchema() == true)
			{
				schemaType = "UfbSchema";
			}
			if (schema->isLogicSchema() == true)
			{
				schemaType = "AppLogicSchema";
			}
			if (schema->isActuatorSchema() == true)
			{
				schemaType = "ActuatorSchema";
			}

			updateDetails << QString{"%1: %2, updated %3 item(s)"}.arg(schemaType).arg(schema->schemaId()).arg(thisSchemaUpdatedCount);
		}

		return;
	};

	// Update UFB schemas first, it'll bump UFB versions, so we will need to update SchemaItemUfbs on AppLogic schemas after that.
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbSchemas;

	{
		UpdateItemParams params{.updateAfbItems = true,
								.updateUfbItems = false,
								.updateActuatorItems = true,
								.updateBusItems = true,
								.fileInfos = ufbSchemaFileInfos,
								.ufbSchemas = ufbSchemas,
								.busses = busses,
								.actuatorHeaders = actuatorHeaders};

		updateSchemaItemFunc(params, "Updating UFB schemas");

		std::vector<std::shared_ptr<DbFile>> files;
		ufbSchemas.reserve(ufbSchemaFileInfos.size());

		bool getFilesOk = db()->getLatestVersion(ufbSchemaFileInfos, &files, this);
		if (getFilesOk == false)
		{
			return;
		}

		for (std::shared_ptr<DbFile>& file : files)
		{
			auto ufbSchema = std::dynamic_pointer_cast<VFrame30::UfbSchema>(VFrame30::UfbSchema::Create(file->data()));
			if (ufbSchema == nullptr)
			{
				QMessageBox::critical(this, qAppName(), tr("Error parsing UFB schema %1.").arg(file->fileName()));
				return;
			}

			ufbSchemas.push_back(ufbSchema);
		}
	}

	// Update AppLogic schemas
	//
	{
		UpdateItemParams params{.updateAfbItems = true,
								.updateUfbItems = true,
								.updateActuatorItems = true,
								.updateBusItems = true,
								.fileInfos = alSchemaFileInfos,
								.ufbSchemas = ufbSchemas,
								.busses = busses,
								.actuatorHeaders = actuatorHeaders};

		updateSchemaItemFunc(params, "Updating AppLogic schemas");
	}

	// Update Actuator schemas
	//
	{
		UpdateItemParams params{.updateAfbItems = true,
								.updateUfbItems = true,
								.updateActuatorItems = true,
								.updateBusItems = true,
								.fileInfos = actuatorSchemaFileInfos,
								.ufbSchemas = ufbSchemas,
								.busses = busses,
								.actuatorHeaders = actuatorHeaders};

		updateSchemaItemFunc(params, "Updating Actuator schemas");
	}

	// --
	//
	if (progressIndicator != totalSchemas)
	{
		updateDetails << tr("...");
		updateDetails << tr("Operation is aborted");
	}
	else
	{
		updateDetails << QString("Done");
	}

	progress.setValue(progress.maximum());

	// Show result, refresh files list
	//
	if (totals.sum() > 0)
	{
		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("Updated %1 schema item(s).").arg(totals.sum()));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nCheckIn schemas to accept changes, Undo to reject.");
		msgBox.setDetailedText(updateDetails.join(QChar::LineSeparator));
		msgBox.exec();
	}
	else
	{
		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("No schema items were updated."));
		msgBox.setInformativeText("All schemas are up to date.");
		msgBox.exec();
	}

	// Refresh view
	//
	if (m_schemaTabPage != nullptr)
	{
		m_schemaTabPage->refreshControlTabPage();
	}

	return;
}

void MainWindow::afbLibraryCheck()
{
	if (theDialogAfbLibraryCheck == nullptr)
	{
		theDialogAfbLibraryCheck = new DialogAfbLibraryCheck(dbController(), this);
		theDialogAfbLibraryCheck->show();
	}
	else
	{
		theDialogAfbLibraryCheck->activateWindow();
	}
	UiTools::adjustDialogPlacement(theDialogAfbLibraryCheck);
}

void MainWindow::showAbout()
{
	QString text = "Supported project database version: " + QString::number(DbController::databaseVersion()) + "<br><br>";
	text += qApp->applicationName() + " provides offline tools for FSC chassis configuration, application logic design and its "
									  "compilation, visualization design and MATS software configuration.";

	LicenseLib::AppLicenser appLicenser;
	UiLib::DialogAbout::show(this,
							 text,
							 ":/Logo/RadiyLogo.png",
							 appLicenser.organization(),
							 appLicenser.person(),
							 appLicenser.endDate(),
							 appLicenser.uuid(),
							 LicenseLib::AppLicenser::workplaceId());

	return;
}

void MainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void MainWindow::debug()
{
	theSettings.setDebugMode(!theSettings.isDebugMode());
	qDebug() << "DebugMode: " << theSettings.isDebugMode();
}

void MainWindow::startBuild()
{
	qDebug() << "MainWindow::startBuild";

	if (m_buildTabPage == nullptr)
	{
		assert(m_buildTabPage);
		return;
	}

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	getCentralWidget()->switchToTabPage(m_buildTabPage);

	if (m_buildTabPage->isBuildRunning() == false)
	{
		m_buildTabPage->build();
	}

	return;
}

void MainWindow::projectHistory()
{
	if (m_dbController == nullptr)
	{
		assert(m_dbController);
		return;
	}

	if (m_dbController->isProjectOpened() == false)
	{
		return;
	}

	std::vector<DbChangeset> history;

	bool ok = m_dbController->getProjectHistory(&history, this);
	if (ok == false)
	{
		return;
	}

	FileHistoryDialog::showHistory(m_dbController, db()->currentProject().projectName(), history, this);

	return;
}

void MainWindow::projectProperties()
{
	if (m_dbController == nullptr)
	{
		assert(m_dbController);
		return;
	}

	if (m_dbController->isProjectOpened() == false)
	{
		return;
	}

	ProjectPropertiesForm::show(this, m_dbController);

	return;
}

void MainWindow::projectDifference()
{
	if (m_dbController == nullptr)
	{
		assert(m_dbController);
		return;
	}

	if (m_dbController->isProjectOpened() == false)
	{
		return;
	}


	DialogProjectDiff dialog(db(), this);

	if (dialog.exec() == QDialog::Accepted)
	{
		ProjectDiffGeneratorThread::run(dialog.fileName(),
										dialog.reportParams(),
										db()->currentProject().projectName(),
										db()->currentUser().username(),
										db()->currentUser().password(),
										&m_signalSetProvider->signalSet(),
										this);
	}

	return;
}

void MainWindow::createSchemasAlbums()
{
	SchemasAlbumGenerator::createSchemasAlbums(db(), &m_signalSetProvider->signalSet(), this);
	return;
}

void MainWindow::pendingChanges()
{
	PendingChangesDialog::show(db(), this);
	return;
}

void MainWindow::projectOpened(DbProject project)
{
	QString title = QString("%1 - %2 (Version %3) - %4")
						.arg(qApp->applicationName())
						.arg(project.projectName())
						.arg(project.version())
						.arg(dbController()->currentUser().username());

	setWindowTitle(title);

	// Action, disable/enable
	//
	assert(m_usersAction != nullptr);

	m_usersAction->setEnabled(true);
	m_subsystemListEditorAction->setEnabled(true);
	m_connectionsEditorAction->setEnabled(true);
	m_diagSignalTypesEditorAction->setEnabled(true);
	m_appSignalListsEditorAction->setEnabled(true);
	m_busEditorAction->setEnabled(true);
	m_tagsEditorAction->setEnabled(true);
	m_matsUsersEditorAction->setEnabled(true);
	m_simProfilesEditorAction->setEnabled(true);
	m_updateUfbsAfbs->setEnabled(true);
	m_AfbLibraryCheck->setEnabled(true);

	// Status bar
	//
	assert(m_statusBarConnectionState != nullptr);

	m_statusBarConnectionState->setText(tr("Opened: %1:%2").arg(theAppSettings.serverHost()).arg(theAppSettings.serverPort()));

	// Show and hide FilesTabPage
	//
	if (db()->currentUser().isAdministrator() == true)
	{
		getCentralWidget()->insertTab(m_filesTabPageIndex, m_filesTabPage, m_filesTabPage->windowTitle());
	}

	// Update Project Defaults
	//
	ProjectDefaults::instance().update(*db(), this);

	return;
}

void MainWindow::projectClosed()
{
	setWindowTitle(qApp->applicationName());

	// Actions, disable/enable
	//
	assert(m_usersAction != nullptr);

	m_usersAction->setEnabled(false);
	m_subsystemListEditorAction->setEnabled(false);
	m_connectionsEditorAction->setEnabled(false);
	m_diagSignalTypesEditorAction->setEnabled(false);
	m_appSignalListsEditorAction->setEnabled(false);
	m_busEditorAction->setEnabled(false);
	m_tagsEditorAction->setEnabled(false);
	m_matsUsersEditorAction->setEnabled(false);
	m_simProfilesEditorAction->setEnabled(false);
	m_updateUfbsAfbs->setEnabled(false);
	m_AfbLibraryCheck->setEnabled(false);

	// Status bar
	//
	assert(m_statusBarConnectionState != nullptr);

	m_statusBarConnectionState->setText(tr("Closed"));

	// Remove FilesTabPage, it will be added again in projectOpened slot if user is an admin
	//
	if (getCentralWidget()->tabText(m_filesTabPageIndex) == m_filesTabPage->windowTitle())
	{
		getCentralWidget()->removeTab(m_filesTabPageIndex);
	}

	// Update Project Defaults
	//
	{
		ProjectDefaults& pd = ProjectDefaults::instance();
		pd.parse(QString{});
	}

	return;
}

void MainWindow::buildStarted()
{
	// #ifdef Q_OS_WINDOWS
	//	Q_ASSERT(m_taskBarButton);

	//	m_taskBarButton->progress()->setRange(0, 100);
	//	m_taskBarButton->progress()->show();

	//	m_timerId = startTimer(50);
	// #endif
}

void MainWindow::buildFinished(int /*errorCount*/)
{
	// #ifdef Q_OS_WINDOWS
	//	m_taskBarButton->progress()->hide();
	//	killTimer(m_timerId);
	// #endif
}


DbController* MainWindow::dbController()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

DbController* MainWindow::db()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}
