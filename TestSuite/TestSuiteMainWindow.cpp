#include "TestSuiteMainWindow.h"
#include "ui_TestSuiteMainWindow.h"
#include "AppConfigSettings.h"
#include "TestSuiteDialogSettings.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

TestSuiteMainWindow::TestSuiteMainWindow(const SoftwareInfo& softwareInfo, QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::TestSuiteMainWindow),
	m_logFile(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + theSettings.librarySettings().instanceStrId()),
	m_testSuite(softwareInfo, theSettings.librarySettings(), &m_logFile, &m_outputLog)
{
	ui->setupUi(this);

	ui->testsTree->setRootIsDecorated(false);
	QStringList headerLabels;
	headerLabels << tr("Test");
	ui->testsTree->setColumnCount(static_cast<int>(headerLabels.size()));
	ui->testsTree->setHeaderLabels(headerLabels);
	ui->testsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//connect(&m_testSuite.configController(), &TestSuite::TestSuiteConfigController::configurationArrived, this, &TestSuiteMainWindow::onConfigurationArrived);
	connect(&m_testSuite, &TestSuite::TestSuite::finished, this, &TestSuiteMainWindow::onTestingFinished);

	connect(&m_logFile, &TestSuiteLogFile::errorArrived, this, &TestSuiteMainWindow::onLogError, Qt::QueuedConnection);
	connect(&m_logFile, &TestSuiteLogFile::warningArrived, this, &TestSuiteMainWindow::onLogWarning, Qt::QueuedConnection);
	connect(&m_logFile, &TestSuiteLogFile::messageArrived, this, &TestSuiteMainWindow::onLogMessage, Qt::QueuedConnection);
	connect(&m_logFile, &TestSuiteLogFile::textArrived, this, &TestSuiteMainWindow::onLogText, Qt::QueuedConnection);

	connect(&m_outputLog, &TestSuiteOutputLog::errorArrived, this, &TestSuiteMainWindow::onOutputLogError, Qt::QueuedConnection);
	connect(&m_outputLog, &TestSuiteOutputLog::warningArrived, this, &TestSuiteMainWindow::onOutputLogWarning, Qt::QueuedConnection);
	connect(&m_outputLog, &TestSuiteOutputLog::messageArrived, this, &TestSuiteMainWindow::onOutputLogMessage, Qt::QueuedConnection);

	createActions();
	createMenu();
	createStatusBar();

//	if (theSettings.librarySettings().loadScriptsFromPath() == true)
//	{
//		fillTestsTree();
//	}

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
	connect(m_pExitAction, &QAction::triggered, this, &TestSuiteMainWindow::exit);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	//m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &TestSuiteMainWindow::showSettings);
/*
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
	//pServiceMenu->addAction(m_pPresetEditorAction);
	pServiceMenu->addSeparator();
	pServiceMenu->addAction(m_pSettingsAction);


	/*
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
	pHelpMenu->addAction(m_pAboutAction);*/

}

void TestSuiteMainWindow::createStatusBar()
{
/*
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
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);*/
}

void TestSuiteMainWindow::fillTestsTree()
{
//	ui->testsTree->clear();

//	QStringList l = m_testLibrary.testScriptsStorage().scriptList();

//	for (const QString& s : l)
//	{
//		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QDir::toNativeSeparators(s));
//		item->setCheckState(0, Qt::Checked);
//		ui->testsTree->addTopLevelItem(item);
//	}
}

void TestSuiteMainWindow::exit()
{
	close();
}

void TestSuiteMainWindow::on_m_run_clicked()
{
//	m_testLibrary.execute();
}

void TestSuiteMainWindow::on_m_stop_clicked()
{
//	m_testLibrary.stop();
}

void TestSuiteMainWindow::newLogItem(const TestSuite::TestLogItem& item)
{
//	ui->resultsLog->moveCursor (QTextCursor::End);
//	ui->resultsLog->insertPlainText(item.toText());
//	ui->resultsLog->insertPlainText("\n");
//	ui->resultsLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onTestingFinished(int result)
{

}

void TestSuiteMainWindow::onConfigurationArrived()
{
	fillTestsTree();

	return;
}

void TestSuiteMainWindow::onLogError(const QString& errMsg)
{
	ui->outputLog->moveCursor (QTextCursor::End);
	ui->outputLog->insertPlainText(errMsg);
	ui->outputLog->insertPlainText("\n");
	ui->outputLog->moveCursor (QTextCursor::End);

	QMessageBox::critical(this, qAppName(), errMsg);
	return;
}

void TestSuiteMainWindow::onLogWarning(const QString& msg)
{
	ui->outputLog->moveCursor (QTextCursor::End);
	ui->outputLog->insertPlainText(msg);
	ui->outputLog->insertPlainText("\n");
	ui->outputLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onLogMessage(const QString& msg)
{
	ui->outputLog->moveCursor (QTextCursor::End);
	ui->outputLog->insertPlainText(msg);
	ui->outputLog->insertPlainText("\n");
	ui->outputLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onLogText(const QString& msg)
{
	ui->outputLog->moveCursor (QTextCursor::End);
	ui->outputLog->insertPlainText(msg);
	ui->outputLog->insertPlainText("\n");
	ui->outputLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onOutputLogError(const QString& errMsg)
{
	ui->resultsLog->moveCursor (QTextCursor::End);
	ui->resultsLog->insertPlainText(errMsg);
	ui->resultsLog->insertPlainText("\n");
	ui->resultsLog->moveCursor (QTextCursor::End);

	QMessageBox::critical(this, qAppName(), errMsg);
	return;
}

void TestSuiteMainWindow::onOutputLogWarning(const QString& msg)
{
	ui->resultsLog->moveCursor (QTextCursor::End);
	ui->resultsLog->insertPlainText(msg);
	ui->resultsLog->insertPlainText("\n");
	ui->resultsLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::onOutputLogMessage(const QString& msg)
{
	ui->resultsLog->moveCursor (QTextCursor::End);
	ui->resultsLog->insertPlainText(msg);
	ui->resultsLog->insertPlainText("\n");
	ui->resultsLog->moveCursor (QTextCursor::End);
}

void TestSuiteMainWindow::showSettings()
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
		theSettings = d.settings();
		theSettings.StoreSystem();
		theSettings.StoreUser();

		// Reconnect
		//
		if (needReconnect == true)
		{
//			m_testSuite.configController().setConnectionParams(theSettings.librarySettings().instanceStrId(),
//															   theSettings.librarySettings().configuratorAddress1(),
//															   theSettings.librarySettings().configuratorAddress2());
		}

		//setWindowTitle(MonitorAppSettings::instance().windowCaption());

		return;
	}

	return;
}

TestSuiteMainWindow* theMainWindow = nullptr;
