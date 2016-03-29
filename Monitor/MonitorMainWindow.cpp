#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "MonitorSchemaWidget.h"
#include "../VFrame30/Scheme.h"

MonitorMainWindow::MonitorMainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	qDebug() << Q_FUNC_INFO;

	// --
	//
	setCentralWidget(new MonitorCentralWidget());

	// Create Menus, ToolBars, StatusBar
	//
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	// Add main tab pages
	//
	//getCentralWidget()->addTabPage(new ProjectsTabPage(dbController(), nullptr), tr("Projects"));
	//getCentralWidget()->addTabPage(new EquipmentTabPage(dbController(), nullptr), tr("Hardware Configuration"));

	// --
	//
	setMinimumSize(500, 300);
	restoreWindowState();

	centralWidget()->show();

	return;
}

MonitorMainWindow::~MonitorMainWindow()
{
	qDebug() << Q_FUNC_INFO;

	return;
}

void MonitorMainWindow::closeEvent(QCloseEvent* e)
{
	saveWindowState();
	e->accept();

	return;
}

void MonitorMainWindow::saveWindowState()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theSettings.writeUserScope();

	return;
}

void MonitorMainWindow::restoreWindowState()
{
	theSettings.loadUserScope();

	move(theSettings.m_mainWindowPos);
	restoreGeometry(theSettings.m_mainWindowGeometry);
	restoreState(theSettings.m_mainWindowState);

	return;
}

void MonitorMainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MonitorMainWindow::exit);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MonitorMainWindow::showSettings);

	m_pDebugAction = new QAction(tr("Debug..."), this);
	m_pDebugAction->setStatusTip(tr("Perform some debug actions, don't run it!"));
	m_pDebugAction->setEnabled(true);
	connect(m_pDebugAction, &QAction::triggered, this, &MonitorMainWindow::debug);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
	connect(m_pLogAction, &QAction::triggered, this, &MonitorMainWindow::showLog);

	m_pAboutAction = new QAction(tr("About..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MonitorMainWindow::showAbout);

	return;
}

void MonitorMainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

#ifdef Q_DEBUG
	pHelpMenu->addAction(m_pDebugAction);
#endif	// Q_DEBUG
	pHelpMenu->addAction(m_pLogAction);
	pHelpMenu->addAction(m_pAboutAction);

	return;
}

void MonitorMainWindow::createToolBars()
{
}

void MonitorMainWindow::createStatusBar()
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

MonitorCentralWidget* MonitorMainWindow::monitorCentralWidget()
{
	MonitorCentralWidget* centralWidget = dynamic_cast<MonitorCentralWidget*>(QMainWindow::centralWidget());
	assert(centralWidget != nullptr);

	return centralWidget;
}

void MonitorMainWindow::exit()
{
	close();
}

void MonitorMainWindow::showLog()
{

}

void MonitorMainWindow::showSettings()
{
	DialogSettings d(this);
	d.setSettings(theSettings);

	if (d.exec() == QDialog::Accepted)
	{
		theSettings = d.settings();
		theSettings.writeSystemScope();

		// Apply settings here
		//
		//dbController()->setHost(theSettings.serverIpAddress());
		return;
	}

	return;
}

void MonitorMainWindow::showAbout()
{
	QMessageBox::about(this, tr("About Monitor"), tr("Monitor software. Version not assigned."));
}

void MonitorMainWindow::debug()
{
#ifdef Q_DEBUG

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													"./",
													tr("Monitor schemas (*.mvs);; All files (*.*)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	QFileInfo fileInfo(fileName);

	// Load scheme
	//
	std::shared_ptr<VFrame30::Scheme> scheme = std::shared_ptr<VFrame30::Scheme>(VFrame30::Scheme::Create(fileName.toStdWString().c_str()));

	if (scheme == nullptr)
	{
		QMessageBox::critical(this, "Monitor", "Cannot load file");
		return;
	}

	// Create tab
	//
	QTabWidget* tabWidget = monitorCentralWidget();


	MonitorSchemaWidget* schemeWidget = new MonitorSchemaWidget(scheme);
	tabWidget->addTab(schemeWidget, "Debug tab: " + fileInfo.fileName());

#endif	// Q_DEBUG
}



