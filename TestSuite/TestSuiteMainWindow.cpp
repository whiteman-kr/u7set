#include "TestSuiteMainWindow.h"
#include "AppConfigSettings.h"
#include "TestSuiteDialogSettings.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "../lib/Ui/TabWidgetEx.h"
#include "../lib/Ui/DialogAbout.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "TestLogTabPage.h"
#include "TestViewTabPage.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

TestSuiteMainWindow::TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget *parent)
	: QMainWindow(parent),
	m_appLog(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + theSettings.librarySettings().instanceStrId()),
	m_configController(softwareInfo, theSettings.librarySettings().configuratorAddress1(), theSettings.librarySettings().configuratorAddress2(), &m_appLog),
	m_testSuite(softwareInfo, theSettings.librarySettings(), &m_appLog, &m_testLogOutput),
	m_dialogAlert(this)

{
	setWindowFlags(Qt::Widget);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	m_tabWidget = new TabWidgetEx{this};
	m_tabWidget->setDocumentMode(false);
	m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tabWidget, &TabWidgetEx::tabCloseRequested, this, &TestSuiteMainWindow::onTabCloseRequested);

	setCentralWidget(m_tabWidget);
	centralWidget()->setAutoFillBackground(true);

	QHBoxLayout* layout = new QHBoxLayout;
	centralWidget()->setLayout(layout);

	auto margins = layout->contentsMargins();
	margins.setTop(0);
	layout->setContentsMargins(margins);

	m_testLogTabPage = new TestLogTabPage(m_testLogOutput, this);
	m_tabWidget->addTab(m_testLogTabPage, "Test Log");
	m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, 0);	// This tab is not closable

	m_testLogOutput.setHtmlFont("Verdana");

	// Create UI elements
	//

	createDocks();
	createToolbar();
	createActions();
	createMenu();
	createStatusBar();

	connect(&m_configController, &TestSuite::TestSuiteConfigController::configurationArrived, this, &TestSuiteMainWindow::onConfigurationArrived);
	connect(&m_testSuite, &TestSuite::TestSuite::finished, this, &TestSuiteMainWindow::onTestingFinished);

	// Logs
	//
	connect(&m_appLog, &Log::LogFile::alertArrived, &m_dialogAlert, &DialogAlert::onAlertArrived);
	connect(&m_appLog, &Log::LogFile::writeFailure, &m_dialogAlert, &DialogAlert::onAlertArrived);

	if (theSettings.useLocalScriptsPath() == true)
	{
		loadScriptsFromLocalPath();
	}

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

	m_mainWindowTimerId_250ms = startTimer(250);

	m_configController.start();

	updateActionsState();
}

TestSuiteMainWindow::~TestSuiteMainWindow()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

}

void TestSuiteMainWindow::createDocks()
{
	setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
	setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
	setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);

	// Tests List dock
	//
	QDockWidget* testsListDock = new QDockWidget{"TestListWidget", this};
	testsListDock->setObjectName("TestListWidget");
	testsListDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	testsListDock->setTitleBarWidget(new QWidget{});		// Hides title bar

	m_testListWidget = new TestListWidget{this};
	connect(m_testListWidget, &TestListWidget::testItemClicked, this, &TestSuiteMainWindow::onShowTestContents);
	testsListDock->setWidget(m_testListWidget);

	addDockWidget(Qt::LeftDockWidgetArea, testsListDock);

	// Overriden Signals dock
	//
//	m_overridePaneDock = new QDockWidget{"Overrides", this};
//	m_overridePaneDock->setObjectName("SimOverridenSignals");
//	m_overridePaneDock->setWidget(new SimOverridePane{m_simulator.get(), dbc(), m_overridePaneDock});
//	m_overridePaneDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

//	addDockWidget(Qt::BottomDockWidgetArea, m_overridePaneDock);

	// OutputLog dock
	//
