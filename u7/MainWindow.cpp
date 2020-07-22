#include "Stable.h"
#include "MainWindow.h"
#include "CentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "../lib/DbController.h"
#include "UserManagementDialog.h"
#include "ProjectsTabPage.h"
#include "FilesTabPage.h"
#include "SchemaTabPageEx.h"
#include "EquipmentTabPage.h"
#include "SignalsTabPage.h"
#include "DialogSubsystemListEditor.h"
#include "DialogConnections.h"
#include "DialogBusEditor.h"
#include "DialogAfbLibraryCheck.h"
#include "BuildTabPage.h"
#include "UploadTabPage.h"
#include "SimulatorTabPage.h"
#include "TestsTabPage.h"
#include "GlobalMessanger.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/ProjectPropertiesForm.h"
#include "Forms/PendingChangesDialog.h"
#include "../lib/Ui/DialogAbout.h"
#include "../VFrame30/VFrame30.h"
#include "../lib/LogicModuleSet.h"
#include "DialogShortcuts.h"
#include "../lib/Ui/UiTools.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

MainWindow::MainWindow(DbController* dbcontroller, QWidget* parent) :
	QMainWindow(parent),
	m_dbController(dbcontroller)
{
	assert(m_dbController);

	// --
	//
	setCentralWidget(new CentralWidget());

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

	// Add main tab pages
	//
	m_projectsTab = new ProjectsTabPage(dbController(), nullptr);
	m_equipmentTab = new EquipmentTabPage(dbController(), nullptr);
	m_signalsTab = new SignalsTabPage(dbController(), nullptr);

	m_filesTabPage = new FilesTabPage(dbController(), nullptr);
	m_filesTabPage->setWindowTitle(tr("Files"));

	getCentralWidget()->addTabPage(m_projectsTab, tr("Projects"));
	getCentralWidget()->addTabPage(m_equipmentTab, tr("Equipment"));
	getCentralWidget()->addTabPage(m_signalsTab, tr("Application Signals"));

	connect(getCentralWidget(), &QTabWidget::currentChanged, m_signalsTab, &SignalsTabPage::onTabPageChanged);

	m_filesTabPageIndex = getCentralWidget()->addTabPage(m_filesTabPage, m_filesTabPage->windowTitle());
	getCentralWidget()->removeTab(m_filesTabPageIndex);	// It will be added in projectOpened slot if required

	m_editSchemaTabPage = new SchemasTabPageEx{db(), this};
	getCentralWidget()->addTabPage(m_editSchemaTabPage, tr("Schemas"));

	m_buildTabPage = new BuildTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_buildTabPage, tr("Build"));

	m_uploadTabPage = new UploadTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_uploadTabPage, tr("Upload"));

	m_simulatorTabPage = new SimulatorTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_simulatorTabPage, tr("Simulator"));

	m_testsTabPage = new TestsTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_testsTabPage, tr("Tests"));

	// --
	//
	setMinimumSize(500, 300);
	restoreWindowState();

	centralWidget()->show();

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
	if (m_buildTabPage != nullptr)
	{
		m_buildTabPage->cancelBuild();
	}
	else
	{
		assert(m_buildTabPage);
	}

	// check if any schema or test is not saved
	//
	if (m_editSchemaTabPage == nullptr || m_testsTabPage == nullptr)
	{
		assert(m_editSchemaTabPage);
		assert(m_testsTabPage);
		e->accept();
		return;
	}

	if (m_editSchemaTabPage->hasUnsavedSchemas() == true || m_testsTabPage->hasUnsavedTests() == true)
	{
		QMessageBox::StandardButton result = QMessageBox::question(this, QApplication::applicationName(),
																   tr("Some items on Schemas and Tests tab pages have unsaved changes."),
																   QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel,
																   QMessageBox::SaveAll);

		if (result == QMessageBox::Cancel)
		{
			e->ignore();
			return;
		}

		if (result == QMessageBox::SaveAll)
		{
			m_editSchemaTabPage->saveUnsavedSchemas();	// It will reset modified flag
			m_testsTabPage->saveUnsavedTests();	// It will reset modified flag
		}

		if (result == QMessageBox::Discard)
		{
			m_editSchemaTabPage->resetModified();		// Reset modidied flag for all opened files, so on closeEvent for tese files
														// prompt to save them will not be shown
			m_testsTabPage->resetModified();
		}
	}

	// save windows state and accept event to close app
	//
	saveWindowState();

	e->accept();

	qApp->closeAllWindows();

	return;
}

