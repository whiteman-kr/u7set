#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "MonitorSchemaWidget.h"
#include "DialogSignalSearch.h"
#include "DialogSignalSnapshot.h"
#include "MonitorArchive.h"
#include "./Trend/MonitorTrends.h"
#include "../VFrame30/Schema.h"
#include "DialogDataSources.h"
#include "../lib/Ui/UiTools.h"
#include "../lib/Ui/DialogAbout.h"

const QString MonitorMainWindow::m_monitorSingleInstanceKey = "MonitorInstanceCheckerKey";

MonitorMainWindow::MonitorMainWindow(const SoftwareInfo& softwareInfo, QWidget* parent) :
	QMainWindow(parent),
	m_configController(softwareInfo, theSettings.configuratorAddress1(), theSettings.configuratorAddress2()),
	m_schemaManager(&m_configController),
	m_dialogAlert(this),
	m_LogFile(qAppName())
{
	qDebug() << Q_FUNC_INFO;

	connect(&m_configController, &MonitorConfigController::configurationArrived, this, &MonitorMainWindow::slot_configurationArrived);
	connect(&m_configController, &MonitorConfigController::unknownClient, this, &MonitorMainWindow::slot_unknownClient);

	// TcpSignalClient
	//
	HostAddressPort fakeAddress(QLatin1String("0.0.0.0"), 0);
	m_tcpSignalClient = new TcpSignalClient(&m_configController, fakeAddress, fakeAddress);

	m_tcpClientThread = new SimpleThread(m_tcpSignalClient);
	m_tcpClientThread->start();

	// TcpSignalClient
	//
	m_tcpSignalRecents = new TcpSignalRecents(&m_configController, fakeAddress, fakeAddress);

	m_tcpRecentsThread = new SimpleThread(m_tcpSignalRecents);
	m_tcpRecentsThread->start();

	connect(&theSignals, &AppSignalManager::addSignalToPriorityList, m_tcpSignalRecents, &TcpSignalRecents::addSignal, Qt::QueuedConnection);
	connect(&theSignals, &AppSignalManager::addSignalsToPriorityList, m_tcpSignalRecents, &TcpSignalRecents::addSignals, Qt::QueuedConnection);

	// TcpSourcesStateClient
	//
	m_tcpSourcesStateClient = new TcpAppSourcesState(&m_configController, fakeAddress, fakeAddress);

	m_sourcesStateClientThread = new SimpleThread(m_tcpSourcesStateClient);
	m_sourcesStateClientThread->start();

	// Log file
	//

	m_LogFile.writeText("---");
	m_LogFile.writeMessage(tr("Application started."));

	// DialogAlert

	connect(&m_LogFile, &Log::LogFile::alertArrived, &m_dialogAlert, &DialogAlert::onAlertArrived);
	connect(&m_LogFile, &Log::LogFile::writeFailure, &m_dialogAlert, &DialogAlert::onAlertArrived);

	// Creating signals controllers for VFrame30
	//
	m_appSignalController = std::make_unique<VFrame30::AppSignalController>(&theSignals);
	m_tuningController = std::make_unique<VFrame30::TuningController>(&theTuningSignals, nullptr);

	// --
	//
	MonitorCentralWidget* monitorCentralWidget = new MonitorCentralWidget(&m_schemaManager,
																		  m_appSignalController.get(),
																		  m_tuningController.get());
	setCentralWidget(monitorCentralWidget);

	// Create Menus, ToolBars, StatusBar
	//
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	// --
	//
	setMinimumSize(500, 300);
	restoreWindowState();

	// --
	//
	connect(monitorCentralWidget, &MonitorCentralWidget::signal_actionCloseTabUpdated, this,
		[this](bool allowed)
		{
			Q_ASSERT(m_closeTabAction);
			m_closeTabAction->setEnabled(allowed);
		});

	connect(monitorCentralWidget, &MonitorCentralWidget::signal_historyChanged, this, &MonitorMainWindow::slot_historyChanged);

	connect(m_selectSchemaWidget, &SelectSchemaWidget::selectionChanged, monitorCentralWidget, &MonitorCentralWidget::slot_selectSchemaForCurrentTab);

	// --
	//
	centralWidget()->show();

	m_configController.start();

	m_updateStatusBarTimerId = startTimer(100);

	// Try attach memory segment, that keep information
	// about instance status
	//
	m_instanceTimer = new QTimer(this);
	m_instanceTimer->start(100);

	connect(m_instanceTimer, &QTimer::timeout, this, &MonitorMainWindow::checkMonitorSingleInstance);

	m_appInstanceSharedMemory.setKey(MonitorMainWindow::getInstanceKey());
	m_appInstanceSharedMemory.attach();

	return;
}

