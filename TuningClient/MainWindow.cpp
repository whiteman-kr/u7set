#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "DialogSettings.h"
#include "TuningFilter.h"
#include "DialogTuningSources.h"
#include "DialogPresetEditor.h"

MainWindow::MainWindow(QWidget *parent) :
	m_configController(this, theSettings.configuratorAddress1(), theSettings.configuratorAddress2()),
	QMainWindow(parent)
{
	if (theSettings.m_mainWindowPos.x() != -1 && theSettings.m_mainWindowPos.y() != -1)
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);
	}

	createActions();
	createMenu();
	createStatusBar();

	setCentralWidget(new QLabel("Waiting for configuration..."));

	// TcpSignalClient
	//
	HostAddressPort fakeAddress(QLatin1String("0.0.0.0"), 0);
	theTcpTuningClient = new TcpTuningClient(&m_configController, fakeAddress, fakeAddress);

	m_tcpClientThread = new SimpleThread(theTcpTuningClient);
	m_tcpClientThread->start();

	connect(theTcpTuningClient, &TcpTuningClient::tuningSourcesArrived, this, &MainWindow::slot_tuningSourcesArrived);
	connect(theTcpTuningClient, &TcpTuningClient::connectionFailed, this, &MainWindow::slot_tuningConnectionFailed);

	connect(this, &MainWindow::filtersUpdated, this, &MainWindow::slot_filtersUpdated);

	QString errorCode;
	if (theUserFilters.load(QString("UserFilters.xml"), &errorCode) == false)
	{
		QMessageBox::critical(this, "Error", tr("Failed to load user filters: %1").arg(errorCode));
	}

	//

	m_updateStatusBarTimerId = startTimer(100);

	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);
	m_configController.start();
}

MainWindow::~MainWindow()
{
	m_tcpClientThread->quitAndWait(10000);
	delete m_tcpClientThread;

	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();


	theFilters.save("ObjectFilters1.xml");
	theUserFilters.save("ObjectFiltersUser1.xml");
}

void MainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	//m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MainWindow::exit);


	m_pPresetEditorAction = new QAction(tr("Preset Editor..."), this);
	m_pPresetEditorAction->setStatusTip(tr("Edit user presets"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pPresetEditorAction->setEnabled(true);
	connect(m_pPresetEditorAction, &QAction::triggered, this, &MainWindow::runPresetEditor);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

	m_pTuningSourcesAction = new QAction(tr("Tuning sources..."), this);
	m_pTuningSourcesAction->setStatusTip(tr("View tuning sources"));
	//m_pTuningSourcesAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pTuningSourcesAction->setEnabled(true);
	connect(m_pTuningSourcesAction, &QAction::triggered, this, &MainWindow::showTuningSources);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
	//connect(m_pLogAction, &QAction::triggered, this, &MonitorMainWindow::showLog);

	m_pAboutAction = new QAction(tr("About..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	//connect(m_pAboutAction, &QAction::triggered, this, &MonitorMainWindow::showAbout);
}

void MainWindow::createMenu()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_pPresetEditorAction);
	pToolsMenu->addAction(m_pTuningSourcesAction);
	pToolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pLogAction);
	pHelpMenu->addAction(m_pAboutAction);
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
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	// Update status bar
	//
	if  (event->timerId() == m_updateStatusBarTimerId && theTcpTuningClient != nullptr)
	{
		assert(m_statusBarConnectionState);
		assert(m_statusBarConnectionStatistics);

		Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();
		Tcp::ConnectionState tuningClientState =  theTcpTuningClient->getConnectionState();

		// State
		//
		QString text = QString(" ConfigSrv: %1   TuningSrv: %2 ")
					   .arg(confiConnState.isConnected ? confiConnState.host.addressStr() : "NoConnection")
						.arg(tuningClientState.isConnected ? tuningClientState.host.addressStr() : "NoConnection");

		m_statusBarConnectionState->setText(text);

		// Statistics
		//
		text = QString(" ConfigSrv: %1   TuningSrv: %2 ")
			   .arg(QString::number(confiConnState.replyCount))
			   .arg(QString::number(tuningClientState.replyCount));

		m_statusBarConnectionStatistics->setText(text);

		return;
	}

	return;
}

void MainWindow::removeWorkspace()
{
	if (m_tuningWorkspace != nullptr)
	{
		QMessageBox::warning(this, "Warning", "Program configuration has been changed and will be updated.");

		delete m_tuningWorkspace;
		m_tuningWorkspace = nullptr;
	}

}

void MainWindow::createWorkspace()
{
	m_tuningWorkspace = new TuningWorkspace(this);
	setCentralWidget(m_tuningWorkspace);
}

void MainWindow::slot_configurationArrived(ConfigSettings settings)
{
	if (settings.updateFilters == false && settings.updateSignals == false && settings.updateSchemas == false)
	{
		return;
	}

	removeWorkspace();

	if (settings.updateFilters == true)
	{
		if (m_configController.getObjectFilters() == false)
		{

		}
	}

	if (settings.updateSchemas == true)
	{
		if (m_configController.getSchemasDetails() == false)
		{

		}
	}

	if (settings.updateSignals == true)
	{
		if (m_configController.getTuningSignals() == false)
		{

		}
		emit signalsUpdated();
	}

	theFilters.createAutomaticFilters();

	createWorkspace();

	return;
}

void MainWindow::slot_filtersUpdated()
{
	removeWorkspace();
	createWorkspace();
	return;
}

void MainWindow::slot_tuningSourcesArrived()
{

}

void MainWindow::slot_tuningConnectionFailed()
{

}

void MainWindow::exit()
{
	close();
}

void MainWindow::showSettings()
{
	DialogSettings d;
	d.exec();
}

void MainWindow::runPresetEditor()
{
	TuningFilterStorage editFilters = theUserFilters;

	DialogPresetEditor d(&editFilters, this);
	if (d.exec() == QDialog::Accepted)
	{
		theUserFilters = editFilters;
		theUserFilters.save("UserFilters.xml");
		emit filtersUpdated();
	}
}

void MainWindow::showTuningSources()
{
	if (theDialogTuningSources == nullptr)
	{
		theDialogTuningSources = new DialogTuningSources(this);
		theDialogTuningSources->show();
	}
	else
	{
		theDialogTuningSources->activateWindow();
	}
}

MainWindow* theMainWindow = nullptr;