void MainWindow::showEvent(QShowEvent*)
{
	// Ensure widget is visible
	//
	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7),
			   static_cast<int>(screenRect.height() * 0.7));
	}

	return;
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

	m_logAction = new QAction(tr("Log..."), this);
	m_logAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
	connect(m_logAction, &QAction::triggered, this, &MainWindow::showLog);



	m_manualRpctAction = new QAction(tr("RPCT User Manual"), this);
	m_manualRpctAction->setStatusTip(tr("Show RPCT User Manual"));
	connect(m_manualRpctAction, &QAction::triggered, this, &MainWindow::showRpctUserManual);

	m_manualRpctAppendixAAction = new QAction(tr("RPCT Errors and Warnings"), this);
	m_manualRpctAppendixAAction->setStatusTip(tr("Show RPCT Errors and Warnings"));
	connect(m_manualRpctAppendixAAction, &QAction::triggered, this, &MainWindow::showRpctUserManualAppendixA);

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

	m_busEditorAction = new QAction(tr("Bus Types Editor..."), this);
	m_busEditorAction->setStatusTip(tr("Run Bus Types Editor"));
	m_busEditorAction->setEnabled(false);
	connect(m_busEditorAction, &QAction::triggered, this, &MainWindow::runBusEditor);

	m_updateUfbsAfbs = new QAction(tr("Update AFBs/UFBs/Busses..."), this);
	m_updateUfbsAfbs->setStatusTip(tr("Update AFBs/UFBs/Busses on all schemas"));
	m_updateUfbsAfbs->setEnabled(false);
	connect(m_updateUfbsAfbs, &QAction::triggered, this, &MainWindow::updateUfbsAfbsBusses);

	m_AfbLibraryCheck = new QAction(tr("AFB Library Check..."), this);
	m_AfbLibraryCheck->setStatusTip(tr("AFB Library Check"));
	m_AfbLibraryCheck->setEnabled(false);
	connect(m_AfbLibraryCheck, &QAction::triggered, this, &MainWindow::afbLibraryCheck);

	m_aboutAction = new QAction(tr("About..."), this);
	m_aboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_debugAction = new QAction(tr("Debug Mode"), this);
	m_debugAction->setStatusTip(tr("Set debug mode, some extra messages will be displayed"));
	m_debugAction->setEnabled(true);
	m_debugAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_D));
	connect(m_debugAction, &QAction::triggered, this, &MainWindow::debug);
	addAction(m_debugAction);

	m_startBuildAction = new QAction(tr("Build Project"), this);
	m_startBuildAction->setStatusTip(tr("Build opened project"));
	m_startBuildAction->setEnabled(false);
	QList<QKeySequence> bks;
	bks << QKeySequence(Qt::CTRL + Qt::Key_B);
	bks << QKeySequence(Qt::Key_F7);
	m_startBuildAction->setShortcuts(bks);
	connect(m_startBuildAction, &QAction::triggered, this, &MainWindow::startBuild);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, [this](){m_startBuildAction->setEnabled(false);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::buildFinished, this, [this](){m_startBuildAction->setEnabled(true);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_startBuildAction->setEnabled(true);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_startBuildAction->setEnabled(false);});
	addAction(m_startBuildAction);


	m_projectHistoryAction = new QAction(tr("Project History..."), this);
	m_projectHistoryAction->setStatusTip(tr("Show project history"));
	m_projectHistoryAction->setEnabled(false);
	connect(m_projectHistoryAction, &QAction::triggered, this, &MainWindow::projectHistory);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_projectHistoryAction->setEnabled(true);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_projectHistoryAction->setEnabled(false);});
	addAction(m_projectHistoryAction);

	m_projectPropertiesAction = new QAction(tr("Project Properties..."), this);
	m_projectPropertiesAction->setEnabled(false);
	connect(m_projectPropertiesAction, &QAction::triggered, this, &MainWindow::projectProperties);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_projectPropertiesAction->setEnabled(true);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_projectPropertiesAction->setEnabled(false);});

	m_pendingChangesAction = new QAction(tr("Pending Changes..."), this);
	m_pendingChangesAction->setEnabled(false);
	connect(m_pendingChangesAction, &QAction::triggered, this, &MainWindow::pendingChanges);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_pendingChangesAction->setEnabled(true);});
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_pendingChangesAction->setEnabled(false);});

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
	pAdmMenu->addAction(m_logAction);

	// Project
	//
	QMenu* pProjectMenu = menuBar()->addMenu(tr("Project"));		// Alt+P now switching to the Projects tab page, don't use &
	pProjectMenu->addAction(m_projectHistoryAction);
	pProjectMenu->addAction(m_projectPropertiesAction);
	pProjectMenu->addAction(m_startBuildAction);
	pProjectMenu->addAction(m_pendingChangesAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_subsystemListEditorAction);
	pToolsMenu->addAction(m_connectionsEditorAction);
	pToolsMenu->addAction(m_busEditorAction);

	pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_updateUfbsAfbs);

	if (theSettings.isExpertMode() == true)
	{
		pToolsMenu->addAction(m_AfbLibraryCheck);
	}

	pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_settingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_manualRpctAction);

	QMenu* rpctHelpMenu = pHelpMenu->addMenu(tr("RPCT User Manual Appendixes"));

	rpctHelpMenu->addAction(m_manualRpctAppendixAAction);
	rpctHelpMenu->addAction(m_scriptHelpAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_manualAfblAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_manualMatsAction);
	pHelpMenu->addAction(m_manualTuningAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_shortcutsAction);
	pHelpMenu->addAction(m_aboutAction);

	return;
}

