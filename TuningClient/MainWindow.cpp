#include "Main.h"
#include "MainWindow.h"

#include "DialogFilterEditor.h"

#include <QApplication>
#include <QDesktopWidget>

#include "../lib/Tuning/TuningFilter.h"
#include "../UtilsLib/LogFile.h"
#include "../lib/Tuning/TuningLog.h"
#include "../lib/Ui/DialogAlert.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "../lib/Ui/DialogAbout.h"

#include "Settings.h"
#include "DialogSettings.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"
#include "TuningSchemaManager.h"

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_configController(softwareInfo, theSettings.configuratorAddress1(), theSettings.configuratorAddress2(), this)
{
	m_singleLmControlModeText = QObject::tr("Single LM Control Mode");
	m_multipleLmControlModeText = QObject::tr("Multiple LM Control Mode");
	m_mixedLmControlModeText = QObject::tr("Mixed LM Control Mode");

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

	theLogFile = new Log::LogFile("TuningClient");

	m_tuningLog = new TuningLog::TuningLog("TuningClientSignals");

	createActions();
	createMenu();
	createStatusBar();

	setWindowTitle(QString("TuningClient - ") + theSettings.instanceStrId());

	setCentralWidget(new QLabel(tr("Waiting for configuration...")));

	// Global connections

	connect(&m_configController, &ConfigController::filtersArrived, this, &MainWindow::slot_projectFiltersUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::signalsArrived, this, &MainWindow::slot_signalsUpdated, Qt::DirectConnection);
	connect(&m_configController, &ConfigController::configurationArrived, this, &MainWindow::slot_configurationArrived);

	// DialogAlert

	m_dialogAlert = new DialogAlert(this);
	connect(theLogFile, &Log::LogFile::alertArrived, m_dialogAlert, &DialogAlert::onAlertArrived);
	connect(theLogFile, &Log::LogFile::writeFailure, m_dialogAlert, &DialogAlert::onAlertArrived);

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
	deleteWorkspace();

	stopTcpClients();

	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	delete theLogFile;

	delete m_tuningLog;
}

