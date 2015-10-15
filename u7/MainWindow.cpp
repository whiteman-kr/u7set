#include "Stable.h"
#include <QCloseEvent>
#include "MainWindow.h"
#include "CentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "../include/DbController.h"
#include "UserManagementDialog.h"
#include "ProjectsTabPage.h"
#include "FilesTabPage.h"
#include "SchemeTabPage.h"
#include "EquipmentTabPage.h"
#include "SignalsTabPage.h"
#include "DialogAfblEditor.h"
#include "DialogSubsystemListEditor.h"
#include "BuildTabPage.h"

#include "../VFrame30/VFrame30.h"

MainWindow::MainWindow(DbController* dbcontroller, QWidget* parent) :
	QMainWindow(parent),
	m_pExitAction(nullptr),
	m_pUsersAction(nullptr),
	m_pLogAction(nullptr),
	m_pSettingsAction(nullptr),
	m_pConfiguratorAction(nullptr),
	m_pAboutAction(nullptr),
	m_pStatusBarInfo(nullptr),
	m_pStatusBarConnectionStatistics(nullptr),
	m_pStatusBarConnectionState(nullptr),
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
	connect(dbController(), &DbController::projectOpened, this, &MainWindow::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &MainWindow::projectClosed);

	////connect(dbStore(), &DbStore::error, this, &MainWindow::databaseError);
	////connect(dbStore(), &DbStore::completed, this, &MainWindow::databaseOperationCompleted);

	//connect(centralWidget(), SIGNAL(historyChanged(bool, bool)), this, SLOT(historyChanged(bool, bool)));

	// Add main tab pages
	//
	getCentralWidget()->addTabPage(new ProjectsTabPage(dbController(), nullptr), tr("Projects"));
	getCentralWidget()->addTabPage(new EquipmentTabPage(dbController(), nullptr), tr("Equipment"));
	getCentralWidget()->addTabPage(new SignalsTabPage(dbController(), nullptr), tr("Application Signals"));
	getCentralWidget()->addTabPage(new FilesTabPage(dbController(), nullptr), tr("Files"));
//	getCentralWidget()->addTabPage(new ConfigurationsTabPage(dbController(), nullptr), tr("Modules Configurations"));

	m_logicScheme = SchemesTabPage::create<VFrame30::LogicScheme>("als", dbController(), AlFileName, nullptr);
	m_workflowScheme = SchemesTabPage::create<VFrame30::WorkflowScheme>("wvs", dbController(), WvsFileName, nullptr);
	m_diagScheme = SchemesTabPage::create<VFrame30::DiagScheme>("dvs", dbController(), DvsFileName, nullptr);

	getCentralWidget()->addTabPage(m_logicScheme, tr("Application Logic"));
	getCentralWidget()->addTabPage(m_workflowScheme, tr("Workflow Schemes"));
	getCentralWidget()->addTabPage(m_diagScheme, tr("Diag Schemes"));

	BuildTabPage* buildTabPage = new BuildTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(buildTabPage, tr("Build"));

	// Connect BuildStarted and BuildFinished signals to SchemesTabPage
	//
	connect(buildTabPage, &BuildTabPage::buildStarted, m_logicScheme, &SchemesTabPage::buildStarted);
	connect(buildTabPage, &BuildTabPage::buildFinished, m_logicScheme, &SchemesTabPage::buildFinished);
	connect(buildTabPage, &BuildTabPage::buildStarted, m_workflowScheme, &SchemesTabPage::buildStarted);
	connect(buildTabPage, &BuildTabPage::buildFinished, m_workflowScheme, &SchemesTabPage::buildFinished);
	connect(buildTabPage, &BuildTabPage::buildStarted, m_diagScheme, &SchemesTabPage::buildStarted);
	connect(buildTabPage, &BuildTabPage::buildFinished, m_diagScheme, &SchemesTabPage::buildFinished);


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
	// check if any scheme is not saved
	//
	if (m_logicScheme == nullptr ||
		m_workflowScheme == nullptr ||
		m_diagScheme == nullptr)
	{
		assert(m_logicScheme);
		assert(m_workflowScheme);
		assert(m_diagScheme);
		e->accept();
		return;
	}

	if (m_logicScheme->hasUnsavedSchemes() == true ||
		m_workflowScheme->hasUnsavedSchemes() == true ||
		m_diagScheme->hasUnsavedSchemes() == true)
	{
		QMessageBox::StandardButton result = QMessageBox::question(this, QApplication::applicationName(),
			 tr("Some schemes have unsaved changes."),
			QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::SaveAll);

		if (result == QMessageBox::Cancel)
		{
			e->ignore();
			return;
		}

		if (result == QMessageBox::SaveAll)
		{
			m_logicScheme->saveUnsavedSchemes();
			m_workflowScheme->saveUnsavedSchemes();
			m_diagScheme->saveUnsavedSchemes();
		}
	}

	// save windows state and accept event to close app
	//
	saveWindowState();

	e->accept();

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
}

void MainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MainWindow::exit);

	m_pUsersAction = new QAction(tr("Users..."), this);
	m_pUsersAction->setStatusTip(tr("User management"));
	m_pUsersAction->setEnabled(false);
	connect(m_pUsersAction, &QAction::triggered, this, &MainWindow::userManagement);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
	connect(m_pLogAction, &QAction::triggered, this, &MainWindow::showLog);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_pConfiguratorAction = new QAction(tr("Module Configurator..."), this);
	m_pConfiguratorAction->setStatusTip(tr("Run module configurator"));
	//m_pConfiguratorAction->setEnabled(true);
	connect(m_pConfiguratorAction, &QAction::triggered, this, &MainWindow::runConfigurator);

    m_pAfblEditorAction = new QAction(tr("AFBL Editor..."), this);
    m_pAfblEditorAction->setStatusTip(tr("Run AFBL Editor"));
	m_pAfblEditorAction->setEnabled(false);
    connect(m_pAfblEditorAction, &QAction::triggered, this, &MainWindow::runAfblEditor);

	m_pSubsystemListEditorAction = new QAction(tr("Subsystem List Editor..."), this);
	m_pSubsystemListEditorAction->setStatusTip(tr("Run Subsystem List Editor"));
	m_pSubsystemListEditorAction->setEnabled(false);
	connect(m_pSubsystemListEditorAction, &QAction::triggered, this, &MainWindow::runSubsystemListEditor);

	m_pAboutAction = new QAction(tr("About..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_pDebugAction = new QAction(tr("Debug..."), this);
	m_pDebugAction->setStatusTip(tr("Perform some debug actions, don't run it!"));
	m_pDebugAction->setEnabled(true);
	connect(m_pDebugAction, &QAction::triggered, this, &MainWindow::debug);

	return;
}

void MainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Administration
	//
	QMenu* pAdmMenu = menuBar()->addMenu(tr("&Administration"));

	pAdmMenu->addAction(m_pUsersAction);
	pAdmMenu->addAction(m_pLogAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_pConfiguratorAction);
    pToolsMenu->addAction(m_pAfblEditorAction);
	pToolsMenu->addAction(m_pSubsystemListEditorAction);
    pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pAboutAction);
	pHelpMenu->addAction(m_pDebugAction);

	return;
}

void MainWindow::createToolBars()
{
}

