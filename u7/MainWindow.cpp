#include "Stable.h"
#include <QCloseEvent>
#include <QDialogButtonBox>
#include "MainWindow.h"
#include "CentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "../lib/DbController.h"
#include "UserManagementDialog.h"
#include "ProjectsTabPage.h"
#include "FilesTabPage.h"
#include "SchemaTabPage.h"
#include "EquipmentTabPage.h"
#include "SignalsTabPage.h"
#include "DialogAfblEditor.h"
#include "DialogSubsystemListEditor.h"
#include "DialogConnections.h"
#include "DialogTuningClients.h"
#include "BuildTabPage.h"
#include "UploadTabPage.h"
#include "GlobalMessanger.h"
#include "Forms/FileHistoryDialog.h"
#include "version.h"

#include "../VFrame30/VFrame30.h"

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
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &MainWindow::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &MainWindow::projectClosed);
	connect(GlobalMessanger::instance(), &GlobalMessanger::changeCurrentTab, getCentralWidget(), &CentralWidget::setCurrentWidget);

	// Add main tab pages
	//
	m_projectsTab = new ProjectsTabPage(dbController(), nullptr);
	m_equipmentTab = new EquipmentTabPage(dbController(), nullptr);
	m_signalsTab = new SignalsTabPage(dbController(), nullptr);

	m_filesTabPage = new FilesTabPage(dbController(), nullptr);
	m_filesTabPage->setWindowTitle(tr("Files"));