TuningUserManager* MainWindow::userManager()
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


	m_pPresetEditorAction = new QAction(tr("Filter Editor..."), this);
	m_pPresetEditorAction->setStatusTip(tr("Edit user filters"));
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

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);

	m_pAppLogAction = new QAction(tr("Application Log..."), this);
	m_pAppLogAction->setStatusTip(tr("Show application log"));
	connect(m_pAppLogAction, &QAction::triggered, this, &MainWindow::showAppLog);

	m_pSignalLogAction = new QAction(tr("Signals Log..."), this);
	m_pSignalLogAction->setStatusTip(tr("Show signals log"));
	connect(m_pSignalLogAction, &QAction::triggered, this, &MainWindow::showSignalsLog);

	m_aboutQtAction = new QAction(tr("About Qt..."), this);
	m_aboutQtAction->setStatusTip(tr("Show Qt information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQt);

	m_pAboutAction = new QAction(tr("About TuningClient..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MainWindow::showAbout);

	m_manualTuningAction = new QAction(tr("Tuning User Manual"), this);
	m_manualTuningAction->setStatusTip(tr("Show Tuning User Manual"));
	connect(m_manualTuningAction, &QAction::triggered, this, &MainWindow::showTuningUserManual);

}

void MainWindow::createMenu()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pServiceMenu = menuBar()->addMenu(tr("&Service"));
	pServiceMenu->addAction(m_pPresetEditorAction);
	pServiceMenu->addSeparator();
	pServiceMenu->addAction(m_pSettingsAction);


	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pTuningSourcesAction);
	pHelpMenu->addAction(m_pStatisticsAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_pAppLogAction);
	pHelpMenu->addAction(m_pSignalLogAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_manualTuningAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_aboutQtAction);
	pHelpMenu->addAction(m_pAboutAction);

}

void MainWindow::createStatusBar()
{

	m_statusBarBuildInfo = new QLabel();
	m_statusBarBuildInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarBuildInfo->setIndent(3);

	m_statusBarLmControlMode = new QLabel();
	m_statusBarLmControlMode->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarLmErrors = new QLabel();
	m_statusBarLmErrors->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarLmErrors->setMinimumWidth(80);
	m_statusBarLmErrors->installEventFilter(this);
	m_statusBarLmErrors->setToolTip(tr("LM Errors (click for details)"));

	m_statusBarSor = new QLabel();
	m_statusBarSor->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarSor->setMinimumWidth(80);
	m_statusBarSor->installEventFilter(this);
	m_statusBarSor->setToolTip(tr("SOR counter (click for details)"));

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarTuningConnection->setMinimumWidth(100);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addWidget(m_statusBarBuildInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarLmControlMode, 0);
	statusBar()->addPermanentWidget(m_statusBarLmErrors, 0);
	statusBar()->addPermanentWidget(m_statusBarSor, 0);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
	for (SchemasWorkspace* sw : m_schemasWorkspaces)
	{
		if (sw->isVisible() == true)
		{
			if (event->matches(QKeySequence::ZoomIn))
			{
				sw->zoomIn();
				return;
			}
			if (event->matches(QKeySequence::ZoomOut))
			{
				sw->zoomOut();
				return;
			}
			if (event->key() == Qt::Key_Asterisk && (event->modifiers() & Qt::ControlModifier))
			{
				sw->zoom100();
				return;
			}
			if (event->key() == Qt::Key_Slash && (event->modifiers() & Qt::ControlModifier))
			{
				sw->zoomToFit();
				return;
			}
		}
	}

	QWidget::keyPressEvent(event);

}


void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m_tuningWorkspace != nullptr && m_tuningWorkspace->hasPendingChanges() == true)
	{
		int result = QMessageBox::warning(this, qAppName(), tr("Warning! Some values were modified but not written. Are you sure you want to exit?"), tr("Yes"), tr("No"));

		if (result == QDialog::Accepted)
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
		m_filterStorage.updateCounters(&m_tuningSignalManager, m_tcpClients, nullptr);

		emit timerTick500();
	}

	return;
}

void MainWindow::createAndCheckFiltersHashes(bool userFiltersOnly)
{
	m_filterStorage.createSignalsAndEqipmentHashes(&m_tuningSignalManager, m_tuningSignalManager.signalHashes(), m_filterStorage.root().get(), userFiltersOnly);

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
}

void MainWindow::runTcpClients()
{
	if (m_tcpClients.empty() == false || m_tcpClientThreads.empty() == false)
	{
		Q_ASSERT(m_tcpClients.empty() == true);
		Q_ASSERT(m_tcpClientThreads.empty() == true);
		return;
	}

	for (const TuningClientSettings::TuningService& ts : theConfigSettings.clientSettings.tuningServices)
	{
		// TuningClientTcpClient
		//
		TuningClientTcpClient* client = new TuningClientTcpClient(m_configController.softwareInfo(),
																  ts.tuningServiceID,
																  ts.singleLmControl,
																  &m_tuningSignalManager,
																  theLogFile,
																  m_tuningLog,
																  &m_userManager);
		client->setInstanceId(theSettings.instanceStrId());
		client->setRequestInterval(theSettings.m_requestInterval);

		const HostAddressPort addrPort = HostAddressPort(ts.clientRequestIP, ts.clientRequestPort);

		client->setServers(addrPort, addrPort, true);
		client->setAutoApply(theConfigSettings.clientSettings.autoApply);
		client->setLmStatusFlagMode(theConfigSettings.lmStatusFlagMode());

		SimpleThread* thread = new SimpleThread(client);
		thread->start();

		m_tcpClients.push_back(client);
		m_tcpClientThreads.push_back(thread);
	}

	if (m_dialogTuningSources != nullptr)
	{
		m_dialogTuningSources->setTuningSources({m_tcpClients.begin(), m_tcpClients.end()});
	}
}

