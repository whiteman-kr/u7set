#include "Stable.h"
#include <QCloseEvent>
#include "MainWindow.h"
#include "CentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "../include/DbStore.h"
#include "UserManagementDialog.h"
#include "DatabaseTabPage.h"
#include "FilesTabPage.h"
#include "ConfigurationsTabPage.h"
#include "VideoFrameTabPage.h"
#include "EquipmentTabPage.h"

#include "../VFrame30/VFrame30.h"

MainWindow::MainWindow(DbStore* dbstore, QWidget* parent) :
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
	m_pDbStore(dbstore)
{
	assert(m_pDbStore);

	// --
	//
	setCentralWidget(new CentralWidget());

	// Create Menus, ToolBars, StatusBar
	//
	createActions();
    createMenus();
	createToolBars();
	createStatusBar();

	restoreWindowState();

	// --
	//
	connect(dbStore(), &DbStore::projectOpened, this, &MainWindow::projectOpened);
	connect(dbStore(), &DbStore::projectClosed, this, &MainWindow::projectClosed);

	connect(dbStore(), &DbStore::error, this, &MainWindow::databaseError);
	connect(dbStore(), &DbStore::completed, this, &MainWindow::databaseOperationCompleted);

	//connect(centralWidget(), SIGNAL(historyChanged(bool, bool)), this, SLOT(historyChanged(bool, bool)));

	// Add main tab pages
	//
	getCentralWidget()->addTabPage(new DatabaseTabPage(dbStore(), nullptr), tr("Database"));
	getCentralWidget()->addTabPage(new EquipmentTabPage(dbStore(), nullptr), tr("Equipment"));
	getCentralWidget()->addTabPage(new FilesTabPage(dbStore(), nullptr), tr("Files"));
	getCentralWidget()->addTabPage(new ConfigurationsTabPage(dbStore(), nullptr), tr("Configurations"));

	getCentralWidget()->addTabPage(VideoFrameTabPage::create<VFrame30::CVideoFrameLogic>("lvf", dbStore(), nullptr), tr("Logic"));
	getCentralWidget()->addTabPage(VideoFrameTabPage::create<VFrame30::CVideoFrameWiring>("wvf", dbStore(), nullptr), tr("Wiring"));
	getCentralWidget()->addTabPage(VideoFrameTabPage::create<VFrame30::CVideoFrameTech>("tvf", dbStore(), nullptr), tr("Tech Frames"));
	getCentralWidget()->addTabPage(VideoFrameTabPage::create<VFrame30::CVideoFrameDiag>("dvf", dbStore(), nullptr), tr("Diag Frames"));

	// --
	//
	setMinimumSize(500, 300);

	centralWidget()->show();

	return;
}

MainWindow::~MainWindow()
{
	qDebug() << Q_FUNC_INFO;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	//QMessageBox mb(this);
	//mb.setText(tr("Do you want to save your changes?"));
	//mb.exec();

	saveWindowState();

	e->accept();
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
	m_pExitAction->setShortcut(QKeySequence::Close);
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
	UserManagementDialog d(this, dbStore());

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

		dbStore()->setHost(theSettings.serverIpAddress());
		dbStore()->setPort(theSettings.serverPort());
		dbStore()->setServerUsername(theSettings.serverUsername());
		dbStore()->setServerPassword(theSettings.serverPassword());

		return;
	}

	return;
}

void MainWindow::runConfigurator()
{
}

void MainWindow::showAbout()
{

}

void MainWindow::debug()
{
	dbStore()->debug();
}

void MainWindow::projectOpened()
{
	setWindowTitle(qApp->applicationName() + QString(" - ") + dbStore()->currentProject().projectName() + QString(" - ") + dbStore()->currentUser().username());

	// Action, disable/enable
	//
	assert(m_pUsersAction != nullptr);

	m_pUsersAction->setEnabled(true);

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

	// Tab Pages, disable all tab pages except the first.
	//
	//getCentralWidget()->setDisabled(true);

	// Status bar
	//
	assert(m_pStatusBarConnectionState != nullptr);

	m_pStatusBarConnectionState->setText(tr("Closed"));
	return;
}

void MainWindow::databaseError(QString message)
{
	QMessageBox::critical(this, qApp->applicationName(), message);
	return;
}

void MainWindow::databaseOperationCompleted(QString message)
{
	if (message.isEmpty() == false)
	{
		QMessageBox mb(this);

		mb.setText(message);

		mb.exec();
	}

	return;
}

DbStore* MainWindow::dbStore()
{
	assert(m_pDbStore != nullptr);
	return m_pDbStore;
}