MonitorMainWindow::~MonitorMainWindow()
{
	qDebug() << Q_FUNC_INFO;

	m_tcpClientThread->quitAndWait(10000);
	delete m_tcpClientThread;

	if (m_tcpRecentsThread)
	{
		m_tcpRecentsThread->quitAndWait(10000);
		delete m_tcpRecentsThread;
	}

	if (m_sourcesStateClientThread != nullptr)
	{
		m_sourcesStateClientThread->quitAndWait(10000);
		delete m_sourcesStateClientThread;
	}

	if (m_tuningTcpClientThread != nullptr)
	{
		m_tuningTcpClientThread->quitAndWait(10000);
		delete m_tuningTcpClientThread;
	}

	return;
}

void MonitorMainWindow::closeEvent(QCloseEvent* e)
{
	saveWindowState();
	e->accept();

	return;
}

void MonitorMainWindow::timerEvent(QTimerEvent* event)
{
	Q_ASSERT(event);

	if (event->timerId() == m_updateStatusBarTimerId)
	{
		updateStatusBar();
	}

	return;
}

void MonitorMainWindow::showEvent(QShowEvent*)
{
	showLogo();
	return;
}

bool MonitorMainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_statusBarLogAlerts &&
			event->type() == QEvent::MouseButtonPress &&
			m_statusBarLogAlerts->text().isEmpty() == false
			)
	{
		showLog();
	}

	return QWidget::eventFilter(object, event);
}

void MonitorMainWindow::showTrends(const std::vector<AppSignalParam>& appSignals)
{
	MonitorTrends::startTrendApp(&m_configController, appSignals, this);
}

void MonitorMainWindow::saveWindowState()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theSettings.writeUserScope();

	return;
}

void MonitorMainWindow::restoreWindowState()
{
	move(theSettings.m_mainWindowPos);
	restoreGeometry(theSettings.m_mainWindowGeometry);
	restoreState(theSettings.m_mainWindowState);

	// Ensure widget is visible
	//
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		frameGeometry().height() > screenRect.height()))
	{
		resize(static_cast<int>(screenRect.width() * 0.7),
			   static_cast<int>(screenRect.height() * 0.7));
	}

	return;
}

void MonitorMainWindow::showLogo()
{
	Q_ASSERT(m_logoLabel);

	if (m_logoImage.isNull() == true)
	{
		m_logoLabel->clear();

		return;
	}

	QImage logo = m_logoImage;

	// Get toolbar content height
	//
	int toolBarSpacing = 0;

	if (QLayout* toolBarLayout = m_toolBar->layout();
		toolBarLayout != nullptr)
	{
		toolBarSpacing = toolBarLayout->spacing();
	}

	int logoMaxHeight = m_toolBar->size().height() - toolBarSpacing * 2;

	// Scale the logo image
	//
	if (logo.height() > logoMaxHeight)
	{
		logo = logo.scaledToHeight(logoMaxHeight, Qt::SmoothTransformation);
	}

	// Show logo if it was enabled in settings
	//
	if (theSettings.showLogo() == true)
	{
		m_logoLabel->setPixmap(QPixmap::fromImage(logo));
	}
	else
	{
		m_logoLabel->clear();
	}

	return;
}

QString MonitorMainWindow::getInstanceKey()
{
	return m_monitorSingleInstanceKey;
}

