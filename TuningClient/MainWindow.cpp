#include "Main.h"
#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "DialogSettings.h"
#include "../lib/Tuning/TuningFilter.h"
#include "DialogTuningSources.h"
#include "DialogUsers.h"
#include "DialogFilterEditor.h"
#include "version.h"

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	m_configController(this, theSettings.configuratorAddress1(), theSettings.configuratorAddress2())
{
	if (theSettings.m_mainWindowPos.x() != -1 && theSettings.m_mainWindowPos.y() != -1)
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);
	}
	else
	{
		resize(1024, 768);
	}

	theLogFile = new Log::LogFile("TuningClient");

	theLogFile->writeText("---");
	theLogFile->writeMessage(tr("Application started."));

	createActions();
	createMenu();
	createStatusBar();

	setWindowTitle(QString("TuningClient - ") + theSettings.instanceStrId());

	setCentralWidget(new QLabel(tr("Waiting for configuration...")));

	// TuningClientTcpClient
	//
	m_tcpClient = new TuningClientTcpClient(theSettings.instanceStrId(), 1, 0, USED_SERVER_COMMIT_NUMBER, &m_tuningSignalManager);
	m_tcpClient->setInstanceId(theSettings.instanceStrId());
	m_tcpClient->setRequestInterval(theSettings.m_requestInterval);

#ifdef Q_DEBUG
	m_tcpClient->setSimulationMode(true);	// For debugging
#endif

	m_tcpClientThread = new SimpleThread(m_tcpClient);
	m_tcpClientThread->start();

	// Global connections

	connect(&m_configController, &ConfigController::serversArrived, m_tcpClient, &TuningClientTcpClient::slot_serversArrived);

	connect(&m_configController, &ConfigController::filtersArrived, this, &MainWindow::slot_projectFiltersUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::schemasDetailsArrived, this, &MainWindow::slot_schemasDetailsUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::signalsArrived, this, &MainWindow::slot_signalsUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);

	connect(&m_configController, &ConfigController::globalScriptArrived, this, &MainWindow::slot_schemasGlobalScriptArrived,
			Qt::QueuedConnection);

	// Load user filters

	QString errorCode;

	if (m_filterStorage.load(theSettings.userFiltersFile(), &errorCode) == false)
	{
		QString msg = tr("Failed to load user filters: %1").arg(errorCode);

		theLogFile->writeError(msg);
		QMessageBox::critical(this, tr("Error"), msg);
	}

	//

	m_mainWindowTimerId_250ms = startTimer(250);

	m_mainWindowTimerId_500ms = startTimer(500);

	m_configController.start();
}

