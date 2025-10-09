#include "../OnlineLib/TcpClientStatistics.h"
#include <UiLib/UiTools.h>
#include <ClientLib/ClientTranslator.h>
#include <ClientLib/TuningUserManager.h>
#include <TestSuiteLib/TestReport.h>
#include <UiLib/DialogAbout.h>
#include <UiLib/TabWidgetEx.h>
#include <UiLib/LogDialog.h>

#include "AppConfigSettings.h"
#include "DialogDataSources.h"
#include "DialogReport.h"
#include "TestLogTabPage.h"
#include "TestSuiteDialogSettings.h"
#include "TestSuiteMainWindow.h"
#include "TestViewTabPage.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QStandardPaths>


TestSuiteMainWindow::TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_appLog(qAppName(),
			 QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' +
				 AppConfigSettings().instance().librarySettings().instanceStrId()),
	m_configController(softwareInfo,
					   AppConfigSettings().instance().librarySettings().configuratorAddress1(),
					   AppConfigSettings().instance().librarySettings().configuratorAddress2(),
					   &m_appLog),
	m_testSuite(m_configController, &m_appLog, &m_testLogOutput),
	m_dialogAlert(this)

{
	setWindowFlags(Qt::Widget);
	setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

	setWindowTitle(tr("%1 - %2").arg(qAppName()).arg(AppConfigSettings().instance().librarySettings().instanceStrId()));

	// Init translator
	//
	m_translator.addLanguage("en", "English");
	m_translator.addLanguage("uk", "Ukrainian");
	m_translator.addLanguage("bg", "Bulgarian");

	for (const QString& l : m_translator.languagesList())
	{
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/TestSuite_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/TestSuiteLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/ClientLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/UtilsLib_%1.qm").arg(l));
		m_translator.addTranslationFile(l, qApp->applicationDirPath() + QObject::tr("/translations/qt_%1.qm").arg(l));
	}

	if (AppConfigSettings::instance().language() != "en")
	{
		QStringList failedTranslations;
		if (m_translator.setLanguage(AppConfigSettings().instance().language(), failedTranslations) == false)
		{
			if (failedTranslations.isEmpty() == false)
			{
				m_appLog.writeError("Failed to load translation files:\n" + failedTranslations.join('\n'));
			}
			else
			{
				m_appLog.writeError("Failed to set language: " + AppConfigSettings().instance().language());
			}
		}
	}
	//

	m_tabWidget = new UiLib::TabWidgetEx{this};
	m_tabWidget->setDocumentMode(false);
	m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tabWidget, &UiLib::TabWidgetEx::tabCloseRequested, this, &TestSuiteMainWindow::onTabCloseRequested);

	setCentralWidget(m_tabWidget);
	centralWidget()->setAutoFillBackground(true);

	QHBoxLayout* layout = new QHBoxLayout;
	centralWidget()->setLayout(layout);

	auto margins = layout->contentsMargins();
	margins.setTop(0);
	layout->setContentsMargins(margins);

	m_testLogTabPage = new TestLogTabPage(m_testSuite.testLog(), m_testLogOutput, this);
	m_tabWidget->addTab(m_testLogTabPage, tr("Test Log"));
	m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, 0); // This tab is not closable

	m_testLogOutput.setHtmlFont("Verdana");

	// Create UI elements
	//

	createDocks();
	createActions();
	createToolbar();
	createMenu();
	createStatusBar();

	connect(&m_configController,
			&TestSuite::TestSuiteConfigController::configurationArrived,
			this,
			&TestSuiteMainWindow::onConfigurationArrived);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::testStarted, m_testListWidget, &TestListWidget::onTestStarted);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::testFinished, m_testListWidget, &TestListWidget::onTestFinished);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::finished, this, &TestSuiteMainWindow::onTestingFinished);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::globalPermissionChanged, this, &TestSuiteMainWindow::onGlobalPermissionChanged);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::scriptPermissionChanged, this, &TestSuiteMainWindow::onScriptPermissionChanged);

	connect(&m_testSuite, &TestSuite::MatsTestSuite::scriptPermissionChanged, m_testListWidget, &TestListWidget::onScriptPermissionChanged);
	connect(&m_testSuite, &TestSuite::MatsTestSuite::noPermissionsExist, m_testListWidget, &TestListWidget::onNoPermissionsExist);


	// Logs
	//
	connect(&m_appLog, &Log::LogFile::alertArrived, &m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);
	connect(&m_appLog, &Log::LogFile::writeFailure, &m_dialogAlert, &UiLib::DialogAlert::onAlertArrived);

	// MainWindow options
	//
	QPoint mainWindowPos = QSettings().value("MainWindow/pos", QPoint(-1, -1)).toPoint();
	QByteArray mainWindowGeometry = QSettings().value("MainWindow/geometry").toByteArray();
	QByteArray mainWindowState = QSettings().value("MainWindow/state").toByteArray();

	if (mainWindowPos.x() != -1 && mainWindowPos.y() != -1)
	{
		move(mainWindowPos);
		restoreGeometry(mainWindowGeometry);
		restoreState(mainWindowState);
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
	QSettings().setValue("MainWindow/pos", pos());
	QSettings().setValue("MainWindow/geometry", saveGeometry());
	QSettings().setValue("MainWindow/state", saveState());
}