//	if (m_slaveWindow == false)
	{
		m_appLogPaneDock = new QDockWidget{"Output", this};
		m_appLogPaneDock->setObjectName("AppLogOutputWidget");

		m_appLogoutputWidget = new AppLogOutputWidget{m_appLogPaneDock};

		m_appLogPaneDock->setWidget(m_appLogoutputWidget);
		m_appLogPaneDock->setAllowedAreas(Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_appLogPaneDock);
	}
}

void TestSuiteMainWindow::createToolbar()
{
	m_toolBar = new QToolBar{"ToolBar"};
	addToolBar(m_toolBar);

	//m_openTestsAction = new QAction{QIcon(":/Images/Images/SimOpen.svg"), tr("Open Build"), this};
	//m_openTestsAction->setShortcut(QKeySequence::Open);
	//connect(m_openTestsAction, &QAction::triggered, this, &SimWidget::openBuild);

	//m_closeTestsAction = new QAction{QIcon(":/Images/Images/SimClose.svg"), tr("Close"), this};
	//m_closeTestsAction->setShortcut(QKeySequence::Close);
	//connect(m_closeTestsAction, &QAction::triggered, this, &SimWidget::closeBuild);

	m_refreshTestsAction = new QAction{QIcon(":/Images/Images/TestsRefresh.svg"), tr("Refresh"), this};
	m_refreshTestsAction->setShortcut(QKeySequence::Refresh);
	connect(m_refreshTestsAction, &QAction::triggered, this, &TestSuiteMainWindow::onTestsRefresh);

	// --
	//
	m_runAction = new QAction{QIcon(":/Images/Images/TestsRun.svg"), tr("Run tests"), this};
	QList<QKeySequence> runsKeys;
	runsKeys << QKeySequence{Qt::CTRL | Qt::Key_R};
	runsKeys << QKeySequence{Qt::CTRL | Qt::Key_F5};
	m_runAction->setShortcuts(runsKeys);
	connect(m_runAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_run_clicked);

//	m_pauseAction = new QAction{QIcon(":/Images/Images/TestsPause.svg"), tr("Pause tests"), this};
//	connect(m_pauseAction, &QAction::triggered, this, &SimWidget::pauseSimulation);

	m_stopAction = new QAction{QIcon(":/Images/Images/TestsStop.svg"), tr("Stop tests"), this};
	m_stopAction->setShortcut(Qt::SHIFT | Qt::Key_F5);
	connect(m_stopAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_stop_clicked);

	// --
	//
	m_timeIndicator = new QLabel;

#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif
	m_timeIndicator->setFont(f);
	updateTimeIndicator(TestSuite::ControlStatus{});

	// --
	//
//	m_toolBar->addAction(m_openProjectAction);
//	m_toolBar->addAction(m_closeProjectAction);
	m_toolBar->addAction(m_refreshTestsAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_runAction);
	//m_toolBar->addAction(m_pauseAction);
	m_toolBar->addAction(m_stopAction);

	m_toolBar->addSeparator();
	m_toolBar->addWidget(m_timeIndicator);

	return;
}

void TestSuiteMainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	//m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &TestSuiteMainWindow::onExit);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &TestSuiteMainWindow::onSettings);
/*
	m_pTuningSourcesAction = new QAction(tr("Tuning sources..."), this);
	m_pTuningSourcesAction->setStatusTip(tr("View tuning sources"));
	//m_pTuningSourcesAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pTuningSourcesAction->setEnabled(true);
	connect(m_pTuningSourcesAction, &QAction::triggered, this, &MainWindow::showTuningSources);*/

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &TestSuiteMainWindow::showStatistics);

	m_pAppLogAction = new QAction(tr("Application Log..."), this);
	m_pAppLogAction->setStatusTip(tr("Show application log"));
	connect(m_pAppLogAction, &QAction::triggered, this, &TestSuiteMainWindow::showAppLog);

	/*m_pSignalLogAction = new QAction(tr("Signals Log..."), this);
	m_pSignalLogAction->setStatusTip(tr("Show signals log"));
	connect(m_pSignalLogAction, &QAction::triggered, this, &MainWindow::showSignalsLog);*/

	m_aboutQtAction = new QAction(tr("About Qt..."), this);
	m_aboutQtAction->setStatusTip(tr("Show Qt information"));
	//m_pAboutAction->setEnabled(true);
	connect(m_aboutQtAction, &QAction::triggered, this, &TestSuiteMainWindow::showAboutQt);

	m_pAboutAction = new QAction(tr("About TestSuite..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	//m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &TestSuiteMainWindow::showAbout);

	/*m_manualTuningAction = new QAction(tr("Tuning User Manual"), this);
	m_manualTuningAction->setStatusTip(tr("Show Tuning User Manual"));
	connect(m_manualTuningAction, &QAction::triggered, this, &MainWindow::showTuningUserManual);*/

}

void TestSuiteMainWindow::createMenu()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));
	pFileMenu->addAction(m_pExitAction);

	// Tools
	//
	QMenu* pServiceMenu = menuBar()->addMenu(tr("&Service"));
	pServiceMenu->addAction(m_pSettingsAction);

	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	//pHelpMenu->addAction(m_pTuningSourcesAction);
	pHelpMenu->addAction(m_pStatisticsAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_pAppLogAction);
	//pHelpMenu->addAction(m_pSignalLogAction);

	pHelpMenu->addSeparator();

	//pHelpMenu->addAction(m_manualTuningAction);

	//pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_aboutQtAction);
	pHelpMenu->addAction(m_pAboutAction);
}

void TestSuiteMainWindow::createStatusBar()
{
	m_statusBarProjectInfo = new QLabel();
	m_statusBarProjectInfo->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarConfigConnection->setMinimumWidth(100);
	m_statusBarConfigConnection->installEventFilter(this);

	m_statusBarAppDataConnection = new QLabel();
	m_statusBarAppDataConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarAppDataConnection->setMinimumWidth(100);
	m_statusBarAppDataConnection->installEventFilter(this);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_statusBarTuningConnection->setMinimumWidth(100);
	m_statusBarTuningConnection->installEventFilter(this);

	m_statusBarLogAlerts = new QLabel();
	m_statusBarLogAlerts->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->installEventFilter(this);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));

	// --
	//
	statusBar()->addPermanentWidget(m_statusBarProjectInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarAppDataConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);
}

void TestSuiteMainWindow::updateStatusBar()
{
	// Status bar
	//
	Q_ASSERT(m_statusBarConfigConnection);
	Q_ASSERT(m_statusBarAppDataConnection);
	Q_ASSERT(m_statusBarTuningConnection);
	Q_ASSERT(m_statusBarLogAlerts);


	std::vector<TcpClientStatistics::Statistics> stats = TcpClientStatistics::statistics();

	// ConfigService
	//
	{
		// ConfigService
		//
		Tcp::ConnectionState configConnState =  m_configController.getConnectionState();

		QString text = tr(" ConfigService: ");

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

		// --
		//
		QString tooltip = tr("Address: %1").arg(configConnState.peerAddr.toString());

		if (tooltip != m_statusBarConfigConnection->toolTip())
		{
			m_statusBarConfigConnection->setToolTip(tooltip);
		}
	}

	// AppDataService connection
	//
	if (m_configController.configuration().appDataServices.empty() == false)
	{
		showSoftwareConnection("AppDataService",
							   "TcpSignal",
							   stats,
							   m_statusBarAppDataConnection);
	}

	// TuningService connection
	//
	if (m_configController.configuration().tuningEnabled == true)
	{
		showSoftwareConnection("TuningService",
							   "TuningTcpClient",
							   stats,
							   m_statusBarTuningConnection);
	}

	// Log alerts tool
	//
	static int m_logErrorsCounter = -1;
	static int m_logWarningsCounter = -1;

	if (m_logErrorsCounter != m_appLog.errorAckCounter() || m_logWarningsCounter != m_appLog.warningAckCounter())
	{
		m_logErrorsCounter = m_appLog.errorAckCounter();
		m_logWarningsCounter = m_appLog.warningAckCounter();

		assert(m_statusBarLogAlerts);

		m_statusBarLogAlerts->setText(QString(" Log E: %1 W: %2 ").arg(m_logErrorsCounter).arg(m_logWarningsCounter));

		if (m_logErrorsCounter == 0 && m_logWarningsCounter == 0)
		{
			m_statusBarLogAlerts->setStyleSheet(m_statusBarProjectInfo->styleSheet());
		}
		else
		{
			if (m_logErrorsCounter == 0)
			{
				m_statusBarLogAlerts->setStyleSheet("QLabel {color : white; background-color: #F87217}");
			}
			else
			{
				m_statusBarLogAlerts->setStyleSheet(QString("QLabel {color : white; background-color: #C00000}"));
			}
		}
	}
}

