#include "MonitorMainWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include "MonitorCentralWidget.h"
#include "Settings.h"
#include "DialogSettings.h"
#include "MonitorSchemaWidget.h"
#include "DialogSignalSearch.h"
#include "DialogSignalSnapshot.h"
#include "MonitorTrends.h"
#include "../VFrame30/Schema.h"

const QString MonitorMainWindow::m_monitorSingleInstanceKey = "MonitorInstanceCheckerKey";

MonitorMainWindow::MonitorMainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_configController(theSettings.configuratorAddress1(), theSettings.configuratorAddress2()),
	m_schemaManager(&m_configController)
{
	qDebug() << Q_FUNC_INFO;

	// TcpSignalClient
	//
	HostAddressPort fakeAddress(QLatin1String("0.0.0.0"), 0);
	m_tcpSignalClient = new TcpSignalClient(&m_configController, fakeAddress, fakeAddress);

	m_tcpClientThread = new SimpleThread(m_tcpSignalClient);
	m_tcpClientThread->start();

	connect(m_tcpSignalClient, &TcpSignalClient::signalParamAndUnitsArrived, this, &MonitorMainWindow::tcpSignalClient_signalParamAndUnitsArrived);
	connect(m_tcpSignalClient, &TcpSignalClient::connectionReset, this, &MonitorMainWindow::tcpSignalClient_connectionReset);

	// --
	//
	MonitorCentralWidget* monitorCentralWidget = new MonitorCentralWidget(&m_schemaManager);
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
			assert(m_closeTabAction);
			m_closeTabAction->setEnabled(allowed);
		});

	connect(monitorCentralWidget, &MonitorCentralWidget::signal_historyChanged, this, &MonitorMainWindow::slot_historyChanged);

	connect(m_schemaListWidget, &SchemaListWidget::selectionChanged, monitorCentralWidget, &MonitorCentralWidget::slot_selectSchemaForCurrentTab);

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
	assert(event);

	// Update status bar
	//
	if  (event->timerId() == m_updateStatusBarTimerId &&
		 m_tcpSignalClient != nullptr)
	{
		assert(m_statusBarConnectionState);
		assert(m_statusBarConnectionStatistics);

		Tcp::ConnectionState confiConnState =  m_configController.getConnectionState();
		Tcp::ConnectionState signalClientState =  m_tcpSignalClient->getConnectionState();

		// State
		//
		QString text = QString(" ConfigSrv: %1   AppDataSrv: %2 ")
					   .arg(confiConnState.isConnected ? confiConnState.peerAddr.addressStr() : "NoConnection")
						.arg(signalClientState.isConnected ? signalClientState.peerAddr.addressStr() : "NoConnection");

		m_statusBarConnectionState->setText(text);

		// Statistics
		//
		text = QString(" ConfigSrv: %1   AppDataSrv: %2 ")
			   .arg(QString::number(confiConnState.replyCount))
			   .arg(QString::number(signalClientState.replyCount));

		m_statusBarConnectionStatistics->setText(text);

		return;
	}

	return;
}

void MonitorMainWindow::showEvent(QShowEvent*)
{
	showLogo();
	return;
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
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
	}

	return;
}