void TestSuiteMainWindow::createDocks()
{
	setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
	setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
	setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);

	// Tests List dock
	//
	QDockWidget* testsListDock = new QDockWidget{tr("TestListWidget"), this};
	testsListDock->setObjectName("TestListWidget");
	testsListDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	testsListDock->setTitleBarWidget(new QWidget{}); // Hides title bar

	m_testListWidget = new TestListWidget{m_testSuite, m_appLog, m_configuration, m_testScriptsStorage, this};
	connect(m_testListWidget, &TestListWidget::testItemClicked, this, &TestSuiteMainWindow::onShowTestContents);
	connect(m_testListWidget,
			&TestListWidget::testSelectionChanged,
			this,
			[this]()
			{
				updateActionsState();
			});
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
		m_appLogPaneDock = new QDockWidget{tr("Output"), this};
		m_appLogPaneDock->setObjectName("AppLogOutputWidget");

		m_appLogoutputWidget = new AppLogOutputWidget{m_appLogPaneDock};

		m_appLogPaneDock->setWidget(m_appLogoutputWidget);
		m_appLogPaneDock->setAllowedAreas(Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_appLogPaneDock);
	}
}

void TestSuiteMainWindow::createToolbar()
{
	m_toolBar = new QToolBar{tr("ToolBar")};
	addToolBar(m_toolBar);

	//
	m_statusIndicator = new QLabel;
	m_statusIndicator->setAlignment(Qt::AlignRight);
	m_statusIndicator->setMinimumHeight(40);

#if defined(Q_OS_WIN)
	QFont f = QFont("Consolas");
#else
	QFont f = QFont("Courier");
#endif
	m_statusIndicator->setFont(f);
	updateStatusIndicator();

	// --
	//
	m_toolBar->addAction(m_reloadTestsScriptsAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_runAction);
	m_toolBar->addAction(m_stopAction);

	m_toolBar->addSeparator();

	m_toolBar->addAction(m_loadTestLogAction);
	m_toolBar->addAction(m_saveTestLogAction);
	m_toolBar->addAction(m_reportToolbarAction);

	m_toolBar->addSeparator();

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_toolBar->addWidget(spacer);

	m_toolBar->addWidget(m_statusIndicator);

	return;
}