//	m_logicSchema = SchemasTabPage::create<VFrame30::LogicSchema, VFrame30::UfbSchema>(
//				dbController(), nullptr,
//				::AlFileExtension, ::AlFileName, ::AlTemplExtension, tr("Control"),
//				::UfbFileExtension, ::UfblFileName, ::UfbTemplExtension, tr("UFB Library"));

	m_logicSchema = SchemasTabPage::create<VFrame30::LogicSchema>(
						dbController(), nullptr,
						::AlFileExtension, ::AlFileName, ::AlTemplExtension, tr("AppLogic"));

	m_ufbLibrary = SchemasTabPage::create<VFrame30::UfbSchema>(
					   dbController(), nullptr,
					   ::UfbFileExtension, ::UfblFileName, ::UfbTemplExtension, tr("UFB Library"));
	m_ufbLibrary->setWindowTitle(tr("UFB Library"));

	m_monitorSchema = SchemasTabPage::create<VFrame30::MonitorSchema>(
						  dbController(), nullptr,
						  ::MvsFileExtension, ::MvsFileName, ::MvsTemplExtension, tr("Control"));

	getCentralWidget()->addTabPage(m_projectsTab, tr("&Projects"));
	getCentralWidget()->addTabPage(m_equipmentTab, tr("&Equipment"));
	getCentralWidget()->addTabPage(m_signalsTab, tr("Application &Signals"));

	m_filesTabPageIndex = getCentralWidget()->addTabPage(m_filesTabPage, m_filesTabPage->windowTitle());
	getCentralWidget()->removeTab(m_filesTabPageIndex);	// It will be added in projectOpened slot if required

	//m_diagSchema = SchemasTabPage::create<VFrame30::DiagSchema>(DvsFileExtension, dbController(), DvsFileName, nullptr);

	getCentralWidget()->addTabPage(m_logicSchema, tr("Application &Logic"));
	getCentralWidget()->addTabPage(m_monitorSchema, tr("&Monitor Schemas"));
	//getCentralWidget()->addTabPage(m_diagSchema, tr("Diag Schemas"));

	m_buildTabPage = new BuildTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_buildTabPage, tr("&Build"));

	m_uploadTabPage = new UploadTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_uploadTabPage, tr("&Upload"));

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

	if (m_ufbLibrary->parent() == nullptr)
	{
		delete m_ufbLibrary;
	}
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

	// check if any schema is not saved
	//
	if (m_logicSchema == nullptr ||
		m_monitorSchema == nullptr)
		//m_diagSchema == nullptr)
	{
		assert(m_logicSchema);
		assert(m_monitorSchema);
		//assert(m_diagSchema);
		e->accept();
		return;
	}

	if (m_logicSchema->hasUnsavedSchemas() == true ||
		m_monitorSchema->hasUnsavedSchemas() == true)
		//m_diagSchema->hasUnsavedSchemas() == true)
	{
		QMessageBox::StandardButton result = QMessageBox::question(this, QApplication::applicationName(),
																   tr("Some schemas have unsaved changes."),
																   QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel,
																   QMessageBox::SaveAll);

		if (result == QMessageBox::Cancel)
		{
			e->ignore();
			return;
		}

		if (result == QMessageBox::SaveAll)
		{
			m_logicSchema->saveUnsavedSchemas();
			m_monitorSchema->saveUnsavedSchemas();
			//m_diagSchema->saveUnsavedSchemas();
		}
	}

	// save windows state and accept event to close app
	//
	saveWindowState();

	e->accept();

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
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
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

	m_settingsAction = new QAction(tr("Settings..."), this);
	m_settingsAction->setStatusTip(tr("Change application settings"));
	m_settingsAction->setEnabled(true);
	connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_afblEditorAction = new QAction(tr("AFB Library Editor..."), this);
	m_afblEditorAction->setStatusTip(tr("Run AFB Editor"));
	m_afblEditorAction->setEnabled(false);
	connect(m_afblEditorAction, &QAction::triggered, this, &MainWindow::runAfblEditor);

	m_ufbLibraryAction = new QAction(tr("UFB Library Editor..."), this);
	m_ufbLibraryAction->setStatusTip(tr("Run UFB Library Editor"));
	m_ufbLibraryAction->setEnabled(false);
	m_ufbLibraryAction->setCheckable(true);
	connect(m_ufbLibraryAction, &QAction::toggled, this, &MainWindow::showUfbLibraryTabPage);

    m_subsystemListEditorAction = new QAction(tr("Subsystem List Editor..."), this);
	m_subsystemListEditorAction->setStatusTip(tr("Run Subsystem List Editor"));
	m_subsystemListEditorAction->setEnabled(false);
	connect(m_subsystemListEditorAction, &QAction::triggered, this, &MainWindow::runSubsystemListEditor);

    m_connectionsEditorAction = new QAction(tr("Connections Editor..."), this);
    m_connectionsEditorAction->setStatusTip(tr("Run Connections Editor"));
    m_connectionsEditorAction->setEnabled(false);
    connect(m_connectionsEditorAction, &QAction::triggered, this, &MainWindow::runConnectionsEditor);

    m_tuningFiltersEditorAction = new QAction(tr("Tuning Clients Filters Editor..."), this);
    m_tuningFiltersEditorAction->setStatusTip(tr("Run Tuning Clients Filters Editor"));
    m_tuningFiltersEditorAction->setEnabled(false);
    connect(m_tuningFiltersEditorAction, &QAction::triggered, this, &MainWindow::runTuningFiltersEditor);

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
	connect(GlobalMessanger::instance(), &GlobalMessanger::buildStarted, this, [this](){m_startBuildAction->setEnabled(false);});
	connect(GlobalMessanger::instance(), &GlobalMessanger::buildFinished, this, [this](){m_startBuildAction->setEnabled(true);});
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_startBuildAction->setEnabled(true);});
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_startBuildAction->setEnabled(false);});
	addAction(m_startBuildAction);


	m_projectHistoryAction = new QAction(tr("Project History..."), this);
	m_projectHistoryAction->setStatusTip(tr("Show project history"));
	m_projectHistoryAction->setEnabled(false);
	connect(m_projectHistoryAction, &QAction::triggered, this, &MainWindow::projectHistory);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, [this](){m_projectHistoryAction->setEnabled(true);});
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, [this](){m_projectHistoryAction->setEnabled(false);});
	addAction(m_projectHistoryAction);

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
	pProjectMenu->addAction(m_startBuildAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_afblEditorAction);
	pToolsMenu->addAction(m_ufbLibraryAction);
	pToolsMenu->addAction(m_subsystemListEditorAction);
	pToolsMenu->addAction(m_connectionsEditorAction);
    pToolsMenu->addAction(m_tuningFiltersEditorAction);

	pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_settingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

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
	close();
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