void MonitorMainWindow::showLogo()
{
	assert(m_logoLabel);

	QImage logo = QImage(":/Images/Images/Logo.png");

	if (m_toolBar->frameSize().height() < logo.height())
	{
		logo = logo.scaledToHeight(m_toolBar->frameSize().height(), Qt::FastTransformation);
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

	m_pSettingsAction = new QAction(tr("Settings..."), this);
	m_pSettingsAction->setStatusTip(tr("Change application settings"));
	m_pSettingsAction->setIcon(QIcon(":/Images/Images/Settings.svg"));
	m_pSettingsAction->setEnabled(true);
	connect(m_pSettingsAction, &QAction::triggered, this, &MonitorMainWindow::showSettings);

	m_pDebugAction = new QAction(tr("Debug..."), this);
	m_pDebugAction->setStatusTip(tr("Perform some debug actions, don't run it!"));
	m_pDebugAction->setEnabled(true);
	connect(m_pDebugAction, &QAction::triggered, this, &MonitorMainWindow::debug);

	m_pLogAction = new QAction(tr("Log..."), this);
	m_pLogAction->setStatusTip(tr("Show application log"));
	//m_pLogAction->setEnabled(false);
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

	m_trendsAction = new QAction(tr("Trends"), this);
	m_trendsAction->setIcon(QIcon(":/Images/Images/Trends.svg"));
	m_trendsAction->setEnabled(true);
	connect(m_trendsAction, &QAction::triggered, this, &MonitorMainWindow::slot_trends);

	m_signalSnapshotAction = new QAction(tr("Signals Snapshot"), this);
	m_signalSnapshotAction->setStatusTip(tr("View signals state in real time"));
	m_signalSnapshotAction->setIcon(QIcon(":/Images/Images/Camera.svg"));
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

	viewMenu->addSeparator();
	viewMenu->addAction(m_trendsAction);
	viewMenu->addAction(m_signalSnapshotAction);
	viewMenu->addAction(m_findSignalAction);


	// Tools
	//
	QMenu* pToolsMenu = menuBar()->addMenu(tr("&Tools"));

	pToolsMenu->addAction(m_pSettingsAction);

	// Help
	//
	menuBar()->addSeparator();
	QMenu* pHelpMenu = menuBar()->addMenu(tr("&?"));

#ifdef Q_DEBUG
	pHelpMenu->addAction(m_pDebugAction);
#endif	// Q_DEBUG
	pHelpMenu->addAction(m_pLogAction);
	pHelpMenu->addAction(m_pAboutAction);

	return;
}

void MonitorMainWindow::createToolBars()
{
	m_toolBar = new QToolBar(this);
	m_toolBar->setObjectName("MonitorMainToolBar");

	m_toolBar->setMovable(false);
	m_toolBar->setIconSize(QSize(28, 28));
	m_toolBar->setStyleSheet("QToolBar{spacing:2px;padding:2px;}");

	m_toolBar->addAction(m_newTabAction);
	m_toolBar->addSeparator();

	m_toolBar->addAction(m_zoomInAction);
	m_toolBar->addAction(m_zoomOutAction);
	m_toolBar->addSeparator();

	m_schemaListWidget = new SchemaListWidget(&m_configController, monitorCentralWidget());
	m_schemaListWidget->setMinimumWidth(300);
	m_toolBar->addWidget(m_schemaListWidget);
	m_toolBar->addSeparator();

	m_toolBar->addAction(m_historyBack);
	m_toolBar->addAction(m_historyForward);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_trendsAction);
	m_toolBar->addAction(m_signalSnapshotAction);
	m_toolBar->addAction(m_findSignalAction);

	// Create logo for toolbar
	//
	m_logoLabel = new QLabel(this);

	// Spacer between actions and logo
	//
	m_logoSpacer = new QWidget(this);
	m_logoSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	m_toolBar->addWidget(m_logoSpacer);
	m_toolBar->addWidget(m_logoLabel);

	this->addToolBar(Qt::TopToolBarArea, m_toolBar);

	return;
}

void MonitorMainWindow::createStatusBar()
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

	return;
}

MonitorCentralWidget* MonitorMainWindow::monitorCentralWidget()
{
	MonitorCentralWidget* centralWidget = dynamic_cast<MonitorCentralWidget*>(QMainWindow::centralWidget());
	assert(centralWidget != nullptr);

	return centralWidget;
}

void MonitorMainWindow::exit()
{
	close();
}

void MonitorMainWindow::showLog()
{

}

void MonitorMainWindow::showSettings()
{
	DialogSettings d(this);
	d.setSettings(theSettings);

	int result = d.exec();

	if (result == QDialog::DialogCode::Accepted)
	{
		theSettings = d.settings();
		theSettings.writeSystemScope();

		// Apply settings here
		//
		showLogo();
		return;
	}

	return;
}

void MonitorMainWindow::showAbout()
{
	QMessageBox::about(this, tr("About Monitor"), tr("Monitor software. Version not assigned."));
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

void MonitorMainWindow::slot_trends()
{
	qDebug() << "";
	qDebug() << Q_FUNC_INFO;

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
			assert(a);

			a->setData(QVariant::fromValue<int>(static_cast<int>(i)));		// Data is index in trend vector
		}

		QAction* triggeredAction = menu.exec(QCursor::pos());
		if (triggeredAction == nullptr)
		{
			return;
		}

		QVariant data = triggeredAction->data();

		bool ok = false;
		size_t trendIndex = data.toInt(&ok);

		if (trendIndex == -1)
		{
			trendToActivate.clear();	// if trendToActivate is empty, then create new trend
		}
		else
		{
			if (ok == false || trendIndex < 0 || trendIndex >= trends.size())
			{
				assert(ok == true);
				assert(trendIndex >= 0 && trendIndex < trends.size());
				return;
			}

			trendToActivate = trends.at(trendIndex);
		}
	}

	// Start new trend or activate chosen one
	//
	if (trendToActivate.isEmpty() == true)
	{
		ConfigSettings conf = m_configController.configuration();
		MonitorTrends::startTrendApp(conf.ads1.ip(), conf.ads1.port(), conf.ads2.ip(), conf.ads2.port(), this);
	}
	else
	{
		MonitorTrends::activateTrendWindow(trendToActivate);
	}

	return;
}