void TestSuiteMainWindow::createActions()
{
	m_loadTestLogAction = new QAction(tr("Load Test Log..."), this);
	m_loadTestLogAction->setStatusTip(tr("Load Test Log from file"));
	m_loadTestLogAction->setIcon(QIcon(":/Images/Images/OpenLog.svg"));
	m_loadTestLogAction->setEnabled(true);
	connect(m_loadTestLogAction, &QAction::triggered, this, &TestSuiteMainWindow::onLoadTestLog);

	m_saveTestLogAction = new QAction(tr("Save Test Log..."), this);
	m_saveTestLogAction->setStatusTip(tr("Save Test Log to file"));
	m_saveTestLogAction->setIcon(QIcon(":/Images/Images/SaveLog.svg"));
	m_saveTestLogAction->setEnabled(true);
	connect(m_saveTestLogAction, &QAction::triggered, this, &TestSuiteMainWindow::onSaveTestLog);

	m_clearTestLogAction = new QAction(tr("Clear Test Log"), this);
	m_clearTestLogAction->setStatusTip(tr("Clear Test Log"));
	// m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_clearTestLogAction->setEnabled(true);
	connect(m_clearTestLogAction, &QAction::triggered, this, &TestSuiteMainWindow::onClearTestLog);

	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	// m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &TestSuiteMainWindow::onExit);

	m_reloadTestsScriptsAction = new QAction{QIcon(":/Images/Images/TestsRefresh.svg"), tr("Reload Tests Scripts"), this};
	m_reloadTestsScriptsAction->setVisible(AppConfigSettings().instance().useLocalScriptsPath() == true);
	connect(m_reloadTestsScriptsAction, &QAction::triggered, this, &TestSuiteMainWindow::onTestsScriptsReload);

	// --
	//
	m_runAction = new QAction{QIcon(":/Images/Images/TestsRun.svg"), tr("Run tests"), this};
	QList<QKeySequence> runsKeys;
	runsKeys << QKeySequence{Qt::Key_F5};
	m_runAction->setShortcuts(runsKeys);
	connect(m_runAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_run_clicked);

	//	m_pauseAction = new QAction{QIcon(":/Images/Images/TestsPause.svg"), tr("Pause tests"), this};
	//	connect(m_pauseAction, &QAction::triggered, this, &SimWidget::pauseSimulation);

	m_stopAction = new QAction{QIcon(":/Images/Images/TestsStop.svg"), tr("Stop tests"), this};
	m_stopAction->setShortcut(Qt::SHIFT | Qt::Key_F5);
	connect(m_stopAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_stop_clicked);

	// --
	//
	m_reportToolbarAction = new QAction{QIcon(":/Images/Images/TestsReport.svg"), tr("Report"), this};
	connect(m_reportToolbarAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_report_clicked);

	m_singleReportAction = new QAction(tr("Report..."), this);
	m_singleReportAction->setStatusTip(tr("Generate the report"));
	connect(m_singleReportAction, &QAction::triggered, this, &TestSuiteMainWindow::on_m_single_report_clicked);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	// m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &TestSuiteMainWindow::onSettings);

	m_pDataSourcesAction = new QAction(tr("Data Sources..."), this);
	m_pDataSourcesAction->setStatusTip(tr("View Data Sources"));
	m_pDataSourcesAction->setEnabled(true);
	connect(m_pDataSourcesAction, &QAction::triggered, this, &TestSuiteMainWindow::showDataSources);

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &TestSuiteMainWindow::showStatistics);

	m_pAppLogAction = new QAction(tr("Application Log..."), this);
	m_pAppLogAction->setStatusTip(tr("Show application log"));
	connect(m_pAppLogAction, &QAction::triggered, this, &TestSuiteMainWindow::showAppLog);

	m_aboutQtAction = new QAction(tr("About Qt..."), this);
	m_aboutQtAction->setStatusTip(tr("Show Qt information"));
	connect(m_aboutQtAction, &QAction::triggered, this, &TestSuiteMainWindow::showAboutQt);

	m_pAboutAction = new QAction(tr("About TestSuite..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	// m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	// m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &TestSuiteMainWindow::showAbout);


	m_viewGlobalScriptAction = new QAction(tr("View GlobalScript"), this);
	m_viewGlobalScriptAction->setStatusTip(tr("View GlobalScript Code"));
	connect(m_viewGlobalScriptAction, &QAction::triggered, this, &TestSuiteMainWindow::viewGlobalScript);

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

	// Tests
	//
	QMenu* pTestsMenu = menuBar()->addMenu(tr("&Tests"));
	pTestsMenu->addAction(m_runAction);
	pTestsMenu->addAction(m_stopAction);

	// Reports
	//
	QMenu* pReportsMenu = menuBar()->addMenu(tr("&Reports"));
	pReportsMenu->addAction(m_loadTestLogAction);
	pReportsMenu->addAction(m_saveTestLogAction);
	pReportsMenu->addSeparator();
	pReportsMenu->addAction(m_clearTestLogAction);
	pReportsMenu->addSeparator();

	m_multipleReportsMenu = pReportsMenu->addMenu(tr("Report"));
	m_multipleReportsMenu->setEnabled(false);
	m_multipleReportsMenu->menuAction()->setVisible(false);

	pReportsMenu->addAction(m_singleReportAction);
	m_singleReportAction->setVisible(false);

	// Service
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));
	pToolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

	pHelpMenu->addAction(m_pDataSourcesAction);
	pHelpMenu->addAction(m_pStatisticsAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_pAppLogAction);
	// pHelpMenu->addAction(m_pSignalLogAction);

	pHelpMenu->addSeparator();

	pHelpMenu->addAction(m_viewGlobalScriptAction);

	pHelpMenu->addSeparator();
	// pHelpMenu->addAction(m_manualTuningAction);

	// pHelpMenu->addSeparator();

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
		Tcp::ConnectionState configConnState = m_configController.getConnectionState();

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
	if (m_configuration.appDataServices.empty() == false)
	{
		showSoftwareConnection("AppDataService", "TcpSignal", stats, m_statusBarAppDataConnection);
	}

	// TuningService connection
	//
	if (m_configuration.tuningEnabled == true)
	{
		showSoftwareConnection("TuningService", "TuningTcpClient", stats, m_statusBarTuningConnection);
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
		if (nameFilter.isEmpty() == true || stats.objectName.startsWith(nameFilter) == true)
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
			statusOk++;
		}

		replyCount += state.replyCount;

		toolTipText += QString("%1 %2 (%3)\n")
						   .arg(state.connectedSoftwareInfo.equipmentID())
						   .arg(state.peerAddr.addressPortStr())
						   .arg(state.isConnected ? tr("ok") : tr("down"));
	}
	toolTipText = toolTipText.trimmed();

	label->setText(caption);
	label->setToolTip(toolTipText);

	QString statusText;

	if (states.size() <= 1)
	{
		statusText = tr("%1: %2 (Replies: %3)").arg(caption).arg(statusOk ? tr("ok") : tr("down")).arg(replyCount);
	}
	else
	{
		statusText = tr("%1: %2/%3 (Replies: %4)").arg(caption).arg(statusOk).arg(states.size()).arg(replyCount);
	}

	label->setText(statusText);

	return;
}