void MonitorMainWindow::createActions()
{
	m_pExitAction = new QAction(tr("Exit"), this);
	m_pExitAction->setStatusTip(tr("Quit the application"));
	m_pExitAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_pExitAction->setShortcut(QKeySequence::Quit);
	m_pExitAction->setShortcutContext(Qt::ApplicationShortcut);
	m_pExitAction->setEnabled(true);
	connect(m_pExitAction, &QAction::triggered, this, &MonitorMainWindow::exit);

	m_pStatisticsAction = new QAction(tr("Connection Statistics..."), this);
	m_pStatisticsAction->setStatusTip(tr("View Connection Statistics"));
	m_pStatisticsAction->setIcon(QIcon(":/Images/Images/NetworkConnections.svg"));
	m_pStatisticsAction->setEnabled(true);
	connect(m_pStatisticsAction, &QAction::triggered, this, &MonitorMainWindow::showStatistics);

	m_pDataSourcesAction = new QAction(tr("Data Sources..."), this);
	m_pDataSourcesAction->setStatusTip(tr("View Data Sources"));
	m_pDataSourcesAction->setIcon(QIcon(":/Images/Images/AppDataSources.svg"));
	m_pDataSourcesAction->setEnabled(true);
	connect(m_pDataSourcesAction, &QAction::triggered, this, &MonitorMainWindow::showDataSources);

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MonitorMainWindow::showSettings);

	m_manualMatsAction = new QAction(tr("MATS User Manual"), this);
	m_manualMatsAction->setStatusTip(tr("Show MATS User Manual"));
	connect(m_manualMatsAction, &QAction::triggered, this, &MonitorMainWindow::showMatsUserManual);

	m_pDebugAction = new QAction(tr("Debug..."), this);
	m_pDebugAction->setStatusTip(tr("Perform some debug actions, don't run it!"));
	m_pDebugAction->setEnabled(true);
	connect(m_pDebugAction, &QAction::triggered, this, &MonitorMainWindow::debug);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	connect(m_pLogAction, &QAction::triggered, this, &MonitorMainWindow::showLog);

	m_pAboutAction = new QAction(tr("About..."), this);
	m_pAboutAction->setStatusTip(tr("Show application information"));
	m_pAboutAction->setIcon(QIcon(":/Images/Images/About.svg"));
	//m_pAboutAction->setEnabled(true);
	connect(m_pAboutAction, &QAction::triggered, this, &MonitorMainWindow::showAbout);

	m_newTabAction = new QAction(tr("New Tab"), this);
	m_newTabAction->setStatusTip(tr("Open current schema in new tab page"));
	m_newTabAction->setIcon(QIcon(":/Images/Images/NewSchema.svg"));
	m_newTabAction->setEnabled(true);
	QList<QKeySequence> newTabShortcuts;
	newTabShortcuts << QKeySequence::AddTab;
	newTabShortcuts << QKeySequence::New;
	m_newTabAction->setShortcuts(newTabShortcuts);
	connect(m_newTabAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_newTab);

	m_closeTabAction = new QAction(tr("Close Tab"), this);
	m_closeTabAction->setStatusTip(tr("Close current tab page"));
	m_closeTabAction->setIcon(QIcon(":/Images/Images/Close.svg"));
	m_closeTabAction->setEnabled(true);
	m_closeTabAction->setShortcuts(QKeySequence::Close);
	m_closeTabAction->setEnabled(monitorCentralWidget()->count() > 1);
	connect(m_closeTabAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_closeCurrentTab);

	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setStatusTip(tr("Zoom in schema view"));
	m_zoomInAction->setIcon(QIcon(":/Images/Images/ZoomIn.svg"));
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_zoomIn);

	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setStatusTip(tr("Zoom out schema view"));
	m_zoomOutAction->setIcon(QIcon(":/Images/Images/ZoomOut.svg"));
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_zoomOut);

	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setStatusTip(tr("Set zoom to 100%"));
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_zoom100);

	m_historyBack = new QAction(tr("Go Back"), this);
	m_historyBack->setStatusTip(tr("Click to go back"));
	m_historyBack->setIcon(QIcon(":/Images/Images/Backward.svg"));
	m_historyBack->setEnabled(false);
	m_historyBack->setShortcut(QKeySequence::Back);
	connect(m_historyBack, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_historyBack);

	m_historyForward = new QAction(tr("Go Forward"), this);
	m_historyForward->setStatusTip(tr("Click to go forward"));
	m_historyForward->setIcon(QIcon(":/Images/Images/Forward.svg"));
	m_historyForward->setEnabled(false);
	m_historyForward->setShortcut(QKeySequence::Forward);
	connect(m_historyForward, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_historyForward);

	m_archiveAction = new QAction(tr("Archive"), this);
	m_archiveAction->setIcon(QIcon(":/Images/Images/Archive.svg"));
	m_archiveAction->setEnabled(true);
	m_archiveAction->setData(QVariant("IAmIndependentArchive"));	// This is required to find this action in MonitorToolBar for drag and drop
	connect(m_archiveAction, &QAction::triggered, this, &MonitorMainWindow::slot_archive);

	m_trendsAction = new QAction(tr("Trends"), this);
	m_trendsAction->setIcon(QIcon(":/Images/Images/Trends.svg"));
	m_trendsAction->setEnabled(true);
	m_trendsAction->setData(QVariant("IAmIndependentTrend"));	// This is required to find this action in MonitorToolBar for drag and drop
	connect(m_trendsAction, &QAction::triggered, this, &MonitorMainWindow::slot_trends);

	m_signalSnapshotAction = new QAction(tr("Signals Snapshot"), this);
	m_signalSnapshotAction->setStatusTip(tr("View signals state in real time"));
	m_signalSnapshotAction->setIcon(QIcon(":/Images/Images/Snapshot.svg"));
	m_signalSnapshotAction->setEnabled(true);
	connect(m_signalSnapshotAction, &QAction::triggered, this, &MonitorMainWindow::slot_signalSnapshot);

	m_findSignalAction = new QAction(tr("Find Signal"), this);
	m_findSignalAction->setStatusTip(tr("Find signal by it's ID"));
	m_findSignalAction->setIcon(QIcon(":/Images/Images/FindSignal.svg"));
	m_findSignalAction->setEnabled(true);
	m_findSignalAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
	connect(m_findSignalAction, &QAction::triggered, this, &MonitorMainWindow::slot_findSignal);

	return;
}