void MonitorMainWindow::slot_signalSnapshot()
{
	DialogSignalSnapshot* dss = new DialogSignalSnapshot(&m_configController, this);
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
		assert(m_historyBack);
		assert(m_historyForward);

		return;
	}

	m_historyBack->setEnabled(enableBack);
	m_historyForward->setEnabled(enableForward);

	return;
}

void MonitorMainWindow::tcpSignalClient_signalParamAndUnitsArrived()
{
	emit signalParamAndUnitsArrived();
}

void MonitorMainWindow::tcpSignalClient_connectionReset()
{
	emit connectionReset();
}


SchemaListWidget::SchemaListWidget(MonitorConfigController* configController, MonitorCentralWidget* centralWidget) :
	m_configController(configController),
	m_centraWidget(centralWidget)
{
	assert(m_configController);
	assert(m_centraWidget);

	m_label = new QLabel;
	m_label->setText(tr("Schema:"));

	m_comboBox = new QComboBox;

	QLayout* layout = new QVBoxLayout(this);

	layout->addWidget(m_label);
	layout->addWidget(m_comboBox);

	setLayout(layout);

	connect(m_configController, &MonitorConfigController::configurationArrived, this, &SchemaListWidget::slot_configurationArrived);
	connect(m_centraWidget, &MonitorCentralWidget::signal_schemaChanged, this, &SchemaListWidget::slot_schemaChanged);
	connect(m_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SchemaListWidget::slot_indexChanged);
}

SchemaListWidget::~SchemaListWidget()
{

}

void SchemaListWidget::slot_configurationArrived(ConfigSettings /*configuration*/)
{
	assert(m_comboBox);
	assert(m_configController);
	assert(m_centraWidget);

	m_comboBox->blockSignals(true);		// don;'t want to emit slot_indexChanged

	// Save state
	//
	QVariant selected;

	MonitorSchemaWidget* tab = m_centraWidget->currentTab();
	if (tab != nullptr)
	{
		selected = tab->schemaId();
	}

	// Clear all and fill with new data;
	//
	m_comboBox->clear();

	std::vector<VFrame30::SchemaDetails> schemas = m_configController->schemasDetails();

	std::sort(schemas.begin(), schemas.end(),
		[](const VFrame30::SchemaDetails& s1, const VFrame30::SchemaDetails& s2) -> bool
		{
			return s1.m_schemaId < s2.m_schemaId;
		});

	for (const VFrame30::SchemaDetails& s : schemas)
	{
		QVariant data = QVariant::fromValue(s.m_schemaId);
		m_comboBox->addItem(s.m_schemaId + "  " + s.m_caption, data);
	}

	// Restore selected
	//
	if (selected.isValid() == true)
	{
		int index = m_comboBox->findData(selected);

		if (index != -1)
		{
			m_comboBox->setCurrentIndex(index);
		}
		else
		{
			m_comboBox->setCurrentIndex(-1);
		}
	}
	else
	{
		m_comboBox->setCurrentIndex(-1);
	}

	// Allow signals
	//
	m_comboBox->blockSignals(false);	// Allow to emit signals

	return;
}

void SchemaListWidget::slot_schemaChanged(QString strId)
{
	if (m_comboBox == nullptr ||
		m_configController == nullptr)
	{
		assert(m_comboBox);
		assert(m_configController);
		return;
	}

	m_comboBox->blockSignals(true);		// don;'t want to emit slot_indexChanged

	// Restore selected
	//
	QVariant data = QVariant::fromValue(strId);

	int index = m_comboBox->findData(data);

	if (index != -1)
	{
		m_comboBox->setCurrentIndex(index);
	}
	else
	{
		m_comboBox->setCurrentIndex(-1);
	}

	// Allow signals
	//
	m_comboBox->blockSignals(false);	// Allo wto emit signals

	return;
}

void SchemaListWidget::slot_indexChanged(int /*index*/)
{
	QVariant data = m_comboBox->currentData();

	if (data.isValid() == false ||
		data.type() != QVariant::String)
	{
		return;
	}

	QString strId = data.toString();

	emit selectionChanged(strId);

	return;
}