void TestSuiteMainWindow::loadScriptsFromConfiguration()
{
	m_testScriptsStorage.setScripts(m_configData.scripts);

	m_testListWidget->fillTestsTree();

	// Reset or restart tests run control thread
	//
	if (m_testSuite.hasRunControl() == false)
	{
		m_testSuite.executeRunControl(m_testScriptsStorage);
	}
	else
	{
		m_testSuite.resetRunControl();
	}
}

void TestSuiteMainWindow::loadScriptsFromLocalPath()
{
	QString errorMsg;
	bool ok = m_testScriptsStorage.loadFromPath(AppConfigSettings().instance().localScriptsPath(), &errorMsg);
	if (ok == false)
	{
		QMessageBox::critical(
			this,
			qAppName(),
			tr("Error loading scripts from path %1: %2.").arg(AppConfigSettings().instance().localScriptsPath()).arg(errorMsg));
		return;
	}

	m_testListWidget->fillTestsTree();

	// Reset or restart tests run control thread
	//
	if (m_testSuite.hasRunControl() == false)
	{
		m_testSuite.executeRunControl(m_testScriptsStorage);
	}
	else
	{
		m_testSuite.resetRunControl();
	}
}

void TestSuiteMainWindow::updateReportActions()
{
	Q_ASSERT(m_multipleReportsMenu);
	m_multipleReportsMenu->clear();

	for (QAction* a : m_multipleReportActions)
	{
		delete a;
	}
	m_multipleReportActions.clear();


	const auto& templates = m_configData.reportTemplates.templates();

	m_singleReportAction->setVisible(templates.size() == 1);
	m_multipleReportsMenu->menuAction()->setVisible(templates.size() > 1);

	if (templates.size() > 1)
	{
		for (const ReportLib::ReportTemplate& report : templates)
		{
			QAction* a = new QAction(report.caption(), this);
			a->setStatusTip(tr("Generate report: %1").arg(report.caption()));
			a->setEnabled(true);
			connect(a,
					&QAction::triggered,
					this,
					[this, report]()
					{
						onGenerateReport(report.caption());
					});

			m_multipleReportActions.push_back(a);
			m_multipleReportsMenu->addAction(a);
		}
	}

	m_multipleReportsMenu->setEnabled(m_multipleReportActions.empty() == false);
}