void TestSuiteMainWindow::showSoftwareConnection(const QString& caption,
												 const QString& nameFilter,
												 const std::vector<TcpClientStatistics::Statistics>& connectionStatistics,
												 QLabel* label)
{
	if (label == nullptr)
	{
		Q_ASSERT(label);
		return;
	}

	QString toolTipText = tr("%1:\n").arg(caption);

	if (connectionStatistics.empty() == true)
	{
		toolTipText += tr("Not configured");
	}

	std::vector<Tcp::ConnectionState> states;
	for (const TcpClientStatistics::Statistics& stats : connectionStatistics)
	{
		if (nameFilter.isEmpty() == true ||
				stats.objectName.startsWith(nameFilter) == true)
		{
			states.push_back(stats.state);
		}
	}

	int statusOk = 0;
	qint64 replyCount = 0;
	for (const Tcp::ConnectionState& state : states)
	{
		if (state.isConnected == true)
		{
			statusOk ++;
		}

		replyCount += state.replyCount;

		toolTipText += QString("%1 %2 (%3)\n")
							.arg(state.connectedSoftwareInfo.equipmentID())
							.arg(state.peerAddr.addressPortStr())
							.arg(state.isConnected ? "ok" : "down");
	}
	toolTipText = toolTipText.trimmed();

	label->setText(caption);
	label->setToolTip(toolTipText);

	QString statusText;

	if (states.size() <= 1)
	{
		statusText = tr("%1: %2 (Replies: %3)")
					 .arg(caption)
					 .arg(statusOk ? "ok" : "down")
					 .arg(replyCount);
	}
	else
	{
		statusText = tr("%1: %2/%3 (Replies: %4)")
					 .arg(caption)
					 .arg(statusOk)
					 .arg(states.size())
					 .arg(replyCount);
	}

	label->setText(statusText);

	return;
}


void TestSuiteMainWindow::loadScriptsFromConfiguration()
{
	m_testScriptsStorage.setScripts(m_configController.scripts());
	fillTestsTree();
}

void TestSuiteMainWindow::loadScriptsFromLocalPath()
{
	QString errorMsg;
	bool ok = m_testScriptsStorage.loadFromPath(theSettings.localScriptsPath(), &errorMsg);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error loading scripts from path %1: %2.").arg(theSettings.localScriptsPath()).arg(errorMsg));
		return;
	}

	fillTestsTree();
}

void TestSuiteMainWindow::clearTestsTree()
{
	m_testListWidget->clearTestsList();
}

void TestSuiteMainWindow::fillTestsTree()
{
	m_testListWidget->updateTestsList(m_testScriptsStorage.scriptList());
}

void TestSuiteMainWindow::updateActionsState()
{
	m_runAction->setEnabled(!m_testSuite.isRunning());
	m_stopAction->setEnabled(m_testSuite.isRunning());
}

