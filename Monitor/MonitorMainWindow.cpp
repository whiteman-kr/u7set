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
#include "../VFrame30/Schema.h"

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

	connect(m_schemaListWidget, &SchemaListWidget::selectionChanged, monitorCentralWidget, &MonitorCentralWidget::slot_selectSchemaForCurrentTab);

	// --
	//
	centralWidget()->show();

	m_configController.start();

	m_updateStatusBarTimerId = startTimer(100);

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
					   .arg(confiConnState.isConnected ? confiConnState.host.addressStr() : "NoConnection")
						.arg(signalClientState.isConnected ? signalClientState.host.addressStr() : "NoConnection");

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
	theSettings.loadUserScope();

	move(theSettings.m_mainWindowPos);
	restoreGeometry(theSettings.m_mainWindowGeometry);
	restoreState(theSettings.m_mainWindowState);

	return;
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
	m_historyBack->setStatusTip(tr("Click to got back"));
	m_historyBack->setIcon(QIcon(":/Images/Images/Backward.svg"));
	m_historyBack->setEnabled(false);
	m_historyBack->setShortcut(QKeySequence::Back);
	connect(m_historyBack, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_historyBack);

	m_historyForward = new QAction(tr("Go Forward"), this);
	m_historyForward->setStatusTip(tr("Click to got forward"));
	m_historyForward->setIcon(QIcon(":/Images/Images/Forward.svg"));
	m_historyForward->setEnabled(false);
	m_historyForward->setShortcut(QKeySequence::Forward);
	connect(m_historyForward, &QAction::triggered, monitorCentralWidget(), &MonitorCentralWidget::slot_historyForward);

	m_signalSnapshotAction = new QAction(tr("Signals Snapshot"), this);
	m_signalSnapshotAction->setStatusTip(tr("View signals state in real time"));
	m_signalSnapshotAction->setIcon(QIcon(":/Images/Images/Camera.svg"));
	m_signalSnapshotAction->setEnabled(true);
	m_signalSnapshotAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
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
	m_toolBar->addAction(m_signalSnapshotAction);
	m_toolBar->addAction(m_findSignalAction);

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

void MonitorMainWindow::slot_signalSnapshot()
{
	DialogSignalSnapshot* dss = new DialogSignalSnapshot(this);
	dss->show();

	return;
}

void MonitorMainWindow::slot_findSignal()
{
	DialogSignalSearch* dsi = new DialogSignalSearch(this);
	dsi->show();

	return;
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

	std::vector<ConfigSchema> schemas = m_configController->schemas();

	std::sort(schemas.begin(), schemas.end(),
		[](const ConfigSchema& s1, const ConfigSchema& s2) -> bool
		{
			return s1.strId < s2.strId;
		});

	for (const ConfigSchema& s : schemas)
	{
		QVariant data = QVariant::fromValue(s.strId);
		m_comboBox->addItem(s.strId + "  " + s.caption, data);
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
	m_comboBox->blockSignals(false);	// Allo wto emit signals

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