void MainWindow::createToolBars()
{
}

void MainWindow::createStatusBar()
{
	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarConnectionStatistics = new QLabel();
	m_statusBarConnectionStatistics->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionStatistics->setMinimumWidth(100);

	m_statusBarConnectionState = new QLabel();
	m_statusBarConnectionState->setAlignment(Qt::AlignHCenter);
	m_statusBarConnectionState->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConnectionStatistics, 0);
	statusBar()->addPermanentWidget(m_statusBarConnectionState, 0);

	return;
}

CentralWidget* MainWindow::getCentralWidget()
{
	CentralWidget* pCentralWidget = dynamic_cast<CentralWidget*>(QMainWindow::centralWidget());
	assert(pCentralWidget != nullptr);
	return pCentralWidget;
}

void MainWindow::exit()
{
	qApp->closeAllWindows();
}

void MainWindow::userManagement()
{
	UserManagementDialog d(this, dbController());

	if (d.exec() == QDialog::Accepted)
	{
	}

	return;
}

void MainWindow::showLog()
{

}

void MainWindow::showSettings()
{
	DialogSettings d(this);
	d.setSettings(theSettings);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		theSettings = d.settings();
		theSettings.writeSystemScope();

		dbController()->setHost(theSettings.serverIpAddress());
		dbController()->setPort(theSettings.serverPort());
		dbController()->setServerUsername(theSettings.serverUsername());
		dbController()->setServerPassword(theSettings.serverPassword());

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
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.6_RPCT-UM.pdf", this);
}

void MainWindow::showRpctUserManualAppendixA()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/Appendixes/D11.6 RPCT User Manual Appendix A Warnings and Errors List.pdf", this);
}

void MainWindow::showAfblReference()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.5_AFBL_RM.pdf", this);
}

void MainWindow::showScriptHelp()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/scripthelp/index.html", this);
}

void MainWindow::showMatsUserManual()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.8_FSC_MATS_User_Manual.pdf", this);
}

void MainWindow::showTuningUserManual()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.9_FSC_Tuning_User_Manual.pdf", this);
}


void MainWindow::runConfigurator()
{
}

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
}



