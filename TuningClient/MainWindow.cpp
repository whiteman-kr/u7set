#include "Main.h"
#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "DialogSettings.h"
#include "../lib/Tuning/TuningFilter.h"
#include "DialogTuningSources.h"
#include "DialogFilterEditor.h"
#include "version.h"

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_configController(softwareInfo, theSettings.configuratorAddress1(), theSettings.configuratorAddress2(), this)
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

	//
	//

	theLogFile = new Log::LogFile(qAppName());

	theLogFile->writeText("---");
	theLogFile->writeMessage(tr("Application started."));

	m_tuningLog = new TuningLog::TuningLog(qAppName());

	createActions();
	createMenu();
	createStatusBar();

	setWindowTitle(QString("TuningClient - ") + theSettings.instanceStrId());

	setCentralWidget(new QLabel(tr("Waiting for configuration...")));

	// TuningClientTcpClient
	//
	m_tcpClient = new TuningClientTcpClient(softwareInfo, &m_tuningSignalManager, theLogFile, m_tuningLog, &m_userManager);
	m_tcpClient->setInstanceId(theSettings.instanceStrId());
	m_tcpClient->setRequestInterval(theSettings.m_requestInterval);

#ifdef Q_DEBUG

	if (theSettings.m_simulationMode == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Warning! TuningClient is running in debugging simulation mode!"));
	}

	m_tcpClient->setSimulationMode(theSettings.m_simulationMode);	// For debugging
#endif

	m_tcpClientThread = new SimpleThread(m_tcpClient);
	m_tcpClientThread->start();

	// Global connections

	connect(&m_configController, &ConfigController::tcpClientConfigurationArrived, m_tcpClient, &TuningClientTcpClient::slot_configurationArrived);

	connect(&m_configController, &ConfigController::filtersArrived, this, &MainWindow::slot_projectFiltersUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::schemasDetailsArrived, this, &MainWindow::slot_schemasDetailsUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::signalsArrived, this, &MainWindow::slot_signalsUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);

	connect(&m_configController, &ConfigController::globalScriptArrived, this, &MainWindow::slot_schemasGlobalScriptArrived,
			Qt::QueuedConnection);

	// DialogAlert

	m_dialogAlert = new DialogAlert(this);
	connect(theLogFile, &Log::LogFile::alertArrived, m_dialogAlert, &DialogAlert::onAlertArrived);

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

	delete m_tuningLog;
}

UserManager* MainWindow::userManager()
{
	return &m_userManager;
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
	connect(m_pLogAction, &QAction::triggered, this, &MainWindow::showLog);

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

	m_statusDiscreteCount = new QLabel();
	m_statusDiscreteCount->setAlignment(Qt::AlignLeft);
	m_statusDiscreteCount->setMinimumWidth(100);
	m_statusDiscreteCount->setToolTip(tr("Alerted discretes counter"));

	m_statusBarLmErrors = new QLabel();
	m_statusBarLmErrors->setAlignment(Qt::AlignLeft);
	m_statusBarLmErrors->setMinimumWidth(100);
	m_statusBarLmErrors->installEventFilter(this);
	m_statusBarLmErrors->setToolTip(tr("LM Errors (click for details)"));

	m_statusBarSor = new QLabel();
	m_statusBarSor->setAlignment(Qt::AlignLeft);
	m_statusBarSor->setMinimumWidth(100);
	m_statusBarSor->installEventFilter(this);
	m_statusBarSor->setToolTip(tr("SOR counter (click for details)"));

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignLeft);
	m_statusBarTuningConnection->setMinimumWidth(100);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignLeft);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusDiscreteCount, 0);
	statusBar()->addPermanentWidget(m_statusBarLmErrors, 0);
	statusBar()->addPermanentWidget(m_statusBarSor, 0);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_tuningWorkspace != nullptr && m_tuningWorkspace->hasPendingChanges() == true)
	{
		int result = QMessageBox::warning(this, qAppName(), tr("Warning! Some values were modified but not written. Are you sure you want to exit?"), tr("Yes"), tr("No"));

		if (result == 1)
		{
			event->ignore();
		}
	}

}

void MainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);

	// Update status bar
	//
	if  (event->timerId() == m_mainWindowTimerId_250ms)
	{
		//theLogFile->writeMessage("Timer");

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

		updateStatusBar();

		//

		if (m_tuningWorkspace != nullptr)
		{
			m_tuningWorkspace->onTimer();
		}
	}

	if  (event->timerId() == m_mainWindowTimerId_500ms)
	{
		m_filterStorage.updateCounters(&m_tuningSignalManager, m_tcpClient, nullptr);

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

	m_filterStorage.createAutomaticFilters(&m_tuningSignalManager,
										   theConfigSettings.filterBySchema,
										   theConfigSettings.filterByEquipment,
										   theConfigSettings.showDiscreteCounters,
										   theConfigSettings.equipmentList);

	m_filterStorage.createSignalsAndEqipmentHashes(&m_tuningSignalManager);

	// Find and possibly remove non-existing signals from the list

	bool removedNotFound = false;

	std::vector<std::pair<QString, QString>> notFoundSignalsAndFilters;

	m_filterStorage.checkAndRemoveFilterSignals(m_tuningSignalManager.signalHashes(), removedNotFound, notFoundSignalsAndFilters, this);

	if (removedNotFound == true)
	{
		QString errorMsg;

		if (m_filterStorage.save(theSettings.userFiltersFile(), &errorMsg, TuningFilter::Source::User) == false)
		{
			theLogFile->writeError(errorMsg);
			QMessageBox::critical(this, tr("Error"), errorMsg);
		}
	}

	// Create workspaces
	if (m_mainLayout == nullptr)
	{
		QWidget* w = new QWidget(this);
		m_mainLayout = new QVBoxLayout(w);
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(w);
	}

	if (m_logonWorkspace != nullptr)
	{
		delete m_logonWorkspace;
		m_logonWorkspace = nullptr;
	}

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
		m_tuningWorkspace = new TuningWorkspace(nullptr, m_filterStorage.root(), &m_tuningSignalManager, m_tcpClient, this);
	}

	// Create login workspace

	if (m_userManager.logonMode() == LogonMode::Permanent && m_userManager.users().empty() == false)
	{
		m_logonWorkspace = new LogonWorkspace(&m_userManager, this);

		connect(this, &MainWindow::timerTick500, m_logonWorkspace, &LogonWorkspace::onTimer);

		m_mainLayout->addWidget(m_logonWorkspace);
	}

	// Now choose, what workspace to display. If both exists, create a tab page.

	if (m_schemasWorkspace == nullptr && m_tuningWorkspace != nullptr)
	{
		// Show Tuning Workspace
		//
		m_mainLayout->addWidget(m_tuningWorkspace, 2);
	}
	else
	{
		if (m_schemasWorkspace != nullptr && m_tuningWorkspace == nullptr)
		{
			// Show Schemas Workspace
			//
			m_mainLayout->addWidget(m_schemasWorkspace, 2);
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

				m_mainLayout->addWidget(tab, 2);
			}
			else
			{
				m_mainLayout->addWidget(new QLabel("No workspaces exist, configuration error."));
			}
		}
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_statusBarLmErrors &&
		m_statusBarLmErrors->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showTuningSources();
	}

	if (object == m_statusBarSor &&
		m_statusBarSor->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showTuningSources();
	}

	if (object == m_statusBarLogAlerts &&
		m_statusBarLogAlerts->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showLog();
	}

	return QWidget::eventFilter(object, event);
}

void MainWindow::updateStatusBar()
{
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

	// Counters

	if (theConfigSettings.showDiscreteCounters == true)
	{
		assert(m_statusDiscreteCount);

		if (m_discreteCounter != m_filterStorage.root()->counters().discreteCounter)
		{
			m_discreteCounter = m_filterStorage.root()->counters().discreteCounter;

			m_statusDiscreteCount->setText(QString("Discretes: %1").arg(m_discreteCounter));

			if (m_discreteCounter == 0)
			{
				m_statusDiscreteCount->setStyleSheet(m_statusBarInfo->styleSheet());
			}
			else
			{
				m_statusDiscreteCount->setStyleSheet("QLabel {color : white; background-color: blue}");
			}
		}
	}

	std::vector<Hash> sources = m_tcpClient->tuningSourcesEquipmentHashes();

	if (sources.empty() == true)
	{
		m_statusBarLmErrors->setText(tr(" No LM information"));
		m_statusBarSor->setText(tr(" No SOR information"));
	}
	else
	{
		assert(m_statusBarLmErrors);

		// Lm Errors tool

		int totalErrorsCount = 0;
		int totalSorCount = 0;

		for (Hash& h : sources)
		{
			TuningFilterCounters counters;

			if (m_tcpClient->tuningSourceCounters(h, counters.errorCounter, counters.sorCounter) == false)
			{
				assert(false);
				continue;
			}

			totalErrorsCount += counters.errorCounter;
			totalSorCount += counters.sorCounter;
		}

		if (m_lmErrorsCounter != totalErrorsCount)
		{
			m_lmErrorsCounter = totalErrorsCount;

			m_statusBarLmErrors->setText(QString(" LM Errors: %1").arg(totalErrorsCount));

			if (totalErrorsCount == 0)
			{
				m_statusBarLmErrors->setStyleSheet(m_statusBarInfo->styleSheet());
			}
			else
			{
				m_statusBarLmErrors->setStyleSheet("QLabel {color : white; background-color: red}");
			}
		}

		// Sor tool

		if (m_sorCounter != totalSorCount)
		{
			m_sorCounter = totalSorCount;

			assert(m_statusBarSor);

			m_statusBarSor->setText(QString(" SOR: %1").arg(totalSorCount));

			if (totalSorCount == 0)
			{
				m_statusBarSor->setStyleSheet(m_statusBarInfo->styleSheet());
			}
			else
			{
				m_statusBarSor->setStyleSheet("QLabel {color : white; background-color: red}");
			}
		}
	}

	// Log alerts tool

	if (m_logErrorsCounter != theLogFile->errorAckCounter() || m_logWarningsCounter != theLogFile->warningAckCounter())
	{
		m_logErrorsCounter = theLogFile->errorAckCounter();
		m_logWarningsCounter = theLogFile->warningAckCounter();

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(QString(" Log E: %1 W: %2").arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarInfo->styleSheet());
		}
		else
		{
			m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: red}");
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
	if (m_userManager.login(this) == false)
	{
		return;
	}

	TuningClientFilterStorage editFilters = m_filterStorage;

	DialogFilterEditor d(&m_tuningSignalManager, m_tcpClient, &editFilters, this);

	if (d.exec() == QDialog::Accepted)
	{
		m_filterStorage = editFilters;

		QString errorMsg;

		if (m_filterStorage.save(theSettings.userFiltersFile(), &errorMsg, TuningFilter::Source::User) == false)
		{
			theLogFile->writeError(errorMsg);
			QMessageBox::critical(this, tr("Error"), errorMsg);
		}

		createWorkspace();
	}
}

void MainWindow::showSettings()
{
	if (m_userManager.login(this) == false)
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

void MainWindow::showLog()
{
	if (theLogFile != nullptr)
	{
		theLogFile->view(this);
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