void TestSuiteMainWindow::updateTimeIndicator(const TestSuite::ControlStatus& state)
{
using namespace std::chrono;

	Q_ASSERT(m_timeIndicator);

	milliseconds durration = duration_cast<milliseconds>(state.m_duration);

	qint64 days = durration.count() / 1_day;
	qint64 hours = (durration.count() % 1_day)  / 1_hour;
	qint64 minutes = (durration.count() % 1_hour)  / 1_min;
	qint64 seconds = (durration.count() % 1_min)  / 1_sec;
	qint64 millisecond = durration.count() % 1_sec;

	auto ms = duration_cast<milliseconds>(state.m_currentTime);
	QDateTime utcOffset = QDateTime::currentDateTime();
	TimeStamp plantTime{ms.count() + utcOffset.offsetFromUtc() * 1000};

	QDateTime currentTime = plantTime.toDateTime();

	if (currentTime.date().year() == 1970)
	{
		currentTime = QDateTime::currentDateTime();
	}

	QLocale locale;

#if 1
	// IF UNCOMMENTING THIS CODE
	// and if you want to show milliseconds,
	// THEN do not forget to send message more frequently
	// in Sim::Control::processRun emit statusUpdate(ControlStatus{cd});
	//
	//        0d 00:20:03.580
	//05/17/2020 15:18:59.335

	QString dateText = QString("%6 %7")
					   .arg(locale.toString(currentTime.date(),  QLocale::FormatType::ShortFormat))
					   .arg(currentTime.toString(QStringLiteral("hh:mm:ss.zzz")));

	QString text = tr("%1d %2:%3:%4.%5\n%6")
					.arg(days, static_cast<int>(dateText.size()) - 14, 10, QChar(' '))
					.arg(hours, 2, 10, QChar('0'))
					.arg(minutes, 2, 10, QChar('0'))
					.arg(seconds, 2, 10, QChar('0'))
					.arg(millisecond, 3, 10, QChar('0'))
					.arg(dateText);
#else
	//        0d 00:20:03
	//05/17/2020 15:18:59

	QString dateText = QString("%6 %7")
					   .arg(locale.toString(currentTime.date(),  QLocale::FormatType::ShortFormat))
					   .arg(currentTime.toString(QStringLiteral("hh:mm:ss")));

	QString text = tr("%1d %2:%3:%4\n%6")
					.arg(days, static_cast<int>(dateText.size()) - 10, 10, QChar(' '))
					.arg(hours, 2, 10, QChar('0'))
					.arg(minutes, 2, 10, QChar('0'))
					.arg(seconds, 2, 10, QChar('0'))
					.arg(dateText);
#endif

	m_timeIndicator->setText(text);

	return;
}

bool TestSuiteMainWindow::eventFilter(QObject *object, QEvent *event)
{
	if ((object == m_statusBarConfigConnection ||
		 object == m_statusBarAppDataConnection ||
		 object == m_statusBarTuningConnection) &&
		event->type() == QEvent::MouseButtonPress)
	{
		showStatistics();
	}

	if (object == m_statusBarLogAlerts &&
		m_statusBarLogAlerts->text().isEmpty() == false &&
		event->type() == QEvent::MouseButtonPress)
	{
		showAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void TestSuiteMainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);
	// Update status bar
	//
	if  (event->timerId() == m_mainWindowTimerId_250ms)
	{
		updateStatusBar();
	}

}


void TestSuiteMainWindow::onExit()
{
	close();
}

void TestSuiteMainWindow::on_m_run_clicked()
{
	if (m_testSuite.isRunning() == true)
	{
		return;
	}

	// Create a list of tests user has selected to run
	//
	QStringList scriptsToExecute = m_testListWidget->selectedTests();

	if (scriptsToExecute.isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Please choose at least one test to run."));
		return;
	}

	m_testLogTabPage->clearOutputLog();

	m_tabWidget->setCurrentIndex(0);

	// Run tests
	//
	bool ok = m_testSuite.execute(scriptsToExecute, theSettings.useLocalScriptsPath() ? theSettings.localScriptsPath() : QString());
	if (ok == false)
	{
		return;
	}

	updateActionsState();
}