void MainWindow::updateUfbsAfbsBusses()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("Update schemas AFBs/UFBs/Busses."));
	mb.setInformativeText(tr("To prevent data loss all ApplicationLogic and UFB schemas must be checked in."));
	mb.setIcon(QMessageBox::NoIcon);
	QPushButton* updateButton = mb.addButton(tr("Update"), QMessageBox::ActionRole);
	mb.addButton(QMessageBox::Cancel);

	mb.exec();

	if (mb.clickedButton() != updateButton)
	{
		return;
	}

	GlobalMessanger::instance().fireChangeCurrentTab(m_editSchemaTabPage);

	// Get Busses
	//
	std::vector<VFrame30::Bus> busses;
	bool ok = EditSchemaWidget::loadBusses(dbController(), &busses, this);

	if (ok == false)
	{
		return;
	}

	// Get UFB schema list
	//
	QStringList checkedOutFiles;

	DbFileTree filesTree;
	db()->getFileListTree(&filesTree, db()->ufblFileId(), "%", true, this);

	std::vector<DbFileInfo>	ufbSchemaFileInfos = filesTree.toVectorIf(
		[](const DbFileInfo& file)
		{
			return file.fileName().endsWith(QLatin1String(".") + Db::File::UfbFileExtension, Qt::CaseInsensitive) == true &&
				file.isFolder() == false;
		});

	for (const DbFileInfo& f : ufbSchemaFileInfos)
	{
		if (f.state() == VcsState::CheckedOut)
		{
			checkedOutFiles.push_back(f.fileName());
		}
	}

	// Get ApplicationLogic schema list
	//
	filesTree.clear();
	db()->getFileListTree(&filesTree, db()->alFileId(), "%", true, this);

	std::vector<DbFileInfo>	alSchemaFileInfos = filesTree.toVectorIf(
		[](const DbFileInfo& file)
		{
			return file.fileName().endsWith(QLatin1String(".") + Db::File::AlFileExtension, Qt::CaseInsensitive) == true &&
				file.isFolder() == false;
		});

	for (const DbFileInfo& f : alSchemaFileInfos)
	{
		if (f.state() == VcsState::CheckedOut)
		{
			checkedOutFiles.push_back(f.fileName());
		}
	}

	if (checkedOutFiles.empty() == false)
	{
		QMessageBox mbError(this);

		mbError.setIcon(QMessageBox::Critical);
		mbError.setText(tr("Update AFBs/UFBs/Busses error."));
		mbError.setInformativeText("There are some checked out Application Logic and/or UFB schemas. CheckIn these files and repeat operation.");
		mbError.setDetailedText(checkedOutFiles.join(QChar::LineSeparator));

		mbError.exec();
		return;
	}

	// Update UFB schemas
	//
	LogicModuleSet logicModuleSet;
	int totalUpdatedAfbs = 0;

	QProgressDialog progress("Updating AFBs/Bussses on UFB schemas...", "Abort", 0, static_cast<int>(ufbSchemaFileInfos.size() + alSchemaFileInfos.size()), this);
	progress.setWindowModality(Qt::WindowModal);
	int progressIndicator = 0;

	QStringList updateDetails;

	{
		std::vector<DbFileInfo> allFiles;
		allFiles.reserve(ufbSchemaFileInfos.size() + alSchemaFileInfos.size());

		allFiles.insert(allFiles.end(), ufbSchemaFileInfos.begin(), ufbSchemaFileInfos.end());
		allFiles.insert(allFiles.end(), alSchemaFileInfos.begin(), alSchemaFileInfos.end());

		std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbSchemas;
		ufbSchemas.reserve(ufbSchemaFileInfos.size());

		for (DbFileInfo& fi : allFiles)
		{
			progress.setValue(progressIndicator++);
			if (progress.wasCanceled() == true)
			{
				break;
			}

			// Get latest version
			//
			std::shared_ptr<DbFile> file;
			ok = db()->getLatestVersion(fi, &file, this);

			if (ok == false || file == nullptr)
			{
				break;
			}

			// Load schema from file
			//
			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());

			if (schema == nullptr ||
				(schema->isUfbSchema() == false && schema->isLogicSchema() == false))
			{
				assert(schema->isUfbSchema() == true || schema->isLogicSchema() == true);
				QMessageBox::critical(this, qAppName(), tr("Error parsing schema %1.").arg(file->fileName()));
				break;
			}

			// Get UFB schema logic module description
			//
			QString lmDescriptionFile;

			if (schema->isUfbSchema() == true)
			{
				std::shared_ptr<VFrame30::UfbSchema> ufbSchema = std::dynamic_pointer_cast<VFrame30::UfbSchema>(schema);
				assert(ufbSchema);

				ufbSchemas.push_back(ufbSchema);

				lmDescriptionFile = ufbSchema->lmDescriptionFile();
			}
			else
			{
				if (schema->isLogicSchema())
				{
					std::shared_ptr<VFrame30::LogicSchema> logicSchema = std::dynamic_pointer_cast<VFrame30::LogicSchema>(schema);
					assert(logicSchema);

					lmDescriptionFile = logicSchema->lmDescriptionFile();
				}
				else
				{
					assert(false);
					break;
				}
			}

			if (logicModuleSet.has(lmDescriptionFile) == false)
			{
				QString errorMessage;
				ok = logicModuleSet.loadFile(db(), lmDescriptionFile, &errorMessage);

				if (ok == false)
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

			// Update AFBs on schemas
			//
			int thisSchemaUpdatedCount = 0;

			{
				int updatedCount = 0;
				QString updateErrorMessage;

				ok = schema->updateAllSchemaItemFbs(logicModuleDescription->afbs(), &updatedCount, &updateErrorMessage);

				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totalUpdatedAfbs += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// Update Busses on schemas
			//
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				ok = schema->updateAllSchemaItemBusses(busses, &updatedCount, &updateErrorMessage);

				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totalUpdatedAfbs += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// Update UFBs on schemas
			//
			if (schema->isLogicSchema() == true)
			{
				int updatedCount = 0;
				QString updateErrorMessage;

				ok = schema->updateAllSchemaItemUfb(ufbSchemas, &updatedCount, &updateErrorMessage);

				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), updateErrorMessage);
					break;
				}

				totalUpdatedAfbs += updatedCount;
				thisSchemaUpdatedCount += updatedCount;
			}

			// CheckOut and save file if changes are made
			//
			if (thisSchemaUpdatedCount > 0)
			{
				ok = db()->checkOut(fi, this);

				if (ok == false || fi.state() != VcsState::CheckedOut)
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

			updateDetails << tr("%1: %2, updated %3 item(s)")
								.arg(schema->isUfbSchema() ? "UFB" : "AL")
								.arg(schema->schemaId())
								.arg(thisSchemaUpdatedCount);
		}

		if (static_cast<size_t>(progressIndicator) != allFiles.size())
		{
			updateDetails << tr("...");
			updateDetails << tr("Operation is aborted");
		}
		else
		{
			updateDetails << QString("Done");
		}
	}

	progress.setValue(progress.maximum());

	// Show result, refresh files list
	//
	if (totalUpdatedAfbs != 0)
	{
		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 AFB(s) are updated according to the latest AFB description.").arg(totalUpdatedAfbs));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nCheckIn schemas to accept changes, Undo to reject.");
		msgBox.setDetailedText(updateDetails.join(QChar::LineSeparator));
		msgBox.exec();
	}

	// Refresh view
	//
	if (m_editSchemaTabPage != nullptr)
	{
		m_editSchemaTabPage->refreshControlTabPage();
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
}

void MainWindow::showAbout()
{
	QString text = "Supported project database version: " + QString::number(DbController::databaseVersion()) + "<br><br>";
	text += qApp->applicationName() + " provides offline tools for FSC chassis configuration, application logic design and its compilation, visualization design and MATS software configuration.<br>";

	DialogAbout::show(this, text, ":/Images/Images/logo.png");

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
	m_busEditorAction->setEnabled(true);
	m_updateUfbsAfbs->setEnabled(true);
	m_AfbLibraryCheck->setEnabled(true);

	// Status bar
	//
	assert(m_statusBarConnectionState != nullptr);

	m_statusBarConnectionState->setText(tr("Opened: ") + theSettings.serverIpAddress());

	// Show and hide FilesTabPage
	//
	if (db()->currentUser().isAdminstrator() == true)
	{
		getCentralWidget()->insertTab(m_filesTabPageIndex, m_filesTabPage, m_filesTabPage->windowTitle());
	}

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
	m_busEditorAction->setEnabled(false);
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

	return;
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