void MonitorMainWindow::createMenus()
{
	// File
	//
	QMenu* pFileMenu = menuBar()->addMenu(tr("&File"));

	pFileMenu->addAction(m_pExitAction);

	// Schema
	//
	QMenu* schemaMenu = menuBar()->addMenu(tr("&Schema"));

	schemaMenu->addAction(m_newTabAction);
	schemaMenu->addAction(m_closeTabAction);

	// View
	//
	QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

	viewMenu->addAction(m_zoomInAction);
	viewMenu->addAction(m_zoomOutAction);
	viewMenu->addAction(m_zoom100Action);
	viewMenu->addSeparator();

	viewMenu->addAction(m_historyForward);
	viewMenu->addAction(m_historyBack );


	// Tools
	//
	QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));

	toolsMenu->addAction(m_archiveAction);
	toolsMenu->addAction(m_trendsAction);

	toolsMenu->addSeparator();
	toolsMenu->addAction(m_signalSnapshotAction);
	toolsMenu->addAction(m_findSignalAction);

	toolsMenu->addSeparator();
	toolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* helpMenu = menuBar()->addMenu(tr("&?"));

#ifdef Q_DEBUG
	//helpMenu->addAction(m_pDebugAction);
#endif	// Q_DEBUG


	helpMenu->addAction(m_manualMatsAction);
	helpMenu->addAction(m_pAboutAction);
	helpMenu->addSeparator();

	helpMenu->addAction(m_pDataSourcesAction);
	helpMenu->addAction(m_pStatisticsAction);

	helpMenu->addSeparator();
	helpMenu->addAction(m_pLogAction);

	return;
}

void MonitorMainWindow::createToolBars()
{
	m_toolBar = new MonitorToolBar("ToolBar", this);
	m_toolBar->setObjectName("MonitorMainToolBar");

	m_toolBar->addAction(m_newTabAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_archiveAction);
	m_toolBar->addAction(m_trendsAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_signalSnapshotAction);
	m_toolBar->addAction(m_findSignalAction);

	m_toolBar->addSeparator();
	m_selectSchemaWidget = new SelectSchemaWidget(&m_configController, monitorCentralWidget());
	m_selectSchemaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_selectSchemaWidget->setMaximumWidth(1280);
	m_toolBar->addWidget(m_selectSchemaWidget);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_historyBack);
	m_toolBar->addAction(m_historyForward);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_zoomInAction);
	m_toolBar->addAction(m_zoomOutAction);

	// Spacer between actions and logo
	//
	m_spacer = new QWidget(this);
	m_spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	m_toolBar->addWidget(m_spacer);

	// Create logo for toolbar
	//
	m_logoLabel = new QLabel(this);
	m_toolBar->addWidget(m_logoLabel);
	this->addToolBar(Qt::TopToolBarArea, m_toolBar);

	int space = m_toolBar->sizeHint().height() / 6;
	m_toolBar->setStyleSheet(QString("QToolBar{spacing:%1;padding:%1;}").arg(space));

	return;
}

void MonitorMainWindow::createStatusBar()
{
	m_statusBarInfo = new QLabel();
	m_statusBarInfo->setAlignment(Qt::AlignLeft);
	m_statusBarInfo->setIndent(3);

	m_statusBarConfigConnection = new QLabel();
	m_statusBarConfigConnection->setAlignment(Qt::AlignHCenter);
	m_statusBarConfigConnection->setMinimumWidth(100);

	m_statusBarAppDataConnection = new QLabel();
	m_statusBarAppDataConnection->setAlignment(Qt::AlignHCenter);
	m_statusBarAppDataConnection->setMinimumWidth(100);

	m_statusBarTuningConnection = new QLabel();
	m_statusBarTuningConnection->setAlignment(Qt::AlignHCenter);
	m_statusBarTuningConnection->setMinimumWidth(100);

	m_statusBarProjectInfo = new QLabel;
	m_statusBarProjectInfo->setAlignment(Qt::AlignHCenter);
	m_statusBarProjectInfo->setMinimumWidth(100);

	m_statusBarLogAlerts = new QLabel;
	m_statusBarLogAlerts->setAlignment(Qt::AlignHCenter);
	m_statusBarLogAlerts->setMinimumWidth(100);
	m_statusBarLogAlerts->setToolTip(tr("Error and warning counters in the log (click to view log)"));
	m_statusBarLogAlerts->installEventFilter(this);

	// --
	//
	statusBar()->addWidget(m_statusBarInfo, 1);
	statusBar()->addPermanentWidget(m_statusBarConfigConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarAppDataConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarTuningConnection, 0);
	statusBar()->addPermanentWidget(m_statusBarProjectInfo, 0);
	statusBar()->addPermanentWidget(m_statusBarLogAlerts, 0);

	return;
}

