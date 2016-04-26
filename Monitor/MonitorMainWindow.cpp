#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "MonitorSchemaWidget.h"
#include "../VFrame30/Schema.h"

MonitorMainWindow::MonitorMainWindow(MonitorConfigController* configController, QWidget *parent) :
	QMainWindow(parent),
	m_configController(configController),
	m_schemaManager(configController)
{
	qDebug() << Q_FUNC_INFO;

	assert(m_configController);

	// --
	//
	MonitorCentralWidget* monitorCentralWidget = new MonitorCentralWidget(&m_schemaManager);
	setCentralWidget(monitorCentralWidget);

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
	m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MonitorMainWindow::exit);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
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
	m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MonitorMainWindow::showAbout);

	m_newTabAction = new QAction(tr("New Tab"), this);
	m_newTabAction->setStatusTip(tr("Open current schema in new tab page"));
	m_newTabAction->setIcon(QIcon(":/Images/Images/NewSchema.svg"));
	m_newTabAction->setEnabled(true);
	QList<QKeySequence> newTabShortcuts;
	newTabShortcuts << QKeySequence::AddTab;
	newTabShortcuts << QKeySequence::New;
	m_newTabAction->setShortcuts(newTabShortcuts);
	connect(m_newTabAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_newTab);

	m_closeTabAction = new QAction(tr("Close Tab"), this);
	m_closeTabAction->setStatusTip(tr("Close current tab page"));
	m_closeTabAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_closeTabAction->setEnabled(true);
	m_closeTabAction->setShortcuts(QKeySequence::Close);
	connect(m_closeTabAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_closeCurrentTab);

	m_zoomInAction = new QAction(tr("ZoomIn"), this);
	m_zoomInAction->setStatusTip(tr("Zoom In schema view"));
	m_zoomInAction->setIcon(QIcon(":/Images/Images/ZoomIn.svg"));
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcuts(QKeySequence::ZoomIn);

	m_zoomOutAction = new QAction(tr("ZoomOut"), this);
	m_zoomOutAction->setStatusTip(tr("Zoom Out schema view"));
	m_zoomOutAction->setIcon(QIcon(":/Images/Images/ZoomOut.svg"));
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcuts(QKeySequence::ZoomOut);

	m_historyBackward = new QAction(tr("Go Back"), this);
	m_historyBackward->setStatusTip(tr("Click to got back"));
	m_historyBackward->setIcon(QIcon(":/Images/Images/Backward.svg"));
	m_historyBackward->setEnabled(true);
	m_historyBackward->setShortcuts(QKeySequence::Back);

	m_historyForward = new QAction(tr("Go Forward"), this);
	m_historyForward->setStatusTip(tr("Click to got forward"));
	m_historyForward->setIcon(QIcon(":/Images/Images/Forward.svg"));
	m_historyForward->setEnabled(true);
	m_historyForward->setShortcuts(QKeySequence::Forward);

	return;
}

void MonitorMainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Schema
	//
	QMenu* schemaMenu = menuBar()->addMenu(tr("&Schema"));

	schemaMenu->addAction(m_newTabAction);
	schemaMenu->addAction(m_closeTabAction);

	// View
	//
	QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

	viewMenu->addAction(m_zoomInAction);
	viewMenu->addAction(m_zoomOutAction);
	viewMenu->addSeparator();

	viewMenu->addAction(m_historyForward);
	viewMenu->addAction(m_historyBackward );


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
	m_toolBar = new QToolBar(this);
	m_toolBar->setMovable(false);
	m_toolBar->setIconSize(QSize(32, 32));
	m_toolBar->setStyleSheet("QToolBar{spacing:6px;padding:6px;}");

	m_toolBar->addAction(m_newTabAction);
	m_toolBar->addSeparator();

	m_toolBar->addAction(m_zoomInAction);
	m_toolBar->addAction(m_zoomOutAction);
	m_toolBar->addSeparator();

	m_toolBar->addAction(m_historyBackward);
	m_toolBar->addAction(m_historyForward);

	this->addToolBar(Qt::TopToolBarArea, m_toolBar);

	return;
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

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		theSettings = d.settings();
		theSettings.writeSystemScope();

		// Apply settings here
		//
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

	// Load schema
	//
	std::shared_ptr<VFrame30::Schema> schema = std::shared_ptr<VFrame30::Schema>(VFrame30::Schema::Create(fileName.toStdWString().c_str()));

	if (schema == nullptr)
	{
		QMessageBox::critical(this, "Monitor", "Cannot load file");
		return;
	}

	// Create tab
	//
//	QTabWidget* tabWidget = monitorCentralWidget();

//	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(schema);
//	tabWidget->addTab(schemaWidget, "Debug tab: " + fileInfo.fileName());

#endif	// Q_DEBUG
}