void TestSuiteMainWindow::updateTestViewTabPages()
{
	std::vector<int> tabsToClose;

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		// Check if tab page with this script already exists, open it if so
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

		if (m_testScriptsStorage.hasScript(p->script().fileNameHash()) == false)
		{
			// No such script, close the tab
			tabsToClose.push_back(i);
		}
		else
		{
			// Update script contents if it has been changed
			const TestSuite::TestScript& script = m_testScriptsStorage.script(p->script().fileNameHash());
			if (script.script() != p->script().script())
			{
				p->setScript(script);
			}
		}
	}

	// Close tabs with non-existing more scripts
	std::sort(tabsToClose.begin(), tabsToClose.end(), std::greater<int>());
	for (int i : tabsToClose)
	{
		onTabCloseRequested(i);
	}
}

void TestSuiteMainWindow::updateActionsState()
{
	auto selection = m_testListWidget->getTestScriptSelection();

	bool isRunning = m_testSuite.isRunning();

	m_runAction->setEnabled(isRunning == false && selection.isEmpty() == false);
	m_stopAction->setEnabled(isRunning == true);

	m_pSettingsAction->setEnabled(isRunning == false);
	m_reloadTestsScriptsAction->setEnabled(isRunning == false);
	for (QAction* a : m_multipleReportActions)
	{
		a->setEnabled(isRunning == false);
	}
	m_reportToolbarAction->setEnabled(isRunning == false);
	m_saveTestLogAction->setEnabled(isRunning == false);
	m_loadTestLogAction->setEnabled(isRunning == false);
	m_clearTestLogAction->setEnabled(isRunning == false);

	m_testListWidget->setSelectionEnabled(isRunning == false);
}

bool TestSuiteMainWindow::loadTestLog()
{
	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Load Test Log"),
													QString(),
													tr("TestSuite Log File (*.tsl);;CSV Files, semicolon separated (*.csv)"));

	if (fileName.isEmpty() == true)
	{
		return false;
	}

	// Clear previous log
	m_testSuite.testLog().clear();
	m_testLogTabPage->clearOutputWidget();


	QString errorMsg;
	bool ok = m_testSuite.testLog().loadFromCSV(fileName, &errorMsg);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMsg);
		return false;
	}

	m_testLogOutput.pushQueue(m_testSuite.testLog().items());

	return true;
}

void TestSuiteMainWindow::saveTestLog()
{
	if (m_testSuite.testLog().empty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Test log is empty. No information could be saved."));
		return;
	}

	QString defaultFileName = QString("TestLog_%1.tsl").arg(QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss"));

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save Test Log"),
													defaultFileName,
													tr("TestSuite Log File (*.tsl);;CSV Files, semicolon separated (*.csv)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

	QString errorMsg;
	bool ok = m_testSuite.testLog().saveToCSV(fileName, &errorMsg);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMsg);
		return;
	}

	return;
}

