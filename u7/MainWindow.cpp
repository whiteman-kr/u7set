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
#include "SchemaTabPage.h"
#include "EquipmentTabPage.h"
#include "SignalsTabPage.h"
#include "DialogAfblEditor.h"
#include "DialogSubsystemListEditor.h"
#include "DialogConnectionsEditor.h"
#include "Rs232SignalListEditor.h"
#include "BuildTabPage.h"
#include "GlobalMessanger.h"

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
	getCentralWidget()->addTabPage(new ProjectsTabPage(dbController(), nullptr), tr("Projects"));
	getCentralWidget()->addTabPage(new EquipmentTabPage(dbController(), nullptr), tr("Equipment"));
	getCentralWidget()->addTabPage(new SignalsTabPage(dbController(), nullptr), tr("Application Signals"));

	m_filesTabPage = new FilesTabPage(dbController(), nullptr);
	m_filesTabPage->setWindowTitle(tr("Files"));
	m_filesTabPageIndex = getCentralWidget()->addTabPage(m_filesTabPage, m_filesTabPage->windowTitle());
	getCentralWidget()->removeTab(m_filesTabPageIndex);	// It will be added in projectOpened slot if required

	m_logicSchema = SchemasTabPage::create<VFrame30::LogicSchema>("als", dbController(), AlFileName, nullptr);
	m_monitorSchema = SchemasTabPage::create<VFrame30::MonitorSchema>("mvs", dbController(), MvsFileName, nullptr);
	//m_diagSchema = SchemasTabPage::create<VFrame30::DiagSchema>("dvs", dbController(), DvsFileName, nullptr);

	getCentralWidget()->addTabPage(m_logicSchema, tr("Application Logic"));
	getCentralWidget()->addTabPage(m_monitorSchema, tr("Monitor Schemas"));
	//getCentralWidget()->addTabPage(m_diagSchema, tr("Diag Schemas"));

	m_buildTabPage = new BuildTabPage(dbController(), nullptr);
	getCentralWidget()->addTabPage(m_buildTabPage, tr("Build"));

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

	m_configuratorAction = new QAction(tr("Module Configurator..."), this);
	m_configuratorAction->setStatusTip(tr("Run module configurator"));
	//m_pConfiguratorAction->setEnabled(true);
	connect(m_configuratorAction, &QAction::triggered, this, &MainWindow::runConfigurator);

	m_afblEditorAction = new QAction(tr("AFBL Editor..."), this);
	m_afblEditorAction->setStatusTip(tr("Run AFBL Editor"));
	m_afblEditorAction->setEnabled(false);
	connect(m_afblEditorAction, &QAction::triggered, this, &MainWindow::runAfblEditor);

    m_subsystemListEditorAction = new QAction(tr("Subsystem List Editor..."), this);
	m_subsystemListEditorAction->setStatusTip(tr("Run Subsystem List Editor"));
	m_subsystemListEditorAction->setEnabled(false);
	connect(m_subsystemListEditorAction, &QAction::triggered, this, &MainWindow::runSubsystemListEditor);

    m_connectionsEditorAction = new QAction(tr("Optical Connections Editor..."), this);
    m_connectionsEditorAction->setStatusTip(tr("Run Optical Connections Editor"));
    m_connectionsEditorAction->setEnabled(false);
    connect(m_connectionsEditorAction, &QAction::triggered, this, &MainWindow::runConnectionsEditor);

	m_rs232SignalListEditorAction = new QAction(tr("RS232/485 Connections Editor..."), this);
	m_rs232SignalListEditorAction->setStatusTip(tr("Run RS232/485 Connections Editor"));
	m_rs232SignalListEditorAction->setEnabled(false);
	connect(m_rs232SignalListEditorAction, &QAction::triggered, this, &MainWindow::runRS232SignalListEditor);

    m_aboutAction = new QAction(tr("About..."), this);
	m_aboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_debugAction = new QAction(tr("Debug..."), this);
	m_debugAction->setStatusTip(tr("Perform some debug actions, don't run it!"));
	m_debugAction->setEnabled(true);
	connect(m_debugAction, &QAction::triggered, this, &MainWindow::debug);

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

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_configuratorAction);
	pToolsMenu->addAction(m_afblEditorAction);
	pToolsMenu->addAction(m_subsystemListEditorAction);
    pToolsMenu->addAction(m_connectionsEditorAction);
	pToolsMenu->addAction(m_rs232SignalListEditorAction);
    pToolsMenu->addSeparator();
	pToolsMenu->addAction(m_settingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_aboutAction);
	pHelpMenu->addAction(m_debugAction);

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

    DialogConnectionsEditor d(dbController(), this);
	d.exec();
}

void MainWindow::runRS232SignalListEditor()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	Rs232SignalListEditor d(dbController(), this);
	d.exec();
}

void MainWindow::showAbout()
{
	QMessageBox aboutDialog;
	aboutDialog.setIconPixmap(QPixmap(":/Images/Images/logo.png"));
	aboutDialog.setText("<h2>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h2>");
	aboutDialog.setInformativeText(qApp->applicationName() + " provides offline tools for FSC chassis configuration, application logic design and its compilation, visualization design and SCADA software configuration.");
	aboutDialog.exec();
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
}

void MainWindow::projectOpened(DbProject project)
{
	setWindowTitle(qApp->applicationName() + QString(" - ") + project.projectName() + QString(" - ") + dbController()->currentUser().username());

	// Action, disable/enable
	//
	assert(m_usersAction != nullptr);

	m_usersAction->setEnabled(true);
    m_afblEditorAction->setEnabled(true);
	m_subsystemListEditorAction->setEnabled(true);
    m_connectionsEditorAction->setEnabled(true);
	m_rs232SignalListEditorAction->setEnabled(true);

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
	m_subsystemListEditorAction->setEnabled(false);
    m_connectionsEditorAction->setEnabled(false);
	m_rs232SignalListEditorAction->setEnabled(false);

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