void MainWindow::stopTcpClients()
{
	if (m_noWorkspaceLabel != nullptr ||
		m_logonWorkspace != nullptr ||
		m_tuningWorkspace != nullptr ||
		m_schemasWorkspaces.empty() == false ||
		m_tabWidget != nullptr)
	{
		// We should not delete TCP clients if workspace exists!
		//
		Q_ASSERT(m_noWorkspaceLabel == nullptr);
		Q_ASSERT(m_logonWorkspace == nullptr);
		Q_ASSERT(m_tuningWorkspace == nullptr);
		Q_ASSERT(m_schemasWorkspaces.empty() == true);
		Q_ASSERT(m_tabWidget == nullptr);
		return;
	}

	if (m_dialogTuningSources != nullptr)
	{
		m_dialogTuningSources->setTuningSources({});
	}

	for (SimpleThread* t : m_tcpClientThreads)
	{
		t->quitAndWait(10000);
		delete t;
	}

	m_tcpClients.clear();
	m_tcpClientThreads.clear();
}

void MainWindow::createWorkspace()
{
	// Check if previous workspace is deleted

	if (m_noWorkspaceLabel != nullptr ||
		m_logonWorkspace != nullptr ||
		m_tuningWorkspace != nullptr ||
		m_schemasWorkspaces.empty() == false ||
		m_tabWidget != nullptr)
	{
		Q_ASSERT(m_noWorkspaceLabel == nullptr);
		Q_ASSERT(m_logonWorkspace == nullptr);
		Q_ASSERT(m_tuningWorkspace == nullptr);
		Q_ASSERT(m_schemasWorkspaces.empty() == true);
		Q_ASSERT(m_tabWidget == nullptr);
		return;
	}

	// Create main layout

	if (m_mainLayout == nullptr)
	{
		QWidget* w = new QWidget(this);
		m_mainLayout = new QVBoxLayout(w);
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(w);
	}

	createAndCheckFiltersHashes(false/*userFiltersOnly*/);

	// Create new workspaces

	if (theConfigSettings.clientSettings.showSchemas == true && theConfigSettings.schemas.empty() == false)
	{
		bool schemaFiltersFound = false;

		std::shared_ptr<TuningFilter> rootFilter = m_filterStorage.root();
		if (rootFilter == nullptr)
		{
			Q_ASSERT(rootFilter);
			return;
		}

		std::vector<ITuningTcpClient*> clientInterfaces;
		for (TuningClientTcpClient* client : m_tcpClients)
		{
			ITuningTcpClient* ic = dynamic_cast<ITuningTcpClient*>(client);
			if (ic == nullptr)
			{
				Q_ASSERT(ic);
				continue;
			}

			clientInterfaces.push_back(ic);
		}

		int count = rootFilter->childFiltersCount();
		for (int i = 0; i < count; i++)
		{
			std::shared_ptr<TuningFilter> childFilter = rootFilter->childFilter(i);
			if (childFilter == nullptr)
			{
				Q_ASSERT(childFilter);
				continue;
			}

			if (childFilter->isSchemasTab() == true)
			{
				schemaFiltersFound = true;

				SchemasWorkspace* sw = new SchemasWorkspace(&m_configController, &m_tuningSignalManager, clientInterfaces,
															childFilter->caption(),
															childFilter->tagsList(),
															childFilter->startSchemaId(),
															theLogFile,
															this);
				m_schemasWorkspaces.push_back(sw);
			}
		}

		if (schemaFiltersFound == false)
		{
			SchemasWorkspace* sw = new SchemasWorkspace(&m_configController, &m_tuningSignalManager, clientInterfaces,
														tr("Schemas"),
														{},
														theConfigSettings.clientSettings.startSchemaID,
														theLogFile,
														this);
			m_schemasWorkspaces.push_back(sw);
		}
	}

	if (theConfigSettings.clientSettings.showSignals == true)
	{
		m_tuningWorkspace = new TuningWorkspace(nullptr, m_filterStorage.root(), &m_tuningSignalManager, m_tcpClients, &m_filterStorage, this);
	}

	// Create login workspace

	if (m_userManager.loginPerOperation() == false && m_userManager.tuningUserAccounts().empty() == false)
	{
		m_logonWorkspace = new LogonWorkspace(&m_userManager, this);

		connect(this, &MainWindow::timerTick500, m_logonWorkspace, &LogonWorkspace::onTimer);

		m_mainLayout->addWidget(m_logonWorkspace);
	}

	// Now choose, what workspace to display. If both exists, create a tab page.

	if (m_schemasWorkspaces.empty() == true && m_tuningWorkspace == nullptr)
	{
		m_noWorkspaceLabel = new QLabel(tr("No workspaces exist, configuration error."));
		m_mainLayout->addWidget(m_noWorkspaceLabel);
	}
	else
	{
		if (m_schemasWorkspaces.empty() == true && m_tuningWorkspace != nullptr)
		{
			// Show One Tuning Workspace
			//
			m_mainLayout->addWidget(m_tuningWorkspace, 2);
		}
		else
		{
			if (m_schemasWorkspaces.size() == 1 && m_tuningWorkspace == nullptr)
			{
				// Show One Schemas Workspace
				//
				m_mainLayout->addWidget(m_schemasWorkspaces[0], 2);
			}
			else
			{
				// Show both Workspaces
				//
				m_tabWidget = new QTabWidget();

				for (SchemasWorkspace* sw : m_schemasWorkspaces)
				{
					m_tabWidget->addTab(sw, sw->caption());
				}

				if (m_tuningWorkspace != nullptr)
				{
					m_tabWidget->addTab(m_tuningWorkspace, tr("Signals"));
				}

				m_mainLayout->addWidget(m_tabWidget, 2);
			}
		}
	}
}