void TestSuiteMainWindow::updateStatusIndicator()
{
	QString text;
	QString styleSheet;

	if (m_testSuite.globalPermission() == false)
	{
		text = tr("No permission to start testing.\n");
		styleSheet = "QLabel {color : #ff0000;}";
	}
	else
	{
		TestSuite::ControlStatus testStatus = m_testSuite.testStatus();

		switch (testStatus.m_state)
		{
		case TestSuite::ControlState::Stop:
			{
				text = tr("Tests are not running.\n");
			}
			break;
		case TestSuite::ControlState::RequestingConfiguration:
			{
				text = tr("Requesting test configuration...\n");
			}
			break;
		case TestSuite::ControlState::InitInputController:
			{
				text = tr("Initializing input controller...\n");
			}
			break;
		case TestSuite::ControlState::InitOutputController:
			{
				text = tr("Initializing output controller...\n");
			}
			break;
		case TestSuite::ControlState::RunningTests:
			{
				text = tr("Running script file: %1 (%2 of %3)\nTest function: %4 (%5 of %6)")
						   .arg(testStatus.m_scriptFile)
						   .arg(testStatus.m_scriptIndex)
						   .arg(testStatus.m_scriptCount)
						   .arg(testStatus.m_testFunction)
						   .arg(testStatus.m_testIndex)
						   .arg(testStatus.m_testCount);
			}
			break;
		case TestSuite::ControlState::CreatingReports:
			{
				text = tr("Creating reports...\n");
			}
			break;
		default:
			Q_ASSERT(false);
		}
	}

	if (text != m_statusIndicator->text())
	{
		m_statusIndicator->setText(text);
	}
	if (styleSheet != m_statusIndicator->styleSheet())
	{
		m_statusIndicator->setStyleSheet(styleSheet);
	}

	return;
}

bool TestSuiteMainWindow::eventFilter(QObject* object, QEvent* event)
{
	if ((object == m_statusBarConfigConnection || object == m_statusBarAppDataConnection || object == m_statusBarTuningConnection) &&
		event->type() == QEvent::MouseButtonPress)
	{
		showStatistics();
	}

	if (object == m_statusBarLogAlerts && m_statusBarLogAlerts->text().isEmpty() == false && event->type() == QEvent::MouseButtonPress)
	{
		showAppLog();
	}

	return QWidget::eventFilter(object, event);
}

void TestSuiteMainWindow::closeEvent(QCloseEvent* event)
{
	if (m_testSuite.isRunning() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Please stop testing before closing the application."));

		event->ignore();
		return;
	}

	event->accept();
}