MonitorCentralWidget* MonitorMainWindow::monitorCentralWidget()
{
	MonitorCentralWidget* centralWidget = dynamic_cast<MonitorCentralWidget*>(QMainWindow::centralWidget());
	Q_ASSERT(centralWidget != nullptr);

	return centralWidget;
}

void MonitorMainWindow::updateStatusBar()
{
	// Update status bar
	//
	Q_ASSERT(m_statusBarConfigConnection);
	Q_ASSERT(m_statusBarAppDataConnection);
	Q_ASSERT(m_statusBarTuningConnection);

	// ConfigService connection
	//
	{
		Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();

		showSoftwareConnection(tr("Configuration Service"), tr("ConfigService"),
							   confiConnState,
							   theSettings.configuratorAddress1(),
							   theSettings.configuratorAddress2(),
							   m_statusBarConfigConnection);
	}

	// AppDataService connection
	//
	if  (m_tcpSignalClient != nullptr)
	{
		Tcp::ConnectionState signalClientState =  m_tcpSignalClient->getConnectionState();

		showSoftwareConnection(tr("Application Data Service"), tr("AppDataService"),
							   signalClientState,
							   m_tcpSignalClient->serverAddressPort1(),
							   m_tcpSignalClient->serverAddressPort2(),
							   m_statusBarAppDataConnection);
	}

	// TuningService connection
	//
	if  (m_configController.configuration().tuningEnabled == true && m_tuningTcpClient != nullptr)
	{
		Tcp::ConnectionState tuningClientState = m_tuningTcpClient->getConnectionState();

		showSoftwareConnection(tr("Tuning Service"), tr("TuningService"),
							   tuningClientState,
							   m_tuningTcpClient->serverAddressPort1(),
							   m_tuningTcpClient->serverAddressPort2(),
							   m_statusBarTuningConnection);
	}

	// BuildNo
	//
	{
		QString text = QString(" Project: %1   Build: %2  ")
				.arg(m_configController.configuration().project)
				.arg(m_configController.configuration().buildNo);

		m_statusBarProjectInfo->setText(text);
	}

	if ((m_logErrorsCounter != m_LogFile.errorAckCounter() || m_logWarningsCounter != m_LogFile.warningAckCounter()))
	{
		m_logErrorsCounter = m_LogFile.errorAckCounter();
		m_logWarningsCounter = m_LogFile.warningAckCounter();

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

	return;
}

void MonitorMainWindow::showSoftwareConnection(const QString& caption, const QString& shortCaption, Tcp::ConnectionState connectionState, HostAddressPort portPrimary, HostAddressPort portSecondary, QLabel* label)
{
	QString tooltipText = tr("%1\r\n\r\n").arg(caption);
	tooltipText.append(tr("Address (primary): %1\r\n").arg(portPrimary.addressPortStr()));
	tooltipText.append(tr("Address (secondary): %1\r\n\r\n").arg(portSecondary.addressPortStr()));
	tooltipText.append(tr("Address (current): %1\r\n").arg(connectionState.peerAddr.addressPortStr()));

	QString statusText;

	if (connectionState.isConnected == true)
	{
		statusText = tr(" %1: %2 ").arg(shortCaption).arg(connectionState.replyCount);
		tooltipText.append(tr("Connection: established"));
	}
	else
	{
		statusText = tr(" %1: no connection ").arg(shortCaption);
		tooltipText.append(tr("Connection: no connection"));
	}

	if (label->text() != statusText)
	{
		label->setText(statusText);
	}
	if (label->toolTip() != tooltipText)
	{
		label->setToolTip(tooltipText);
	}
}

void MonitorMainWindow::exit()
{
	close();
}

void MonitorMainWindow::showLog()
{
	m_LogFile.view(this);
}

void MonitorMainWindow::showDataSources()
{
	if (m_dialogDataSources == nullptr)
	{
		m_dialogDataSources = new DialogDataSources(m_tcpSourcesStateClient,
													m_configController.configuration().tuningEnabled,
													m_tuningTcpClient,
													false,
													this);
		m_dialogDataSources->show();

		auto f = [this]() -> void
			{
				m_dialogDataSources = nullptr;
			};

		connect(m_dialogDataSources, &DialogDataSources::dialogClosed, this, f);
	}
	else
	{
		m_dialogDataSources->activateWindow();
	}

	UiTools::adjustDialogPlacement(m_dialogDataSources);
}


void MonitorMainWindow::showSettings()
{
	DialogSettings d(this);
	d.setSettings(theSettings);

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		// --
		//
		bool needReconnect = false;

		if (theSettings.instanceStrId() != d.settings().instanceStrId() ||
			theSettings.configuratorAddress1() != d.settings().configuratorAddress1() ||
			theSettings.configuratorAddress2() != d.settings().configuratorAddress2())
		{
			needReconnect = true;
		}

		// --
		//
		theSettings = d.settings();
		theSettings.writeSystemScope();

		// Apply settings here
		//
		showLogo();

		// Reconnect
		//
		if (needReconnect == true)
		{
			m_configController.setConnectionParams(theSettings.instanceStrId(),
												   theSettings.configuratorAddress1(),
												   theSettings.configuratorAddress2());
		}

		return;
	}

	return;
}