MainWindow::~MainWindow()
{

	m_tcpClientThread->quitAndWait(10000);
	delete m_tcpClientThread;

	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theLogFile->writeMessage(tr("Application finished."));

	delete theLogFile;
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

	m_pUsersAction = new QAction(tr("Users..."), this);
	m_pUsersAction->setStatusTip(tr("Edit users"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pUsersAction->setEnabled(true);
	connect(m_pUsersAction, &QAction::triggered, this, &MainWindow::runUsersEditor);

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
	connect(m_pAboutAction, &QAction::triggered, this, &MainWindow::showAbout);
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

	QMenu* pServiceMenu = menuBar()->addMenu(tr("&Service"));
	pServiceMenu->addAction(m_pTuningSourcesAction);
	pServiceMenu->addAction(m_pUsersAction);
	pServiceMenu->addAction(m_pSettingsAction);

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

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignHCenter);
	m_statusBarConfigConnection->setMinimumWidth(100);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignHCenter);
	m_statusBarTuningConnection->setMinimumWidth(100);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	// Update status bar
	//
	if  (event->timerId() == m_mainWindowTimerId_250ms)
	{
		if (theSharedMemorySingleApp != nullptr)
		{
			bool ok = theSharedMemorySingleApp->lock();
			if (ok == true)
			{
				TuningClientSharedData* data = (TuningClientSharedData*)theSharedMemorySingleApp->data();

				if (data->showCommand == true)
				{
					data->showCommand = false;

					showMinimized(); // This is to bring up the window if not minimized but beneath some other window
					setWindowState(Qt::WindowActive);
					showNormal();
				}


				ok = theSharedMemorySingleApp->unlock();
				if (ok == false)
				{
					qDebug() << "Failed to unlock QSharedMemory object!";
					assert(false);
				}
			}
			else
			{
				qDebug() << "Failed to lock QSharedMemory object!";
				assert(false);
			}
		}


		// Status bar
		//
		assert(m_statusBarConfigConnection);
		assert(m_statusBarTuningConnection);

		Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();
		Tcp::ConnectionState tuningConnState =  m_tcpClient->getConnectionState();


		// ConfigService
		//
		QString text = tr(" ConfigService: ");
		if (confiConnState.isConnected == false)
		{
			text += tr(" no connection");
		}
		else
		{
			text += tr(" connected, packets: %1").arg(QString::number(confiConnState.replyCount));
		}

		m_statusBarConfigConnection->setText(text);
		m_statusBarConfigConnection->setToolTip(m_configController.getStateToolTip());

		// TuningService
		//
		text = tr(" TuningService: ");
		if (tuningConnState.isConnected == false)
		{
			text += tr(" no connection");
		}
		else
		{
			text += tr(" connected, packets: %1").arg(QString::number(tuningConnState.replyCount));
		}

		m_statusBarTuningConnection->setText(text);
		return;
	}

	if  (event->timerId() == m_mainWindowTimerId_500ms)
	{
		emit timerTick500();
	}

	return;
}

void MainWindow::createWorkspace()
{
	if (m_tuningWorkspace != nullptr || m_schemasWorkspace != nullptr)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Program configuration has been changed and will be updated."));
	}

	// Update automatic filters

	m_filterStorage.removeFilters(TuningFilter::Source::Schema);
	m_filterStorage.removeFilters(TuningFilter::Source::Equipment);

	m_filterStorage.createAutomaticFilters(&m_tuningSignalManager, theConfigSettings.filterBySchema, theConfigSettings.filterByEquipment, theConfigSettings.equipmentList);


	// Find and possibly remove non-existing signals from the list

	bool removedNotFound = false;

	std::vector<std::pair<QString, QString>> notFoundSignalsAndFilters;

	m_filterStorage.checkAndRemoveFilterSignals(m_tuningSignalManager.signalHashes(), removedNotFound, notFoundSignalsAndFilters, this);

	if (removedNotFound == true)
	{
		QString errorMsg;

		if (m_filterStorage.save(theSettings.userFiltersFile(), &errorMsg) == false)
		{
			theLogFile->writeError(errorMsg);
			QMessageBox::critical(this, tr("Error"), errorMsg);
		}
	}

	// Create workspaces

	if (m_tuningWorkspace != nullptr)
	{
		delete m_tuningWorkspace;
		m_tuningWorkspace = nullptr;
	}

	if (m_schemasWorkspace != nullptr)
	{
		delete m_schemasWorkspace;
		m_schemasWorkspace = nullptr;
	}

	if (theConfigSettings.showSchemas == true && theConfigSettings.schemas.empty() == false)
	{
		m_schemasWorkspace = new SchemasWorkspace(&m_configController, &m_tuningSignalManager, m_tcpClient, m_globalScript, this);
	}

	if (theConfigSettings.showSignals == true)
	{
		m_tuningWorkspace = new TuningWorkspace(nullptr, m_filterStorage.m_root, &m_tuningSignalManager, m_tcpClient, this);
	}

	// Now choose, what workspace to display. If both exists, create a tab page.

	if (m_schemasWorkspace == nullptr && m_tuningWorkspace != nullptr)
	{
		// Show Tuning Workspace
		//
		setCentralWidget(m_tuningWorkspace);
	}
	else
	{
		if (m_schemasWorkspace != nullptr && m_tuningWorkspace == nullptr)
		{
			// Show Schemas Workspace
			//
			setCentralWidget(m_schemasWorkspace);
		}
		else
		{
			if (m_schemasWorkspace != nullptr && m_tuningWorkspace != nullptr)
			{
				// Show both Workspaces
				//

				QTabWidget* tab = new QTabWidget();
				tab->addTab(m_schemasWorkspace, tr("Schemas"));
				tab->addTab(m_tuningWorkspace, tr("Signals"));

				//tab->setStyleSheet("QTabWidget::tab-bar{alignment:center; }");

				setCentralWidget(tab);
			}
			else
			{
				setCentralWidget(new QLabel("No workspaces exist, configuration error."));
			}
		}
	}

}