void TestSuiteMainWindow::timerEvent(QTimerEvent* event)
{
	assert(event);
	// Update status bar
	//
	if (event->timerId() == m_mainWindowTimerId_250ms)
	{
		updateStatusBar();

		updateStatusIndicator();
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

	// Ask for a password
	//
	QString userName;
	QString password;

	if (m_configuration.login == true)
	{
		ClientLib::TuningUserManager userManager;
		userManager.setConfiguration(true, m_configuration.userAccounts, true, 120, m_configuration.matsUsers.users());

		if (userManager.login(this) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Tests execution failed: authorization failed!"));
			return;
		}

		userName = userManager.userName();
		password = userManager.password();
	}

	// Create a list of tests user has selected to run
	//
	TestSuite::TestScriptSelection selection = m_testListWidget->getTestScriptSelection();
	if (selection.isEmpty() == true)
	{
		QMessageBox::warning(this, qAppName(), tr("Please choose at least one test to run."));
		return;
	}

	m_testSuite.testLog().clear();
	m_testLogTabPage->clearOutputWidget();
	m_testListWidget->clearTestsResults();

	m_tabWidget->setCurrentIndex(0);

	// Run tests
	//
	TestSuite::ControlParams controlParams{selection.selectedFiles(),
										   {}, // Reports path
										   selection,
										   userName,
										   password};

	bool ok = m_testSuite.execute(m_testScriptsStorage, controlParams);
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

void TestSuiteMainWindow::on_m_report_clicked()
{
	if (m_testSuite.testLog().empty() == true)
	{
		if (QMessageBox::question(this, qAppName(), tr("Test log is empty. Do you want to load test log from file?")) == QMessageBox::Yes)
		{
			if (loadTestLog() == false)
			{
				return;
			}
		}
		else
		{
			QMessageBox::critical(this, qAppName(), tr("No data exist for the report!"));
			return;
		}
	}

	if (m_configData.reportTemplates.templates().size() == 1)
	{
		on_m_single_report_clicked();
	}
	else
	{
		DialogReport d(m_configData.reportTemplates, m_testSuite.testLog(), this);
		d.exec();
	}
}

void TestSuiteMainWindow::on_m_single_report_clicked()
{
	if (m_configData.reportTemplates.templates().size() != 1)
	{
		Q_ASSERT(false);
		return;
	}

	onGenerateReport(m_configData.reportTemplates.templates()[0].caption());
}

void TestSuiteMainWindow::onSaveTestLog()
{
	saveTestLog();
}

void TestSuiteMainWindow::onLoadTestLog()
{
	loadTestLog();
}

void TestSuiteMainWindow::onClearTestLog()
{
	m_testSuite.testLog().clear();
	m_testLogTabPage->clearOutputWidget();
}

void TestSuiteMainWindow::onSettings()
{
	TestSuiteDialogSettings d(m_translator, this);
	d.setSettings(AppConfigSettings::instance().data());

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		// --
		//
		bool needReconnect = false;
		bool needReloadScripts = false;

		AppConfigSettings prevSettings = AppConfigSettings().instance();

		// --
		//
		AppConfigSettings::instance().setData(d.settings());
		AppConfigSettings::instance().save();

		const auto& newSettings = AppConfigSettings().instance();

		if (prevSettings.librarySettings().instanceStrId() != newSettings.librarySettings().instanceStrId() ||
			prevSettings.librarySettings().configuratorAddress1() != newSettings.librarySettings().configuratorAddress1() ||
			prevSettings.librarySettings().configuratorAddress2() != newSettings.librarySettings().configuratorAddress2())
		{
			needReconnect = true;
		}

		if (prevSettings.localScriptsPath() != newSettings.localScriptsPath() ||
			prevSettings.useLocalScriptsPath() != newSettings.useLocalScriptsPath())
		{
			needReloadScripts = true;
		}

		// --
		//
		if (prevSettings.useLocalScriptsPath() == true && newSettings.useLocalScriptsPath() == false)
		{
			// Tests are NOT loaded from local folder now - clear them
			//
			loadScriptsFromConfiguration();
		}
		else
		{
			if (prevSettings.useLocalScriptsPath() == false && newSettings.useLocalScriptsPath() == true)
			{
				// Tests ARE loaded from local folder now - load them
				//
				loadScriptsFromLocalPath();
			}
		}

		// Reconnect
		//
		if (needReconnect == true)
		{
			m_configController.setConnectionParams(newSettings.librarySettings().instanceStrId(),
												   newSettings.librarySettings().configuratorAddress1(),
												   newSettings.librarySettings().configuratorAddress2());
		}

		m_reloadTestsScriptsAction->setVisible(newSettings.useLocalScriptsPath() == true);

		setWindowTitle(tr("%1 - %2").arg(qAppName()).arg(newSettings.librarySettings().instanceStrId()));

		return;
	}

	return;
}

void TestSuiteMainWindow::showStatistics()
{
	if (m_dialogStatistics == nullptr)
	{
		m_dialogStatistics = new SchemaClientLib::DialogTcpStatistics(this);
		m_dialogStatistics->show();

		auto f = [this]() -> void
		{
			m_dialogStatistics = nullptr;
		};

		connect(m_dialogStatistics, &SchemaClientLib::DialogTcpStatistics::dialogClosed, this, f);
	}
	else
	{
		m_dialogStatistics->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogStatistics);
}

void TestSuiteMainWindow::showDataSources()
{
	DialogDataSources::create(m_configController, &m_appLog, this);
}

void TestSuiteMainWindow::showAppLog()
{
	Log::LogFileDialog::view(m_appLog, this);
}

void TestSuiteMainWindow::showAboutQt()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void TestSuiteMainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to run application logic tests.");
	UiLib::DialogAbout::show(this, text, ":/Logo/RadiyLogo.png");
}