void TestSuiteMainWindow::on_m_stop_clicked()
{
	if (m_testSuite.isRunning() == true)
	{
		m_testSuite.stop();
	}
}

void TestSuiteMainWindow::onSettings()
{
	TestSuiteDialogSettings d(this);
	d.setSettings(theSettings);

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		// --
		//
		bool needReconnect = false;

		auto currentSettings = theSettings;

		if (currentSettings.librarySettings().instanceStrId() != d.settings().librarySettings().instanceStrId() ||
			currentSettings.librarySettings().configuratorAddress1() != d.settings().librarySettings().configuratorAddress1() ||
			currentSettings.librarySettings().configuratorAddress2() != d.settings().librarySettings().configuratorAddress2())
		{
			needReconnect = true;
		}

		// --
		//
		if (currentSettings.useLocalScriptsPath() == true && d.settings().useLocalScriptsPath() == false)
		{
			// Tests are NOT loaded from local folder now - clear them
			//
			loadScriptsFromConfiguration();
		}
		else
		{
			if (currentSettings.useLocalScriptsPath() == false && d.settings().useLocalScriptsPath() == true)
			{
				// Tests ARE loaded from local folder now - load them
				//
				loadScriptsFromLocalPath();
			}
		}

		// --
		//
		theSettings = d.settings();
		theSettings.StoreSystem();
		theSettings.StoreUser();

		// Reconnect
		//
		if (needReconnect == true)
		{
			m_configController.setConnectionParams(theSettings.librarySettings().instanceStrId(),
															   theSettings.librarySettings().configuratorAddress1(),
															   theSettings.librarySettings().configuratorAddress2());
		}

		return;
	}

	return;
}

void TestSuiteMainWindow::showStatistics()
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

void TestSuiteMainWindow::showAppLog()
{
	m_appLog.view(this);
}

void TestSuiteMainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void TestSuiteMainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to run application logic tests.");
	DialogAbout::show(this, text, ":/Images/Images/logo.png");
}

void TestSuiteMainWindow::onTestsRefresh()
{
	// Reload scripts that displayed by the user interface. Actual executed scripts are loaded at testing start.
	//
	if (theSettings.useLocalScriptsPath() == true)
	{
		loadScriptsFromLocalPath();
	}
	else
	{
		loadScriptsFromConfiguration();
	}
}

void TestSuiteMainWindow::onShowTestContents(const QString& testName)
{
	const TestSuite::TestScript& script = m_testScriptsStorage.script(::calcHash(testName));

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		// Check if tab page with this script already exists, open it if so
		//
		QWidget* w = m_tabWidget->widget(i);
		if (w == nullptr)
		{
			Q_ASSERT(w);
			continue;
		}
		TestViewTabPage* p = dynamic_cast<TestViewTabPage*>(w);
		if (p == nullptr)
		{
			continue;
		}
		if (p->script().hash() == script.hash())
		{
			m_tabWidget->setCurrentIndex(i);;
			return;
		}
	}

	TestViewTabPage* p = new TestViewTabPage(script, this);
	m_tabWidget->addTab(p, script.fileName());
	m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);

	return;
}

void TestSuiteMainWindow::onTabCloseRequested(int index)
{
	QWidget* w = m_tabWidget->widget(index);
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}

	if (dynamic_cast<TestViewTabPage*>(w) == nullptr)
	{
		return;
	}

	delete w;
	m_tabWidget->removeTab(index);
}

void TestSuiteMainWindow::onConfigurationArrived()
{
	if (theSettings.useLocalScriptsPath() == false)
	{
		loadScriptsFromConfiguration();
	}

	return;
}

void TestSuiteMainWindow::onTestingFinished(int result)
{
	updateActionsState();
}

TestSuiteMainWindow* theMainWindow = nullptr;
