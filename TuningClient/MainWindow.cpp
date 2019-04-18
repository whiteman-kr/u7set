#include "Main.h"
#include "MainWindow.h"

#include "DialogFilterEditor.h"

#include <QApplication>
#include <QDesktopWidget>

#include "../lib/Tuning/TuningFilter.h"
#include "../lib/LogFile.h"
#include "../lib/Tuning/TuningLog.h"
#include "../lib/Ui/DialogAlert.h"
#include "../lib/Ui/UiTools.h"
#include "../lib/Ui/DialogAbout.h"

#include "Settings.h"
#include "DialogSettings.h"
#include "TuningClientTcpClient.h"
#include "TuningClientFilterStorage.h"
#include "TuningSchemaManager.h"

QColor redColor = QColor(192, 0, 0);

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_configController(softwareInfo, theSettings.configuratorAddress1(), theSettings.configuratorAddress2(), this)
{
	m_singleLmControlModeText = QObject::tr("Single LM Control Mode");
	m_multipleLmControlModeText = QObject::tr("Multiple LM Control Mode");

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

	theLogFile->writeText("---");
	theLogFile->writeMessage(tr("Application started."));

	m_tuningLog = new TuningLog::TuningLog("TuningClientSignals");

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
	pHelpMenu->addAction(m_pAboutAction);
}