void MainWindow::createStatusBar()
{
	m_pStatusBarInfo = new QLabel();
	m_pStatusBarInfo->setAlignment(Qt::AlignLeft);
	m_pStatusBarInfo->setIndent(3);

	m_pStatusBarConnectionStatistics = new QLabel();
	m_pStatusBarConnectionStatistics->setAlignment(Qt::AlignHCenter);
	m_pStatusBarConnectionStatistics->setMinimumWidth(100);

	m_pStatusBarConnectionState = new QLabel();
	m_pStatusBarConnectionState->setAlignment(Qt::AlignHCenter);
	m_pStatusBarConnectionState->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_pStatusBarInfo, 1);
	statusBar()->addPermanentWidget(m_pStatusBarConnectionStatistics, 0);
	statusBar()->addPermanentWidget(m_pStatusBarConnectionState, 0);

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

	if (d.exec() == QDialog::Accepted)
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

void MainWindow::runSubsystemListEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	DialogSubsystemListEditor d(dbController(), this);
	d.exec();
}

void MainWindow::showAbout()
{

}

void MainWindow::debug()
{
	std::vector<std::shared_ptr<DbFile>> files;

	// Add file
	std::shared_ptr<DbFile> f1 = std::make_shared<DbFile>();
	f1->setFileName("file_1.fcf");

	std::shared_ptr<DbFile> f2 = std::make_shared<DbFile>();
	f2->setFileName("file_2.fcf");

	QByteArray data1;
	data1.push_back(1);
	data1.push_back(2);
	data1.push_back(3);
	f1->swapData(data1);

	QByteArray data2;
	data2.push_back(3);
	data2.push_back(4);
	f2->swapData(data2);

	files.push_back(f1);
	files.push_back(f2);

	dbController()->addFiles(&files, 0, this);

	// Ckeck in file_1
	std::vector<DbFileInfo>	fiv;
	fiv.push_back(static_cast<DbFileInfo>(*f1));

	dbController()->checkIn(fiv, "check_in file 1", this);

	// check out file_1
	dbController()->checkOut(fiv, this);

	// undo all files
	fiv.clear();
	fiv.push_back(static_cast<DbFileInfo>(*f1));
	fiv.push_back(static_cast<DbFileInfo>(*f2));
	dbController()->undoChanges(fiv, this);

	/*DbFileInfo fi;
	fi.setFileId(1);*/

	//std::shared_ptr<DbFile> file;

	//dbController()->getWorkcopy(fi, &file, this);

	//qDebug() << "Get Workcopy, size:" << file->size() << " Name?:" << file->fileName();

	//f1->setFileId(1);
	//dbController()->setWorkcopy(files, this);


	//dbController()->addFiles(&files, 0, this);
	//dbController()->addFiles(&files, 0, this);

	//std::vector<DbFileInfo> files;
	//dbController()->getFileList(&files, this);

}

void MainWindow::projectOpened(DbProject project)
{
	setWindowTitle(qApp->applicationName() + QString(" - ") + project.projectName() + QString(" - ") + dbController()->currentUser().username());

	// Action, disable/enable
	//
	assert(m_pUsersAction != nullptr);

	m_pUsersAction->setEnabled(true);
	m_pAfblEditorAction->setEnabled(true);
	m_pSubsystemListEditorAction->setEnabled(true);

	// Tab Pages, enable all tab pages
	//
	//getCentralWidget()->setEnabled(true);

	// Status bar
	//
	assert(m_pStatusBarConnectionState != nullptr);

	m_pStatusBarConnectionState->setText(tr("Opened: ") + theSettings.serverIpAddress());
	return;
}

void MainWindow::projectClosed()
{
	setWindowTitle(qApp->applicationName());

	// Actions, disable/enable
	//
	assert(m_pUsersAction != nullptr);

	m_pUsersAction->setEnabled(false);
	m_pAfblEditorAction->setEnabled(false);
	m_pSubsystemListEditorAction->setEnabled(false);

	// Tab Pages, disable all tab pages except the first.
	//
	//getCentralWidget()->setDisabled(true);

	// Status bar
	//
	assert(m_pStatusBarConnectionState != nullptr);

	m_pStatusBarConnectionState->setText(tr("Closed"));
	return;
}


DbController* MainWindow::dbController()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}