void MainWindow::slot_configurationArrived()
{
	createWorkspace();

	return;
}

void MainWindow::slot_projectFiltersUpdated(QByteArray data)
{
	QString errorStr;


	m_filterStorage.removeFilters(TuningFilter::Source::Project);

	if (m_filterStorage.load(data, &errorStr) == false)
	{
		QString completeErrorMessage = QObject::tr("Object Filters file loading error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
	}

}

void MainWindow::slot_schemasDetailsUpdated(QByteArray data)
{
	QString errorStr;

	if (m_filterStorage.loadSchemasDetails(data, &errorStr) == false)
	{
		QString completeErrorMessage = QObject::tr("Schemas Details file loading error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
	}
}

void MainWindow::slot_signalsUpdated(QByteArray data)
{
	if (m_tuningSignalManager.load(data) == false)
	{
		QString completeErrorMessage = QObject::tr("Tuning signals file loading error.");
		theLogFile->writeError(completeErrorMessage);
	}
}

void MainWindow::slot_schemasGlobalScriptArrived(QByteArray data)
{
	m_globalScript = data.toStdString().c_str();
}

void MainWindow::exit()
{
	close();
}

void MainWindow::runPresetEditor()
{
	if (theUserManager.requestPassword(this, false) == false)
	{
		return;
	}

	TuningClientFilterStorage editFilters = m_filterStorage;

	DialogFilterEditor d(&m_tuningSignalManager, m_tcpClient, &editFilters, this);

	if (d.exec() == QDialog::Accepted)
	{
		m_filterStorage = editFilters;

		QString errorMsg;

		if (m_filterStorage.save(theSettings.userFiltersFile(), &errorMsg) == false)
		{
			theLogFile->writeError(errorMsg);
			QMessageBox::critical(this, tr("Error"), errorMsg);
		}

		createWorkspace();
	}
}

void MainWindow::runUsersEditor()
{
	if (theUserManager.requestPassword(this, true) == false)
	{
		return;
	}

	DialogUsers d(theUserManager, this);
	if (d.exec() == QDialog::Accepted && theSettings.admin() == true)
	{
		theUserManager = d.m_userManager;
		theUserManager.Store();
	}
}

void MainWindow::showSettings()
{
	if (theUserManager.requestPassword(this, true) == false)
	{
		return;
	}

	DialogSettings* d = new DialogSettings(this);

	d->exec();

	delete d;
}


void MainWindow::showTuningSources()
{
	if (theDialogTuningSources == nullptr)
	{
		theDialogTuningSources = new DialogTuningSources(m_tcpClient, this);
		theDialogTuningSources->show();
	}
	else
	{
		theDialogTuningSources->activateWindow();
	}
}

void MainWindow::showAbout()
{
	QDialog aboutDialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	QHBoxLayout* hl = new QHBoxLayout;

	/*QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(":/Images/Images/logo.png"));

	hl->addWidget(logo);*/

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";
#ifndef Q_DEBUG
	text += "Build: Release";
#else
	text += "Build: Debug";
#endif
	text += "<br>Commit SHA1: " USED_SERVER_COMMIT_SHA;

	QLabel* label = new QLabel(text, &aboutDialog);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vl->addWidget(label);

	label = new QLabel(&aboutDialog);
	label->setText(qApp->applicationName() + " allows user to modify tuning values.");
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

MainWindow* theMainWindow = nullptr;
Log::LogFile* theLogFile = nullptr;

UserManager theUserManager;