void MainWindow::createStatusBar()
{

	m_statusBarBuildInfo = new QLabel();
	m_statusBarBuildInfo->setAlignment(Qt::AlignLeft);
	m_statusBarBuildInfo->setIndent(3);
	//m_statusBarBuildInfo->setIndent(3);
	//m_statusBarBuildInfo->setText(m_singleLmControlMode ? m_singleLmControlModeText : m_multipleLmControlModeText);

	m_statusBarLmControlMode = new QLabel();
	m_statusBarLmControlMode->setAlignment(Qt::AlignLeft);
	//m_statusBarLmControlMode->setIndent(3);
	m_statusBarLmControlMode->setText(m_singleLmControlMode ? m_singleLmControlModeText : m_multipleLmControlModeText);

	m_statusBarLmErrors = new QLabel();
	m_statusBarLmErrors->setAlignment(Qt::AlignLeft);
	m_statusBarLmErrors->setMinimumWidth(80);
	m_statusBarLmErrors->installEventFilter(this);
	m_statusBarLmErrors->setToolTip(tr("LM Errors (click for details)"));

	m_statusBarSor = new QLabel();
	m_statusBarSor->setAlignment(Qt::AlignLeft);
	m_statusBarSor->setMinimumWidth(80);
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
	statusBar()->addWidget(m_statusBarBuildInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarLmControlMode, 0);
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
		m_filterStorage.updateCounters(&m_tuningSignalManager, m_tcpClient, nullptr);

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

void MainWindow::createWorkspace()
{
	if (m_tuningWorkspace != nullptr || m_schemasWorkspace != nullptr)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Program configuration has been changed and will be updated."));
	}

	// Create main layout

	if (m_mainLayout == nullptr)
	{
		QWidget* w = new QWidget(this);
		m_mainLayout = new QVBoxLayout(w);
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
		setCentralWidget(w);
	}

	// Delete old workspaces

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

	if (m_tabWidget != nullptr)
	{
		delete m_tabWidget;
		m_tabWidget = nullptr;
	}

	createAndCheckFiltersHashes(false/*userFiltersOnly*/);

	// Create new workspaces

	if (theConfigSettings.showSchemas == true && theConfigSettings.schemas.empty() == false)
	{
		m_schemasWorkspace = new SchemasWorkspace(&m_configController, &m_tuningSignalManager, m_tcpClient, this);
	}

	if (theConfigSettings.showSignals == true)
	{
		m_tuningWorkspace = new TuningWorkspace(nullptr, m_filterStorage.root(), &m_tuningSignalManager, m_tcpClient, &m_filterStorage, this);
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

				m_tabWidget = new QTabWidget();
				m_tabWidget->addTab(m_schemasWorkspace, tr("Schemas"));
				m_tabWidget->addTab(m_tuningWorkspace, tr("Signals"));

				m_mainLayout->addWidget(m_tabWidget, 2);
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
	assert(m_tcpClient);

	// BuildInfo

	QString text = tr("Project %1, build %2").arg(theConfigSettings.buildInfo.projectName).arg(theConfigSettings.buildInfo.buildNo);

	if (m_statusBarBuildInfo->text() != text)
	{
		m_statusBarBuildInfo->setText(text);
	}

	// LM Control Mode

	if (m_singleLmControlMode != m_tcpClient->singleLmControlMode() || m_activeClientId != m_tcpClient->activeClientId() || m_activeClientIp != m_tcpClient->activeClientIp())
	{
		m_singleLmControlMode = m_tcpClient->singleLmControlMode();

		m_activeClientId = m_tcpClient->activeClientId();
		m_activeClientIp = m_tcpClient->activeClientIp();

		QString str = m_singleLmControlMode ? m_singleLmControlModeText : m_multipleLmControlModeText;

		if (m_activeClientId.isEmpty() == false && m_activeClientIp.isEmpty() == false)
		{
			str += tr(", active client is %1, %2").arg(m_activeClientId).arg(m_activeClientIp);

			if (m_tcpClient->clientIsActive() == true)
			{
				str += tr(" (current)");
			}
		}

		m_statusBarLmControlMode->setText(str);
	}

	Tcp::ConnectionState configConnState =  m_configController.getConnectionState();
	Tcp::ConnectionState tuningConnState =  m_tcpClient->getConnectionState();

	// ConfigService
	//
	text = tr(" ConfigService: ");

	if (configConnState.isConnected == false)
	{
		text += tr(" no connection");
	}
	else
	{
		text += tr(" connected, packets: %1").arg(QString::number(configConnState.replyCount));
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

	if (tuningConnState.isConnected == false)
	{
		text += tr(" no connection");
	}
	else
	{
		text += tr(" connected, packets: %1").arg(QString::number(tuningConnState.replyCount));
	}

	if (text != m_statusBarTuningConnection->text())
	{
		m_statusBarTuningConnection->setText(text);
	}

	tooltip = m_tcpClient->getStateToolTip();

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

			QString text = tr(" %1 %2 ").arg(f->caption()).arg(counters.discreteCounter);

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

	std::vector<Hash> sources = m_tcpClient->tuningSourcesEquipmentHashes();

	if (sources.empty() == true)
	{
		m_statusBarLmErrors->setText(tr(" No LM information "));
		m_statusBarSor->setText(QString());
	}
	else
	{
		assert(m_statusBarLmErrors);

		// Lm Errors tool

		TuningCounters rootCounters = m_filterStorage.root()->counters();

		if (m_lmErrorsCounter != rootCounters.errorCounter)
		{
			m_lmErrorsCounter = rootCounters.errorCounter;

			m_statusBarLmErrors->setText(tr(" LM Errors: %1 ").arg(m_lmErrorsCounter));

			if (m_lmErrorsCounter == 0)
			{
				if (m_statusBarLmErrors->styleSheet() != "")
				{
					m_statusBarLmErrors->setStyleSheet("");
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

		// Sor tool

		if (theConfigSettings.showSOR == true)
		{
			QString sorStatus;

			if (rootCounters.sorActive == false)
			{
				sorStatus = tr(" SOR: ");
			}
			else
			{
				if (rootCounters.sorValid == false)
				{
					sorStatus = tr(" SOR: ? ");
				}
				else
				{
					if (rootCounters.sorCounter == 0)
					{
						sorStatus = tr(" SOR: No ");
					}
					else
					{
						if (rootCounters.sorCounter == 1)
						{
							sorStatus = tr(" SOR: Yes ");
						}
						else
						{
							sorStatus = tr(" SOR: Yes [%1] ").arg(rootCounters.sorCounter);
						}
					}
				}
			}

			if (m_sorStatus != sorStatus)
			{
				m_sorStatus = sorStatus;

				assert(m_statusBarSor);

				m_statusBarSor->setText(sorStatus);

				if ((rootCounters.sorActive == true && rootCounters.sorValid == false) || rootCounters.sorCounter > 0)
				{
                    m_statusBarSor->setStyleSheet(QString("QLabel {color : white; background-color: %1}").arg(redColor.name()));

				}
				else
				{
					m_statusBarSor->setStyleSheet(m_statusBarLmControlMode->styleSheet());
				}
			}
		}
		else
		{
			m_statusBarSor->setText(QString());
		}
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
            m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: %1}").arg(redColor.name()));
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

	DialogFilterEditor d(&m_tuningSignalManager, m_tcpClient, &editFilters, this);

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
		m_dialogTuningSources = new DialogTuningSources(m_tcpClient, true, this);
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
		m_dialogStatistics = new DialogStatistics(this);
		m_dialogStatistics->show();

		auto f = [this]() -> void
			{
				m_dialogStatistics = nullptr;
			};

		connect(m_dialogStatistics, &DialogStatistics::dialogClosed, this, f);
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

void MainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to modify tuning values.");
	DialogAbout::show(this, text, ":/Images/Images/logo.png");
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