void MainWindow::deleteWorkspace()
{
	// Delete old workspaces

	if (m_noWorkspaceLabel != nullptr)
	{
		delete m_noWorkspaceLabel;
		m_noWorkspaceLabel = nullptr;
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

	if (m_schemasWorkspaces.empty() == false)
	{
		for (SchemasWorkspace* sw : m_schemasWorkspaces)
		{
			delete sw;
		}
		m_schemasWorkspaces.clear();
	}

	if (m_tabWidget != nullptr)
	{
		delete m_tabWidget;
		m_tabWidget = nullptr;
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
		showAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void MainWindow::updateStatusBar()
{
	// Status bar
	//
	assert(m_statusBarLmControlMode);
	assert(m_statusBarConfigConnection);

	// BuildInfo

	QString text = tr("Project %1, build %2").arg(theConfigSettings.buildInfo.projectName).arg(theConfigSettings.buildInfo.buildNo);

	if (m_statusBarBuildInfo->text() != text)
	{
		m_statusBarBuildInfo->setText(text);
	}

	// LM Control Mode Label

	{
		int singleLmControlModeCount = 0;

		for (const TuningClientTcpClient* client: m_tcpClients)
		{
			if (client->singleLmControlMode() == true)
			{
				singleLmControlModeCount++;
			}
		}

		QString str = m_multipleLmControlModeText;

		if (m_tcpClients.empty() == false)
		{
			if (singleLmControlModeCount == static_cast<int>(m_tcpClients.size()))
			{
				str = m_singleLmControlModeText;
			}
			else
			{
				if (singleLmControlModeCount > 0)
				{
					str = m_mixedLmControlModeText;
				}
			}
		}

		if (m_statusBarLmControlMode->text() != str)
		{
			m_statusBarLmControlMode->setText(str);
		}
	}

	// LM Control Tooltip

	{
		QString str;

		for (const TuningClientTcpClient* client: m_tcpClients)
		{
			str += tr("%1: ").arg(client->tuningServiceId());

			QString activeClientId = client->activeClientId();
			QString activeClientIp = client->activeClientIp();

			if (activeClientId.isEmpty() == false && activeClientIp.isEmpty() == false)
			{
				str += tr("active client is %1, %2").arg(activeClientId).arg(activeClientIp);

				if (client->clientIsActive() == true)
				{
					str += tr(" (current)");
				}
			}
			else
			{
				str += tr("active");
			}

			str += "\n";
		}

		str = str.trimmed();

		if (m_statusBarLmControlMode->toolTip() != str)
		{
			m_statusBarLmControlMode->setToolTip(str);
		}
	}


	// ConfigService
	//
	Tcp::ConnectionState configConnState =  m_configController.getConnectionState();

	text = tr(" ConfigService: ");

	if (configConnState.isConnected == false)
	{
		text += tr(" no connection");
	}
	else
	{
		text += tr("%1").arg(QString::number(configConnState.replyCount));
	}

	if (text != m_statusBarConfigConnection->text())
	{
		m_statusBarConfigConnection->setText(text);
	}

	QString tooltip = m_configController.getStateToolTip();

	if (tooltip != m_statusBarConfigConnection->toolTip())
	{
		m_statusBarConfigConnection->setToolTip(tooltip);
	}

	// TuningService
	//

	text = tr(" TuningService: ");

	tooltip.clear();

	for (const TuningClientTcpClient* client: m_tcpClients)
	{
		Tcp::ConnectionState tuningConnState =  client->getConnectionState();

		if (tuningConnState.isConnected == true)
		{
			text += tr(" %1 /").arg(QString::number(tuningConnState.replyCount));
		}
		else
		{
			if (m_tcpClients.size() > 1)
			{
				text += tr(" No /");
			}
			else
			{
				text += tr(" No connection");
			}
		}

		tooltip += client->getStateToolTip() + "\n\n";
	}

	text.remove(text.length() - 1, 1);	// remove last "/"

	tooltip = tooltip.trimmed();

	if (text != m_statusBarTuningConnection->text())
	{
		m_statusBarTuningConnection->setText(text);
	}

	if (tooltip != m_statusBarTuningConnection->toolTip())
	{
		m_statusBarTuningConnection->setToolTip(tooltip);
	}

	// Counters

	{
		int labelCount = 0;

		int filtersCount = m_filterStorage.root()->childFiltersCount();
		for (int i = 0; i < filtersCount; i++)
		{
			TuningFilter* f = m_filterStorage.root()->childFilter(i).get();
			if (f == nullptr)
			{
				Q_ASSERT(f);
				return;
			}
			if (f->isCounter() == true && f->counterType() == TuningFilter::CounterType::StatusBar)
			{
				labelCount++;
			}
		}

		if (static_cast<int>(m_statusDiscreteCount.size()) != labelCount)
		{
			// Counters count changed, recreate labels

			for (QLabel* l : m_statusDiscreteCount)
			{
				delete l;
			}
			m_statusDiscreteCount.clear();

			for (int i = 0; i < labelCount; i++)
			{
				QLabel* l = new QLabel();
				l->setAlignment(Qt::AlignLeft);
				l->setMinimumWidth(80);
				l->setToolTip(tr("Counter %1").arg(i));

				statusBar()->insertPermanentWidget(2 + i, l, 0);

				m_statusDiscreteCount.push_back(l);
			}
		}

		int labelIndex = 0;

		for (int i = 0; i < filtersCount; i++)
		{
			TuningFilter* f = m_filterStorage.root()->childFilter(i).get();
			if (f == nullptr)
			{
				Q_ASSERT(f);
				return;
			}

			if (f->isCounter() == false || f->counterType() != TuningFilter::CounterType::StatusBar)
			{
				continue;
			}

			TuningCounters counters = f->counters();

			if (static_cast<int>(m_statusDiscreteCount.size()) < labelIndex)
			{
				Q_ASSERT(false);
				return;
			}

			QLabel* l = m_statusDiscreteCount[labelIndex++];
			if (l == nullptr)
			{
				Q_ASSERT(l);
				return;
			}

			text = tr(" %1 %2 ").arg(f->caption()).arg(counters.discreteCounter);

			if (l->text() != text)
			{
				l->setText(text);
			}

			if (counters.discreteCounter == 0)
			{
				if (l->styleSheet() != "")
				{
					l->setStyleSheet("");
				}
			}
			else
			{
				QString styleSheet = QString("QLabel {background-color : %1; color: %2}").arg(f->backAlertedColor().name()).arg(f->textAlertedColor().name());

				if (l->styleSheet() != styleSheet)
				{
					l->setStyleSheet(styleSheet);
				}
			}
		}
	}

	// LM Errors

	TuningCounters rootCounters = m_filterStorage.root()->counters();

	{
		assert(m_statusBarLmErrors);

		// Lm Errors tool

		text = tr(" LM Errors: %1 ").arg(rootCounters.errorCounter);

		if (m_statusBarLmErrors->text() != text)
		{
			m_statusBarLmErrors->setText(text);
		}

		if (rootCounters.errorCounter == 0)
		{
			if (m_statusBarLmErrors->styleSheet() != "")
			{
				m_statusBarLmErrors->setStyleSheet(QString());
			}
		}
		else
		{
			QString styleSheet = QString("QLabel {color : white; background-color: %1}").arg(redColor.name());

			if (m_statusBarLmErrors->styleSheet() != styleSheet)
			{
				m_statusBarLmErrors->setStyleSheet(styleSheet);
			}
		}

	}

	// SOR counter

	if (theConfigSettings.lmStatusFlagMode() == LmStatusFlagMode::SOR)
	{
		if (rootCounters.sorActive == false)
		{
			text = tr(" SOR: -");
		}
		else
		{
			if (rootCounters.sorValid == false)
			{
				text = tr(" SOR: ? ");
			}
			else
			{
				if (rootCounters.sorCounter == 0)
				{
					text = tr(" SOR: No ");
				}
				else
				{
					if (rootCounters.sorCounter == 1)
					{
						text = tr(" SOR: Yes ");
					}
					else
					{
						text = tr(" SOR: Yes [%1] ").arg(rootCounters.sorCounter);
					}
				}
			}
		}

		if (m_statusBarSor->text() != text)
		{
			assert(m_statusBarSor);

			m_statusBarSor->setText(text);

			QString stylesheet;

			if ((rootCounters.sorActive == true && rootCounters.sorValid == false) || rootCounters.sorCounter > 0)
			{
				stylesheet = QString("QLabel {color : white; background-color: %1}").arg(redColor.name());
			}

			if (m_statusBarSor->styleSheet() != stylesheet)
			{
				m_statusBarSor->setStyleSheet(stylesheet);
			}
		}
	}
	else
	{
		m_statusBarSor->setText(QString());
		m_statusBarSor->setStyleSheet(QString());
	}

	// Log alerts tool

	if (m_logErrorsCounter != theLogFile->errorAckCounter() || m_logWarningsCounter != theLogFile->warningAckCounter())
	{
		m_logErrorsCounter = theLogFile->errorAckCounter();
		m_logWarningsCounter = theLogFile->warningAckCounter();

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(QString(" Log E: %1 W: %2 ").arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarLmControlMode->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: %1}").arg(redColor.name()));
			}
		}
	}
}

void MainWindow::slot_configurationArrived()
{
	QWidget* wm = QApplication::activeModalWidget();
	QWidget* wp = QApplication::activePopupWidget();
	if (wm != nullptr || wp != nullptr)
	{
		// Some modal or popup window is active, so deleting workspace is not available.
		// In this case we need to restart the application.
		//
		QMessageBox::warning(this, tr("Warning"), tr("Program configuraton has been changed. Press OK to restart the program."));

		// Remove the shared memory single application object
		//
		delete theSharedMemorySingleApp;
		theSharedMemorySingleApp = nullptr;

		// Restart the program
		//
		QString program = qApp->arguments()[0];
		QStringList arguments = qApp->arguments().mid(1); // remove the 1st argument - the program name
		qApp->quit();
		QProcess::startDetached(program, arguments);

		return;
	}

	if (m_tuningWorkspace != nullptr || m_schemasWorkspaces.empty() == false)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Program configuration has been changed and will be updated."));
	}

	deleteWorkspace();

	stopTcpClients();

	runTcpClients();

	createWorkspace();

	return;
}

