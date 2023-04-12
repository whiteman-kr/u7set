#include "TestSuiteMainWindow.h"
#include "ui_TestSuiteMainWindow.h"
#include "AppConfigSettings.h"
#include "TestSuiteDialogSettings.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "../lib/Ui/DialogAbout.h"
#include "../OnlineLib/TcpClientStatistics.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

TestSuiteMainWindow::TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::TestSuiteMainWindow),
	m_appLog(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + theSettings.librarySettings().instanceStrId()),
	m_configController(softwareInfo, theSettings.librarySettings().configuratorAddress1(), theSettings.librarySettings().configuratorAddress2(), &m_appLog),
	m_testSuite(softwareInfo, theSettings.librarySettings(), &m_appLog, &m_testLog),
	m_dialogAlert(this)

{
	ui->setupUi(this);

	ui->testsTree->setRootIsDecorated(false);
	QStringList headerLabels;
	headerLabels << tr("Test");
	ui->testsTree->setColumnCount(static_cast<int>(headerLabels.size()));
	ui->testsTree->setHeaderLabels(headerLabels);
	ui->testsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	connect(&m_configController, &TestSuite::TestSuiteConfigController::configurationArrived, this, &TestSuiteMainWindow::onConfigurationArrived);
	connect(&m_testSuite, &TestSuite::TestSuite::finished, this, &TestSuiteMainWindow::onTestingFinished);

	// Logs
	//
	connect(&m_appLog, &TestSuiteLogFile::errorArrived, this, &TestSuiteMainWindow::onAppLogError, Qt::QueuedConnection);
	connect(&m_appLog, &TestSuiteLogFile::warningArrived, this, &TestSuiteMainWindow::onAppLogWarning, Qt::QueuedConnection);
	connect(&m_appLog, &TestSuiteLogFile::messageArrived, this, &TestSuiteMainWindow::onAppLogMessage, Qt::QueuedConnection);
	connect(&m_appLog, &TestSuiteLogFile::textArrived, this, &TestSuiteMainWindow::onAppLogText, Qt::QueuedConnection);
	connect(&m_appLog, &Log::LogFile::alertArrived, &m_dialogAlert, &DialogAlert::onAlertArrived);
	connect(&m_appLog, &Log::LogFile::writeFailure, &m_dialogAlert, &DialogAlert::onAlertArrived);

	connect(&m_testLog, &TestSuiteTestLog::errorArrived, this, &TestSuiteMainWindow::onTestLogError, Qt::QueuedConnection);
	connect(&m_testLog, &TestSuiteTestLog::warningArrived, this, &TestSuiteMainWindow::onTestLogWarning, Qt::QueuedConnection);
	connect(&m_testLog, &TestSuiteTestLog::messageArrived, this, &TestSuiteMainWindow::onTestLogMessage, Qt::QueuedConnection);

	createActions();
	createMenu();
	createStatusBar();

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
}

TestSuiteMainWindow::~TestSuiteMainWindow()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	delete ui;
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
	ui->testsTree->clear();
}

void TestSuiteMainWindow::fillTestsTree()
{
	clearTestsTree();

	QStringList l = m_testScriptsStorage.scriptList();

	for (const QString& fileName : l)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << fileName);
		item->setCheckState(0, Qt::Checked);
		ui->testsTree->addTopLevelItem(item);
		item->setData(0, Qt::UserRole, fileName);
	}
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
	QStringList scriptsToExecute;

	for (int i = 0; i < ui->testsTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->testsTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
		{
			QString fileName = item->data(0, Qt::UserRole).toString();
			scriptsToExecute.push_back(fileName);
		}
	}

	if (scriptsToExecute.isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Please choose at least one test to run."));
		return;
	}

	// Run tests
	//
	bool ok = m_testSuite.execute(scriptsToExecute, theSettings.useLocalScriptsPath() ? theSettings.localScriptsPath() : QString());
	if (ok == false)
	{
		return;
	}
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

}

void TestSuiteMainWindow::onAppLogError(const QString& errMsg)
{
	ui->appLog->moveCursor (QTextCursor::End);
	ui->appLog->insertPlainText(errMsg);
	ui->appLog->insertPlainText("\n");
	ui->appLog->moveCursor (QTextCursor::End);

	QMessageBox::critical(this, qAppName(), errMsg);
	return;
}

void TestSuiteMainWindow::onAppLogWarning(const QString& msg)
{
	ui->appLog->moveCursor (QTextCursor::End);
	ui->appLog->insertPlainText(msg);
	ui->appLog->insertPlainText("\n");
	ui->appLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onAppLogMessage(const QString& msg)
{
	ui->appLog->moveCursor (QTextCursor::End);
	ui->appLog->insertPlainText(msg);
	ui->appLog->insertPlainText("\n");
	ui->appLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onAppLogText(const QString& msg)
{
	ui->appLog->moveCursor (QTextCursor::End);
	ui->appLog->insertPlainText(msg);
	ui->appLog->insertPlainText("\n");
	ui->appLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onTestLogError(const QString& errMsg)
{
	ui->testLog->moveCursor (QTextCursor::End);
	ui->testLog->insertPlainText(errMsg);
	ui->testLog->insertPlainText("\n");
	ui->testLog->moveCursor (QTextCursor::End);

	QMessageBox::critical(this, qAppName(), errMsg);
	return;
}

void TestSuiteMainWindow::onTestLogWarning(const QString& msg)
{
	ui->testLog->moveCursor (QTextCursor::End);
	ui->testLog->insertPlainText(msg);
	ui->testLog->insertPlainText("\n");
	ui->testLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onTestLogMessage(const QString& msg)
{
	ui->testLog->moveCursor (QTextCursor::End);
	ui->testLog->insertPlainText(msg);
	ui->testLog->insertPlainText("\n");
	ui->testLog->moveCursor (QTextCursor::End);
}



TestSuiteMainWindow* theMainWindow = nullptr;