void MainWindow::runConfigurator()
{
}

void MainWindow::runAfblEditor()
{
    if (dbController()->isProjectOpened() == false)
    {
        return;
    }

    DialogAfblEditor d(dbController(), this);
    d.exec();
}

void MainWindow::showUfbLibraryTabPage(bool show)
{
	qDebug() << "Show Ufb Library TabPage, show = " << show;

	if (m_ufbLibrary == nullptr ||
		m_logicSchema == nullptr)
	{
		assert(m_logicSchema);
		assert(m_ufbLibrary);
		return;
	}

	if (show == true)
	{
		// Add m_ufbLibrary to TabPage after AppSchema
		//
		int logicSchemaTabIndex = getCentralWidget()->indexOf(m_logicSchema);

		if (logicSchemaTabIndex == -1)
		{
			assert(logicSchemaTabIndex != -1);
			return;
		}

		getCentralWidget()->insertTab(logicSchemaTabIndex + 1, m_ufbLibrary, m_ufbLibrary->windowTitle());
		getCentralWidget()->setCurrentWidget(m_ufbLibrary);
	}
	else
	{
		int tabIndex = getCentralWidget()->indexOf(m_ufbLibrary);

		if (tabIndex == -1)
		{
			assert(tabIndex != -1);
			return;
		}

		getCentralWidget()->removeTab(tabIndex);
	}

	return;
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

void MainWindow::runTuningFiltersEditor()
{
    if (dbController()->isProjectOpened() == false)
    {
        return;
    }

    if (theDialogTuningClients == nullptr)
    {
        theDialogTuningClients = new DialogTuningClients(this);
        theDialogTuningClients->show();
    }
    else
    {
        theDialogTuningClients->activateWindow();
    }
}

void MainWindow::showAbout()
{
	QDialog aboutDialog(this);

	QHBoxLayout* hl = new QHBoxLayout;

	QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(":/Images/Images/logo.png"));

	hl->addWidget(logo);

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";
#ifndef Q_DEBUG
	text += "Build: Release";
#else
	text += "Build: Debug";
#endif
	text += "<br>Commit SHA1: " USED_SERVER_COMMIT_SHA;
	text += "<br>Supported project database version: " + QString::number(DbController::databaseVersion());

	QLabel* label = new QLabel(text, &aboutDialog);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vl->addWidget(label);

	label = new QLabel(&aboutDialog);
	label->setText(qApp->applicationName() + " provides offline tools for FSC chassis configuration, application logic design and its compilation, visualization design and SCADA software configuration.");
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	label->setWordWrap(true);
	vl->addWidget(label);

	QPushButton* copyCommitSHA1Button = new QPushButton("Copy commit SHA1");
	connect(copyCommitSHA1Button, &QPushButton::clicked, [](){
		qApp->clipboard()->setText(USED_SERVER_COMMIT_SHA);
	});

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(copyCommitSHA1Button, QDialogButtonBox::ActionRole);
	buttonBox->addButton(QDialogButtonBox::Ok);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hl);
	mainLayout->addWidget(buttonBox);
	aboutDialog.setLayout(mainLayout);

	connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);

	aboutDialog.exec();
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

void MainWindow::projectOpened(DbProject project)
{
	setWindowTitle(qApp->applicationName() + QString(" - ") + project.projectName() + QString(" - ") + dbController()->currentUser().username());

	// Action, disable/enable
	//
	assert(m_usersAction != nullptr);

	m_usersAction->setEnabled(true);
    m_afblEditorAction->setEnabled(true);
	m_ufbLibraryAction->setEnabled(true);
	m_subsystemListEditorAction->setEnabled(true);
    m_connectionsEditorAction->setEnabled(true);
    m_tuningFiltersEditorAction->setEnabled(true);


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
	m_afblEditorAction->setEnabled(false);
	m_ufbLibraryAction->setEnabled(false);
	m_subsystemListEditorAction->setEnabled(false);
    m_connectionsEditorAction->setEnabled(false);
    m_tuningFiltersEditorAction->setEnabled(false);

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