void MainWindow::slot_projectFiltersUpdated(QByteArray data)
{
	QString errorStr;


	m_filterStorage.removeFilters(TuningFilter::Source::Project);
	m_filterStorage.removeFilters(TuningFilter::Source::Schema);
	m_filterStorage.removeFilters(TuningFilter::Source::Equipment);

	if (m_filterStorage.load(data, &errorStr) == false)
	{
		QString completeErrorMessage = QObject::tr("Object Filters file loading error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
	}

	m_filterStorage.createSchemaCounterFilters();


}

void MainWindow::slot_signalsUpdated(QByteArray data)
{
	if (m_tuningSignalManager.load(data) == false)
	{
		QString completeErrorMessage = QObject::tr("Tuning signals file loading error.");
		theLogFile->writeError(completeErrorMessage);
	}
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

	DialogFilterEditor d(&m_tuningSignalManager, &editFilters, this);

	if (d.exec() == QDialog::Accepted)
	{
		//m_filterStorage = editFilters;  // This is not allowed, we need to keep shared pointers to existing non-user filters

		// Delete user filters from main storage

		m_filterStorage.removeFilters(TuningFilter::Source::User);

		// Add user filters from editing storage

		for (int i = 0; i < editFilters.root()->childFiltersCount(); i++)
		{
			std::shared_ptr<TuningFilter> child = editFilters.root()->childFilter(i);

			if (child == nullptr)
			{
				Q_ASSERT(child);
				return;
			}

			if (child->source() != TuningFilter::Source::User)
			{
				continue;
			}

			m_filterStorage.add(child, false);
		}

		QString errorMsg;

		if (m_filterStorage.save(theSettings.userFiltersFile(), &errorMsg, TuningFilter::Source::User) == false)
		{
			theLogFile->writeError(errorMsg);
			QMessageBox::critical(this, tr("Error"), errorMsg);
		}

		slot_userFiltersChanged();
	}
}

void MainWindow::showSettings()
{
	DialogSettings* d = new DialogSettings(this);

	d->exec();

	delete d;
}


void MainWindow::showTuningSources()
{
	if (m_dialogTuningSources == nullptr)
	{
		std::vector<TuningTcpClient*> clients;
		for (const auto& c: m_tcpClients)
		{
			TuningTcpClient* tc = dynamic_cast<TuningTcpClient*>(c);
			if (tc == nullptr)
			{
				Q_ASSERT(tc);
				return;
			}

			clients.push_back(tc);
		}

		m_dialogTuningSources = new DialogTuningSources(clients, true, this);
		m_dialogTuningSources->show();

		auto f = [this]() -> void
		{
			m_dialogTuningSources = nullptr;
		};

		connect(m_dialogTuningSources, &DialogTuningSources::dialogClosed, this, f);

	}
	else
	{
		m_dialogTuningSources->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogTuningSources);
}

void MainWindow::showStatistics()
{
	if (m_dialogStatistics == nullptr)
	{
		m_dialogStatistics = new DialogTcpStatistics(this);
		m_dialogStatistics->show();

		auto f = [this]() -> void
		{
			m_dialogStatistics = nullptr;
		};

		connect(m_dialogStatistics, &DialogTcpStatistics::dialogClosed, this, f);
	}
	else
	{
		m_dialogStatistics->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogStatistics);
}

void MainWindow::showAppLog()
{
	if (theLogFile != nullptr)
	{
		theLogFile->view(this);
	}
}

void MainWindow::showSignalsLog()
{
	if (m_tuningLog != nullptr)
	{
		m_tuningLog->viewSignalsLog(this);
	}
}

void MainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void MainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to modify tuning values.");
	DialogAbout::show(this, text, ":/Images/Images/logo.png");
}

void MainWindow::showTuningUserManual()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.9_FSC_Tuning_User_Manual.pdf", this);
}

void MainWindow::slot_userFiltersChanged()
{
	// Update user filters

	createAndCheckFiltersHashes(true/*userFiltersOnly*/);

	if (m_tuningWorkspace != nullptr)
	{
		m_tuningWorkspace->updateFilters(m_filterStorage.root());
	}

}

MainWindow* theMainWindow = nullptr;
Log::LogFile* theLogFile = nullptr;