void TestSuiteMainWindow::onTestsScriptsReload()
{
	// Reload scripts that displayed by the user interface. Actual executed scripts are loaded at testing start.
	//
	if (AppConfigSettings::instance().useLocalScriptsPath() == true)
	{
		loadScriptsFromLocalPath();
	}
	else
	{
		loadScriptsFromConfiguration();
	}

	updateTestViewTabPages();
}

void TestSuiteMainWindow::onShowTestContents(const QString& scriptName, const QString& functionName)
{
	const TestSuite::TestScript& script = m_testScriptsStorage.script(::calcHash(scriptName));

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
		if (p->script().fileNameHash() == script.fileNameHash())
		{
			m_tabWidget->setCurrentIndex(i);
			if (functionName.isEmpty() == false)
			{
				p->scrollToFunction(functionName);
			}
			return;
		}
	}

	TestViewTabPage* p = new TestViewTabPage(script, this);
	if (functionName.isEmpty() == false)
	{
		p->scrollToFunction(functionName);
	}
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

	m_tabWidget->removeTab(index);
	w->deleteLater();
}

void TestSuiteMainWindow::onGenerateReport(const QString& caption)
{
	if (m_testSuite.testLog().empty() == true)
	{
		if (QMessageBox::question(this, qAppName(), tr("Test log is empty. Do you want to load test log from file?")) == QMessageBox::Yes)
		{
			if (loadTestLog() == false)
			{
				return;
			}
		}
		else
		{
			QMessageBox::critical(this, qAppName(), tr("No data exist for the report!"));
			return;
		}
	}

	TestSuite::TestReport::generateReport(m_configData.reportTemplates, m_testSuite.testLog(), caption, this);

	return;
}

void TestSuiteMainWindow::viewGlobalScript()
{
	int count = m_testScriptsStorage.count();
	for (int i = 0; i < count; i++)
	{
		auto& script = m_testScriptsStorage.script(i);
		if (script.isGlobalScript() == true)
		{
			onShowTestContents(script.fileName(), QString());
		}
	}
}

void TestSuiteMainWindow::onConfigurationArrived()
{
	m_configuration = m_configController.configuration();
	m_configData = m_configController.configData();

	if (AppConfigSettings::instance().useLocalScriptsPath() == false)
	{
		loadScriptsFromConfiguration();
	}
	else
	{
		loadScriptsFromLocalPath();
	}

	updateTestViewTabPages();

	updateReportActions();

	updateActionsState();

	return;
}

void TestSuiteMainWindow::onTestingFinished(int /*result*/)
{
	updateActionsState();
}

void TestSuiteMainWindow::onGlobalPermissionChanged(bool result)
{
	if (result == false && m_testSuite.isRunning() == true)
	{
		TestSuite::ControlStatus testStatus = m_testSuite.testStatus();

		m_testSuite.testLog().writeError(testStatus.m_scriptFile + tr(": no global permission: script terminated."), QString());

		m_testSuite.stop();
	}

	updateActionsState();
}

void TestSuiteMainWindow::onScriptPermissionChanged(QString scriptFileName, bool permission)
{
	TestSuite::ControlStatus testStatus = m_testSuite.testStatus();

	if (permission == false && m_testSuite.isRunning() == true && testStatus.m_state == TestSuite::ControlState::RunningTests &&
		scriptFileName == testStatus.m_scriptFile)
	{
		m_testSuite.testLog().writeError(testStatus.m_scriptFile + tr(": no local permission: script terminated."), QString());

		m_testSuite.stop();
	}

	updateActionsState();
}

TestSuiteMainWindow* theMainWindow = nullptr;