void MonitorMainWindow::showStatistics()
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

void MonitorMainWindow::showAbout()
{
	QString text = qApp->applicationName() + tr(" allows user to view schemas and trends.<br>");
	DialogAbout::show(this, text, ":/Images/Images/Logo.png");

	return;
}

void MonitorMainWindow::showMatsUserManual()
{
	UiTools::openHelp(QApplication::applicationDirPath()+"/docs/D11.8_FSC_MATS_User_Manual.pdf", this);
}

void MonitorMainWindow::debug()
{
#ifdef Q_DEBUG

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													"./",
													tr("Monitor schemas (*.mvs);; All files (*.*)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	QFileInfo fileInfo(fileName);

	// Load schema
	//
	std::shared_ptr<VFrame30::Schema> schema = std::shared_ptr<VFrame30::Schema>(VFrame30::Schema::Create(fileName.toStdWString().c_str()));

	if (schema == nullptr)
	{
		QMessageBox::critical(this, "Monitor", "Cannot load file");
		return;
	}

	// Create tab
	//
//	QTabWidget* tabWidget = monitorCentralWidget();

//	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(schema);
//	tabWidget->addTab(schemaWidget, "Debug tab: " + fileInfo.fileName());

#endif	// Q_DEBUG
}

void MonitorMainWindow::checkMonitorSingleInstance()
{
	if (m_appInstanceSharedMemory.isAttached() == true &&
	        theSettings.singleInstance() == true)
	{
		// If memory segment is attached, and singleInstance option is set,
		// get information from this memory segment
		//

		m_appInstanceSharedMemory.lock();

		char* sharedData = static_cast<char*>(m_appInstanceSharedMemory.data());

		// If memory segment contains "1" value - other instance of program
		// has been executed. Show message of execution on debug console, and move window
		// to top
		//

		if (*sharedData != 0)
		{
			qDebug() << "Another instance of Monitor has been started";

			*sharedData = 0;

			// To move window to top, add WindowStaysOnTopHint flag. In linux X11Bypass tag required
			// to do this. When flags added - activateWindow and show it to apply changes. After that, window
			// will be every time on top, so we need remove WindowStaysOnTop flag, apply changes, and only then remove
			// X11Bypass flag.
			//

			this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
			this->activateWindow();
			this->show();
			this->setWindowFlags(this->windowFlags() & (~Qt::WindowStaysOnTopHint));
			this->activateWindow();
			this->show();
			this->setWindowFlags(this->windowFlags() & (~Qt::X11BypassWindowManagerHint));
			this->activateWindow();
			this->show();
		}

		m_appInstanceSharedMemory.unlock();
	}
	else
	{
		if (m_appInstanceSharedMemory.isAttached() == false &&
		        theSettings.singleInstance() == true)
		{
			qDebug() << "Single instance checker shared Memory segment is not attached";

			bool result = m_appInstanceSharedMemory.attach();
			if (result == false)
			{
				qDebug() << "Single instance attach error: " << m_appInstanceSharedMemory.errorString();
			}
		}
	}
}

void MonitorMainWindow::slot_archive()
{
	qDebug() << "";
	qDebug() << Q_FUNC_INFO;

	// Get Archive list
	//
	std::vector<QString> archives = MonitorArchive::getArchiveList();

	// Choose window
	//
	QString archiveWindowToActivate;

	if (archives.empty() == true)
	{
		archiveWindowToActivate.clear();	// if archiveWindowToActivate is empty, then create new ArchiveWidget
	}
	else
	{
		QMenu menu;

		QAction* newArchiveAction = menu.addAction("New Window...");
		newArchiveAction->setData(QVariant::fromValue<int>(-1));		// Data -1 means, create new widget

		menu.addSeparator();

		for (size_t i = 0; i < archives.size(); i++)
		{
			QAction* a = menu.addAction(archives[i]);
			Q_ASSERT(a);

			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in archives vector
		}

		QAction* triggeredAction = menu.exec(QCursor::pos());
		if (triggeredAction == nullptr)
		{
			return;
		}

		QVariant data = triggeredAction->data();

		bool ok = false;
		size_t archiveIndex = data.toInt(&ok);

		if (archiveIndex == -1)
		{
			archiveWindowToActivate.clear();	// if trendToActivate is empty, then create new trend
		}
		else
		{
			if (ok == false || archiveIndex < 0 || archiveIndex >= archives.size())
			{
				Q_ASSERT(ok == true);
				Q_ASSERT(archiveIndex >= 0 && archiveIndex < archives.size());
				return;
			}

			archiveWindowToActivate = archives.at(archiveIndex);
		}
	}

	// Start new trend or activate chosen one
	//
	if (archiveWindowToActivate.isEmpty() == true)
	{
		std::vector<AppSignalParam> appSignals;
		MonitorArchive::startNewWidget(&m_configController, appSignals, this);
	}
	else
	{
		MonitorArchive::activateWindow(archiveWindowToActivate);
	}

	return;
}

void MonitorMainWindow::slot_trends()
{
	// Get Trends list
	//
	std::vector<QString> trends = MonitorTrends::getTrendsList();

	// Choose trend
	//
	QString trendToActivate;

	if (trends.empty() == true)
	{
		trendToActivate.clear();	// if trendToActivate is empty, then create new trend
	}
	else
	{
		QMenu menu;

		QAction* newTrendAction = menu.addAction("New Trend...");
		newTrendAction->setData(QVariant::fromValue<int>(-1));		// Data -1 means, create new trend widget

		menu.addSeparator();

		for (size_t i = 0; i < trends.size(); i++)
		{
			QAction* a = menu.addAction(trends[i]);
			Q_ASSERT(a);

			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in trend vector
		}

		QAction* triggeredAction = menu.exec(QCursor::pos());
		if (triggeredAction == nullptr)
		{
			return;
		}

		QVariant data = triggeredAction->data();

		bool ok = false;
		int trendIndex = data.toInt(&ok);

		if (trendIndex == -1)
		{
			trendToActivate.clear();	// if trendToActivate is empty, then create new trend
		}
		else
		{
			if (ok == false || trendIndex < 0 || trendIndex >= static_cast<int>(trends.size()))
			{
				Q_ASSERT(ok == true);
				Q_ASSERT(trendIndex >= 0 && trendIndex < static_cast<int>(trends.size()));
				return;
			}

			trendToActivate = trends.at(trendIndex);
		}
	}

	// Start new trend or activate chosen one
	//
	if (trendToActivate.isEmpty() == true)
	{
		std::vector<AppSignalParam> appSignals;
		MonitorTrends::startTrendApp(&m_configController, appSignals, this);
	}
	else
	{
		MonitorTrends::activateTrendWindow(trendToActivate);
	}

	return;
}

void MonitorMainWindow::slot_signalSnapshot()
{
	DialogSignalSnapshot* dss = new DialogSignalSnapshot(&m_configController, m_tcpSignalClient, this);
	dss->show();

	return;
}

void MonitorMainWindow::slot_findSignal()
{
	DialogSignalSearch* dsi = new DialogSignalSearch(this);
	dsi->show();
	return;
}

void MonitorMainWindow::slot_historyChanged(bool enableBack, bool enableForward)
{
	if (m_historyBack == nullptr ||
		m_historyForward == nullptr)
	{
		Q_ASSERT(m_historyBack);
		Q_ASSERT(m_historyForward);
		return;
	}

	m_historyBack->setEnabled(enableBack);
	m_historyForward->setEnabled(enableForward);

	return;
}

void MonitorMainWindow::slot_configurationArrived(ConfigSettings configuration)
{
	if (m_tuningTcpClientThread != nullptr)
	{
		m_tuningController->resetTcpClient();

		m_tuningTcpClientThread->quitAndWait(10000);
		delete m_tuningTcpClientThread;

		m_tuningTcpClientThread = nullptr;
		m_tuningTcpClient = nullptr;
	}

	// TuningTcpClient
	//
	if (configuration.tuningEnabled == true)
	{
		m_tuningTcpClient = new MonitorTuningTcpClient(m_configController.softwareInfo(), &theTuningSignals, &m_LogFile);

		m_tuningTcpClient->setServers(configuration.tuningService.address(),
									  configuration.tuningService.address(),
									  false);

		m_tuningTcpClientThread = new SimpleThread(m_tuningTcpClient);
		m_tuningTcpClientThread->start();

		m_tuningController->setTcpClient(m_tuningTcpClient);
	}

	if (m_dialogDataSources != nullptr)
	{
		m_dialogDataSources->setTuningTcpClient(m_configController.configuration().tuningEnabled, m_tuningTcpClient, false);
	}

	m_statusBarTuningConnection->setVisible(configuration.tuningEnabled == true);

	m_logoImage = configuration.logoImage;

	showLogo();

	return;
}

void MonitorMainWindow::slot_unknownClient()
{
	// CfgService did not find SoftwareID
	//
	QMessageBox::critical(this,
						  qAppName(),
						  tr("Configuration Service does not recognize Monitor EquipmentID %1")
								.arg(m_configController.softwareInfo().equipmentID()));

	return;
}

MonitorConfigController* MonitorMainWindow::configController()
{
	return &m_configController;
}

const MonitorConfigController* MonitorMainWindow::configController() const
{
	return &m_configController;
}

TcpSignalClient* MonitorMainWindow::tcpSignalClient()
{
	return m_tcpSignalClient;
}

const TcpSignalClient* MonitorMainWindow::tcpSignalClient() const
{
	return m_tcpSignalClient;
}

MonitorToolBar::MonitorToolBar(const QString& tittle, QWidget* parent) :
	QToolBar(tittle, parent)
{
	setAcceptDrops(true);
	setMovable(false);

	return;
}

void MonitorToolBar::addAction(QAction* action)
{
	Q_ASSERT(action);

	QWidget::addAction(action);
	widgetForAction(action)->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

	return;
}

void MonitorToolBar::dragEnterEvent(QDragEnterEvent* event)
{
	// Find Trend action
	//
	QWidget* trendActionWidget = nullptr;
	QWidget* archiveActionWidget = nullptr;

	QList<QAction*> allActions = actions();
	for (QAction* a : allActions)
	{
		QVariant d = a->data();

		if (d.isValid() &&
			d.type() == QVariant::String)
		{
			if (d.toString() == QLatin1String("IAmIndependentTrend"))
			{
				trendActionWidget = widgetForAction(a);
				trendActionWidget->setAcceptDrops(true);
			}

			if (d.toString() == QLatin1String("IAmIndependentArchive"))
			{
				archiveActionWidget = widgetForAction(a);
				archiveActionWidget->setAcceptDrops(true);
			}
		}
	}

	if (trendActionWidget != nullptr &&
		trendActionWidget->geometry().contains(event->pos()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	if (archiveActionWidget != nullptr &&
		archiveActionWidget->geometry().contains(event->pos()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void MonitorToolBar::dropEvent(QDropEvent* event)
{
	// Find Trend action
	//
	QWidget* trendActionWidget = nullptr;
	QAction* trendAction = nullptr;

	QWidget* archiveActionWidget = nullptr;
	QAction* archiveAction = nullptr;

	QList<QAction*> allActions = actions();

	for (QAction* a : allActions)
	{
		QVariant d = a->data();
		if (d.isValid() &&
			d.type() == QVariant::String)
		{
			if (d.toString() == QLatin1String("IAmIndependentTrend"))
			{
				trendAction = a;
				trendActionWidget = widgetForAction(trendAction);
			}

			if (d.toString() == QLatin1String("IAmIndependentArchive"))
			{
				archiveAction = a;
				archiveActionWidget = widgetForAction(archiveAction);
			}
		}
	}

	if (trendAction != nullptr &&
		trendActionWidget != nullptr &&
		trendActionWidget->geometry().contains(event->pos()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		// Lets assume parent isMaonitorMainWindow
		//
		MonitorMainWindow* m = dynamic_cast<MonitorMainWindow*>(this->parent());
		if (m == nullptr)
		{
			Q_ASSERT(m);
			return;
		}

		// Load data from drag and drop
		//
		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

		::Proto::AppSignalSet protoSetMessage;
		bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

		if (ok == false)
		{
			event->acceptProposedAction();
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.reserve(protoSetMessage.appsignal_size());

		// Parse data
		//
		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
		{
			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

			AppSignalParam appSignalParam;
			ok = appSignalParam.load(appSignalMessage);

			if (ok == true)
			{
				appSignals.push_back(appSignalParam);
			}
		}

		if (appSignals.empty() == false)
		{
			m->showTrends(appSignals);
		}
	}

	if (archiveAction != nullptr &&
		archiveActionWidget != nullptr &&
		archiveActionWidget->geometry().contains(event->pos()) &&
		event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		// Lets assume parent isMonitorMainWindow
		//
		MonitorMainWindow* mainWindow = dynamic_cast<MonitorMainWindow*>(this->parent());
		if (mainWindow == nullptr)
		{
			Q_ASSERT(mainWindow);
			return;
		}

		// Load data from drag and drop
		//
		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

		::Proto::AppSignalSet protoSetMessage;
		bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

		if (ok == false)
		{
			event->acceptProposedAction();
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.reserve(protoSetMessage.appsignal_size());

		// Parse data
		//
		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
		{
			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

			AppSignalParam appSignalParam;
			ok = appSignalParam.load(appSignalMessage);

			if (ok == true)
			{
				appSignals.push_back(appSignalParam);
			}
		}

		if (appSignals.empty() == false)
		{
			MonitorArchive::startNewWidget(mainWindow->configController(), appSignals, mainWindow);
		}
	}

	return;
}

