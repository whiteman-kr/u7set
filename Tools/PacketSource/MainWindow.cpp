#include "MainWindow.h"

#include "../../lib/Ui/DialogAbout.h"
#include "../../lib/XmlHelper.h"

#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(const Options& options, QMainWindow* parent)
	: QMainWindow(parent)
	, m_options(options)
{
	runConfigSocket();					// init config socket thread

	createInterface();					// init interface
	restoreWindowState();

	connect(&m_pscore, &PacketSourceCore::signalsLoadedInSources, this, &MainWindow::signalsLoadedInSources, Qt::QueuedConnection);
	connect(&m_pscore, &PacketSourceCore::ualTesterSocketConnect, this, &MainWindow::ualTesterSocketConnect, Qt::QueuedConnection);

	m_pscore.setBuildOption(m_options.build());
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
	setWindowTitle(tr("Packet Source"));

	createActions();
	createMenu();
	createToolBars();
	createPanels();
	createViews();
	createContextMenu();
	createHeaderContexMenu();
	createStatusBar();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createActions()
{
	// Sources
	//
	m_sourceStartAction = new QAction(tr("Start"), this);
	m_sourceStartAction->setShortcut(Qt::Key_F5);
	m_sourceStartAction->setIcon(QIcon(":/Images/Run.svg"));
	m_sourceStartAction->setToolTip(tr("Start all sources"));
	connect(m_sourceStartAction, &QAction::triggered, this, &MainWindow::startSource);

	m_sourceStopAction = new QAction(tr("Stop"), this);
	m_sourceStopAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
	m_sourceStopAction->setIcon(QIcon(":/Images/Stop.svg"));
	m_sourceStopAction->setToolTip(tr("Stop all sources"));
	connect(m_sourceStopAction, &QAction::triggered, this, &MainWindow::stopSource);

	m_sourceSelectAllAction = new QAction(tr("Select all"), this);
	m_sourceSelectAllAction->setIcon(QIcon(":/Images/SelectAll.svg"));
	m_sourceSelectAllAction->setToolTip(tr("Select all sources"));
	connect(m_sourceSelectAllAction, &QAction::triggered, this, &MainWindow::selectAllSources);

	// Signals
	//
	m_signalSetStateAction = new QAction(tr("Set state ..."), this);
	m_signalSetStateAction->setToolTip(tr("Set signal state"));
	connect(m_signalSetStateAction, &QAction::triggered, this, &MainWindow::setSignalState);

	m_signalInitAction = new QAction(tr("Initialization"), this);
	m_signalInitAction->setShortcut(Qt::CTRL + Qt::Key_I);
	m_signalInitAction->setIcon(QIcon(":/Images/Init.svg"));
	m_signalInitAction->setToolTip(tr("Initialization all signals"));
	connect(m_signalInitAction, &QAction::triggered, this, &MainWindow::initSignalsState);

	m_signalHistoryAction = new QAction(tr("History ..."), this);
	m_signalHistoryAction->setShortcut(Qt::CTRL + Qt::Key_H);
	m_signalHistoryAction->setIcon(QIcon(":/Images/History.svg"));
	m_signalHistoryAction->setToolTip(tr("Hystory of signals state"));
	connect(m_signalHistoryAction, &QAction::triggered, this, &MainWindow::history);

	m_signalSaveStatesAction = new QAction(tr("Save signals state ..."), this);
	m_signalSaveStatesAction->setShortcut(Qt::CTRL + Qt::Key_S);
	m_signalSaveStatesAction->setIcon(QIcon(":/Images/Upload.svg"));
	m_signalSaveStatesAction->setToolTip(tr("Save signals state"));
	connect(m_signalSaveStatesAction, &QAction::triggered, this, &MainWindow::saveSignalsState);

	m_signalRestoreStatesAction = new QAction(tr("Restore signals state ..."), this);
	m_signalRestoreStatesAction->setShortcut(Qt::CTRL + Qt::Key_R);
	m_signalRestoreStatesAction->setIcon(QIcon(":/Images/Download.svg"));
	m_signalRestoreStatesAction->setToolTip(tr("Restore signals state"));
	connect(m_signalRestoreStatesAction, &QAction::triggered, this, &MainWindow::restoreSignalsState);


	m_signalSelectAllAction = new QAction(tr("Select all"), this);
	m_signalSelectAllAction->setIcon(QIcon(":/Images/SelectAll.svg"));
	m_signalSelectAllAction->setToolTip(tr("Select all signals"));
	connect(m_signalSelectAllAction, &QAction::triggered, this, &MainWindow::selectAllSignals);

	// ?
	//
	m_optionAction = new QAction(tr("&Options"), this);
	m_optionAction->setShortcut(Qt::CTRL + Qt::Key_O);
	m_optionAction->setIcon(QIcon(":/Images/Options.svg"));
	m_optionAction->setToolTip(tr("Options of sources"));
	connect(m_optionAction, &QAction::triggered, this, &MainWindow::onOptions);

	m_pAboutAppAction = new QAction(tr("About ..."), this);
	m_pAboutAppAction->setIcon(QIcon(":/Images/About.svg"));
	m_pAboutAppAction->setToolTip("");
	connect(m_pAboutAppAction, &QAction::triggered, this, &MainWindow::aboutApp);

	// source contex menu
	//
	m_sourceTextCopyAction = new QAction(tr("&Copy"), this);
	m_sourceTextCopyAction->setIcon(QIcon(":/Images/Copy.svg"));
	connect(m_sourceTextCopyAction, &QAction::triggered, this, &MainWindow::copySourceText);

	// signal contex menu
	//
	m_signalTextCopyAction = new QAction(tr("&Copy"), this);
	m_signalTextCopyAction->setIcon(QIcon(":/Images/Copy.svg"));
	connect(m_signalTextCopyAction, &QAction::triggered, this, &MainWindow::copySignalText);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
	QMenuBar* pMenuBar = menuBar();
	if (pMenuBar == nullptr)
	{
		return;
	}

	// Sources
	//
	m_sourceMenu = pMenuBar->addMenu(tr("Sources"));

	m_sourceMenu->addAction(m_sourceStartAction);
	m_sourceMenu->addAction(m_sourceStopAction);
	m_sourceMenu->addSeparator();
	m_sourceMenu->addAction(m_sourceSelectAllAction);

	// Signals
	//
	m_signalMenu = pMenuBar->addMenu(tr("Signals"));

	m_signalMenu->addAction(m_signalInitAction);
	m_signalMenu->addSeparator();
	m_signalMenu->addAction(m_signalHistoryAction);
	m_signalMenu->addSeparator();
	m_signalMenu->addAction(m_signalSaveStatesAction);
	m_signalMenu->addAction(m_signalRestoreStatesAction);
	m_signalMenu->addSeparator();
	m_signalMenu->addAction(m_signalSelectAllAction);

	// Tools
	//
	m_toolsMenu = pMenuBar->addMenu(tr("Tools"));

	m_toolsMenu->addAction(m_optionAction);

	// ?
	//
	m_infoMenu = pMenuBar->addMenu(tr("?"));

	m_infoMenu->addAction(m_pAboutAppAction);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createToolBars()
{
	// Control panel of serial ports
	//
	m_mainToolBar = new QToolBar(this);
	if (m_mainToolBar == nullptr)
	{
		return false;
	}

	m_mainToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	m_mainToolBar->setWindowTitle(tr("Control panel of sources"));
	m_mainToolBar->setObjectName(m_mainToolBar->windowTitle());
	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(m_mainToolBar);

	m_mainToolBar->addAction(m_sourceStartAction);
	m_mainToolBar->addAction(m_sourceStopAction);
	m_mainToolBar->addSeparator();
	m_mainToolBar->addAction(m_optionAction);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
	// Frame of data panel
	//
	m_pFrameDataPanel = new FrameDataPanel(this);
	if (m_pFrameDataPanel != nullptr)
	{
		m_pFrameDataPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::RightDockWidgetArea, m_pFrameDataPanel);

		m_pFrameDataPanel->hide();

		QAction* dataAction = m_pFrameDataPanel->toggleViewAction();
		if (dataAction != nullptr)
		{
			dataAction->setText(tr("&Frame of data ..."));
			dataAction->setShortcut(Qt::CTRL + Qt::Key_D);
			dataAction->setIcon(QIcon(":/Images/FrameData.svg"));
			dataAction->setToolTip(tr("Frame of data"));

			if (m_signalMenu != nullptr)
			{
				m_signalMenu->insertAction(m_signalHistoryAction, dataAction);
			}

			if (m_mainToolBar != nullptr)
			{
				m_mainToolBar->insertAction(m_optionAction, dataAction);
			}
		}
	}

	// Search measurements panel
	//
	m_pFindSignalPanel = new FindSignalPanel(this);
	if (m_pFindSignalPanel != nullptr)
	{
		m_pFindSignalPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::RightDockWidgetArea, m_pFindSignalPanel);

		m_pFindSignalPanel->hide();

		QAction* findAction = m_pFindSignalPanel->toggleViewAction();
		if (findAction != nullptr)
		{
			findAction->setText(tr("&Find ..."));
			findAction->setShortcut(Qt::CTRL + Qt::Key_F);
			findAction->setIcon(QIcon(":/Images/Find.svg"));
			findAction->setToolTip(tr("Find signal"));

			if (m_signalMenu != nullptr)
			{
				m_signalMenu->insertAction(m_signalHistoryAction, findAction);
				m_signalMenu->insertSection(m_signalHistoryAction, QString());
			}

			if (m_mainToolBar != nullptr)
			{
				m_mainToolBar->insertAction(m_optionAction, findAction);
				m_mainToolBar->insertSeparator(m_optionAction);
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createViews()
{
	// View of sources
	//
	m_pSourceView = new QTableView(this);
	if (m_pSourceView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = new QSortFilterProxyModel(this);
	pSourceProxyModel->setSourceModel(&m_sourceTable);
	m_pSourceView->setModel(pSourceProxyModel);
	m_pSourceView->setSortingEnabled(true);
	pSourceProxyModel->sort(SOURCE_LIST_COLUMN_LM_IP, Qt::AscendingOrder);

	m_pSourceView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
	{
		m_pSourceView->setColumnWidth(column, SourceListColumnWidth[column]);
	}

	m_pSourceView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(m_pSourceView, &QTableView::clicked , this, &MainWindow::onSourceListClicked);

	// View of Signals
	//
	m_pSignalView = new QTableView(this);
	if (m_pSignalView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSignalProxyModel = new QSortFilterProxyModel(this);
	pSignalProxyModel->setSourceModel(&m_signalTable);
	m_pSignalView->setModel(pSignalProxyModel);
	m_pSignalView->setSortingEnabled(true);
	pSignalProxyModel->sort(SIGNAL_LIST_COLUMN_CUSTOM_ID, Qt::AscendingOrder);

	m_pSignalView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pSignalView->setColumnWidth(column, SignalListColumnWidth[column]);
	}

	m_pSignalView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pSignalView->setWordWrap(false);

	connect(m_pSignalView, &QTableView::doubleClicked , this, &MainWindow::onSignalListDoubleClicked);

	// Layouts
	//

	QVBoxLayout *ssLayout = new QVBoxLayout;

	ssLayout->addWidget(m_pSourceView);
	ssLayout->addWidget(m_pSignalView);

	QHBoxLayout *mainLayout = new QHBoxLayout;

	mainLayout->addLayout(ssLayout);

	QWidget* pWidget = new QWidget(this);
	pWidget->setLayout(mainLayout);

	setCentralWidget(pWidget);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createContextMenu()
{
	// View of sources
	//
	m_sourceContextMenu = new QMenu(this);

	m_sourceContextMenu->addAction(m_sourceStartAction);
	m_sourceContextMenu->addAction(m_sourceStopAction);
	m_sourceContextMenu->addSeparator();
	m_sourceContextMenu->addAction(m_sourceTextCopyAction);

	m_pSourceView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSourceView, &QTableView::customContextMenuRequested, this, &MainWindow::onSourceContextMenu);


	// View of signals
	//
	m_pSignalView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSignalView, &QTableView::customContextMenuRequested, this, &MainWindow::onSignalContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createHeaderContexMenu()
{
	// init header context menu for View of sources
	//
	m_pSourceView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSourceView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::onSourceHeaderContextMenu);

	m_sourceHeaderContextMenu = new QMenu(m_pSourceView);

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
	{
		m_pSourceColumnAction[column] = m_sourceHeaderContextMenu->addAction(SourceListColumn[column]);
		if (m_pSourceColumnAction[column] != nullptr)
		{
			m_pSourceColumnAction[column]->setCheckable(true);
			m_pSourceColumnAction[column]->setChecked(true);

			connect(m_sourceHeaderContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MainWindow::onSourceColumnAction);
		}
	}

	hideSourceColumn(SOURCE_LIST_COLUMN_MODULE_TYPE, true);
	hideSourceColumn(SOURCE_LIST_COLUMN_SUB_SYSTEM, true);
	hideSourceColumn(SOURCE_LIST_COLUMN_FRAME_COUNT, true);
	hideSourceColumn(SOURCE_LIST_COLUMN_SIGNAL_COUNT, true);

	// init header context menu for View of signals
	//
	m_pSignalView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSignalView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::onSignalHeaderContextMenu);

	m_signalHeaderContextMenu = new QMenu(m_pSignalView);

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pSignalColumnAction[column] = m_signalHeaderContextMenu->addAction(SignalListColumn[column]);
		if (m_pSignalColumnAction[column] != nullptr)
		{
			m_pSignalColumnAction[column]->setCheckable(true);
			m_pSignalColumnAction[column]->setChecked(true);

			connect(m_signalHeaderContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MainWindow::onSignalColumnAction);
		}
	}

	hideSignalColumn(SIGNAL_LIST_COLUMN_FORMAT, true);
	hideSignalColumn(SIGNAL_LIST_COLUMN_STATE_OFFSET, true);
	hideSignalColumn(SIGNAL_LIST_COLUMN_STATE_BIT, true);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createStatusBar()
{
	QStatusBar* m_statusBar = statusBar();
	if (m_statusBar == nullptr)
	{
		return;
	}

	m_statusEmpty = new QLabel(m_statusBar);
	m_statusLoadSignals = new QProgressBar(m_statusBar);
	m_statusConnectToConfigServer = new QLabel(m_statusBar);
	m_statusSendToAppDaraServer = new QLabel(m_statusBar);
	m_statusUalTesterClient = new QLabel(m_statusBar);

	m_statusLoadSignals->hide();
	m_statusLoadSignals->setRange(0, 100);
	m_statusLoadSignals->setFixedWidth(100);
	m_statusLoadSignals->setFixedHeight(10);
	m_statusLoadSignals->setLayoutDirection(Qt::LeftToRight);

	m_statusBar->addWidget(m_statusUalTesterClient);
	m_statusBar->addWidget(m_statusSendToAppDaraServer);
	m_statusBar->addWidget(m_statusConnectToConfigServer);
	m_statusBar->addWidget(m_statusLoadSignals);
	m_statusBar->addWidget(m_statusEmpty);

	m_statusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: off "));
	m_statusConnectToConfigServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\""));

	m_statusSendToAppDaraServer->setText(tr(" AppDataService: off "));
	m_statusSendToAppDaraServer->setToolTip("Equipment ID: Not loaded\nPackets send to: Not loaded");

	m_statusUalTesterClient->setText(tr(" UalTester: off "));

	m_statusEmpty->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runConfigSocket()
{
	// init SoftwareInfo
	//
	SoftwareInfo si;

	QString equipmentID = m_options.build().cfgSrvEquipmentID();
	si.init(E::SoftwareType::TestClient, equipmentID, 1, 0);

	// create config socket thread
	//
	HostAddressPort configSocketAddress = m_options.build().cfgSrvIP();
	m_pConfigSocket = new ConfigSocket(si, configSocketAddress, &m_pscore);

	// connect
	//
	connect(m_pConfigSocket, &ConfigSocket::socketConnected, this, &MainWindow::configSocketConnected, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::socketDisconnected, this, &MainWindow::configSocketDisconnected, Qt::QueuedConnection);

	connect(m_pConfigSocket, &ConfigSocket::unknownClient, this, &MainWindow::configSocketUnknownClient, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::unknownAdsEquipmentID, this, &MainWindow::configSocketUnknownAdsEquipmentID, Qt::QueuedConnection);

	// loading
	//
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &MainWindow::configSocketConfigurationLoaded, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::sourceBaseLoaded, this, &MainWindow::sourcesLoaded, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoading, this, &MainWindow::configSocketSignalBaseLoading, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, this, &MainWindow::configSocketSignalBaseLoaded, Qt::QueuedConnection);

	// start
	//
	m_pConfigSocket->start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopConfigSocket()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	delete m_pConfigSocket;
	m_pConfigSocket = nullptr;
}


// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketConnected()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	if (m_statusConnectToConfigServer == nullptr)
	{
		return;
	}

	HostAddressPort configSocketAddress = m_pConfigSocket->cfgSrvIP();

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: on "));
	m_statusConnectToConfigServer->setToolTip(m_pConfigSocket->cfgSrvInfo());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketDisconnected()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	if (m_statusLoadSignals == nullptr || m_statusConnectToConfigServer == nullptr || m_statusSendToAppDaraServer == nullptr)
	{
		return;
	}

	m_statusLoadSignals->hide();

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: off "));
	m_statusConnectToConfigServer->setToolTip(m_pConfigSocket->cfgSrvInfo());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketUnknownClient()
{
	QMessageBox::critical(this,
						  windowTitle(),
						  tr("Configuration Service does not recognize EquipmentID \"%1\" for software \"PacketSource\"")
						  .arg(m_options.build().cfgSrvEquipmentID()));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketUnknownAdsEquipmentID(const QStringList& adsIDList)
{
	QString adsEquipmentID = m_options.build().appDataSrvEquipmentID();

	// create dialog for comment
	//
	QDialog* pSelectIdDialog = new QDialog(this);
	if (pSelectIdDialog == nullptr)
	{
		return;
	}
		pSelectIdDialog->setWindowTitle(tr("Select EquipmentID"));
		pSelectIdDialog->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

		QRect screen = QDesktopWidget().availableGeometry(this);
		pSelectIdDialog->resize(static_cast<int>(screen.width() * 0.20), static_cast<int>(screen.height() * 0.02));
		pSelectIdDialog->move(screen.center() - pSelectIdDialog->rect().center());


		QLabel* pCommentLabel = new QLabel(	tr("App Data Service EquipmentID \"%1\" is wrong!\n"
											   "Please select EquipmentID from list:").arg(adsEquipmentID), this);

		QComboBox* pEquipmentIdCombo = new QComboBox(pSelectIdDialog);
		for(const QString& adsID : adsIDList)
		{
			pEquipmentIdCombo->addItem(adsID);
		}

		QHBoxLayout *buttonLayout = new QHBoxLayout;

		QPushButton* pSelectButton = new QPushButton(tr("Select"), pSelectIdDialog);
		QPushButton* pCancelButton = new QPushButton(tr("Cancel"), pSelectIdDialog);

		buttonLayout->addStretch();
		buttonLayout->addWidget(pSelectButton);
		buttonLayout->addWidget(pCancelButton);

		QVBoxLayout *mainLayout = new QVBoxLayout;

		mainLayout->addWidget(pCommentLabel);
		mainLayout->addWidget(pEquipmentIdCombo);
		mainLayout->addLayout(buttonLayout);

		pSelectIdDialog->setLayout(mainLayout);
		pSelectIdDialog->setModal(true);

		connect(pSelectButton, &QPushButton::clicked, pSelectIdDialog, &QDialog::accept);
		connect(pCancelButton, &QPushButton::clicked, pSelectIdDialog, &QDialog::reject);

		int result = pSelectIdDialog->exec();
		if (result == QDialog::Accepted)
		{
			adsEquipmentID = pEquipmentIdCombo->currentText();
		}

	delete pSelectIdDialog;

	clearViews();

	// save new options
	//
	m_options.build().setAppDataSrvEquipmentID(adsEquipmentID);
	m_options.build().save();

	// restart
	//
	m_pscore.clear();
	m_pscore.setBuildOption(m_options.build());

	// recconect
	//
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	m_pConfigSocket->reconncect();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketConfigurationLoaded()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	//
	//
	if (m_statusLoadSignals == nullptr || m_statusConnectToConfigServer == nullptr || m_statusSendToAppDaraServer == nullptr)
	{
		return;
	}

	m_statusLoadSignals->show();
	m_statusLoadSignals->setValue(0);

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: on "));
	m_statusConnectToConfigServer->setToolTip(m_pConfigSocket->cfgSrvInfo());

	//
	//
	m_statusSendToAppDaraServer->setText(tr(" AppDataService: on "));
	m_statusSendToAppDaraServer->setToolTip(m_pConfigSocket->appDataSrvInfo());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketSignalBaseLoading(int persentage)
{
	if (m_statusLoadSignals == nullptr)
	{
		return;
	}

	m_statusLoadSignals->setValue(persentage);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketSignalBaseLoaded()
{
	if (m_statusLoadSignals == nullptr)
	{
		return;
	}

	m_statusLoadSignals->hide();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::sourcesLoaded()
{
	clearViews();

	// load sources
	//
	std::vector<PS::Source*> ptrSourceList;

	int sourceCount = m_pscore.sourceBase().count();
	for(int i = 0; i < sourceCount; i++)
	{
		PS::Source* pSource = m_pscore.sourceBase().sourcePtr(i);
		if (pSource == nullptr)
		{
			continue;
		}

		ptrSourceList.push_back(pSource);
	}

	m_sourceTable.set(ptrSourceList);		// set sources
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalsLoadedInSources()
{
	m_pSourceView->setCurrentIndex(m_selectedSourceIndex);
	onSourceListClicked(m_selectedSourceIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::hideSourceColumn(int column, bool hide)
{
	if (column < 0 || column >= SOURCE_LIST_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pSourceView->hideColumn(column);
		m_pSourceColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pSourceView->showColumn(column);
		m_pSourceColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::hideSignalColumn(int column, bool hide)
{
	if (column < 0 || column >= SIGNAL_LIST_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pSignalView->hideColumn(column);
		m_pSignalColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pSignalView->showColumn(column);
		m_pSignalColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startSource()
{
	if(m_pSourceView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pSourceView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	if (m_pscore.sourceBase().count() == 0)
	{
		return;
	}

	int count = m_pSourceView->selectionModel()->selectedRows().count();
	if (count == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select source!"));
		return;
	}

	for( int i = 0; i < count; i++)
	{
		int sourceIndex = pSourceProxyModel->mapToSource(m_pSourceView->selectionModel()->selectedRows().at(i)).row();
		m_pscore.sourceBase().runSourece(sourceIndex);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopSource()
{
	if(m_pSourceView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pSourceView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int count = m_pSourceView->selectionModel()->selectedRows().count();
	if (count == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select source!"));
		return;
	}

	for( int i = 0; i < count; i++)
	{
		int sourceIndex = pSourceProxyModel->mapToSource(m_pSourceView->selectionModel()->selectedRows().at(i)).row();
		m_pscore.sourceBase().stopSourece(sourceIndex);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllSources()
{
	if(m_pSourceView == nullptr)
	{
		return;
	}

	m_pSourceView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setSignalState()
{
	if(m_pSignalView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSignalProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pSignalView->model());
	if(pSignalProxyModel == nullptr)
	{
		return;
	}

	//
	//
	QModelIndexList rows = m_pSignalView->selectionModel()->selectedRows();

	PS::Signal* pFirstSignal = nullptr;
	bool signalIsReady = false;

	if (rows.count() > 0)
	{
		int signalIndex = pSignalProxyModel->mapToSource(rows.first()).row();
		if (signalIndex >= 0 && signalIndex < m_signalTable.signalCount())
		{
			pFirstSignal = m_signalTable.signalPtr(signalIndex);
			if (pFirstSignal != nullptr)
			{
				if (pFirstSignal->regValueAddr().offset() != BAD_ADDRESS && pFirstSignal->regValueAddr().bit() != BAD_ADDRESS)
				{
					if (pFirstSignal->valueData() != nullptr)
					{
						signalIsReady = true;
					}
				}
			}
		}
	}

	//
	//
	if (pFirstSignal == nullptr || signalIsReady == false)
	{
		return;
	}

	//
	//
	SignalStateDialog dialog(pFirstSignal);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	//
	//
	int rowCount = rows.count();
	for (int r = 0; r < rowCount; r++)
	{
		int signalIndex = pSignalProxyModel->mapToSource(rows[r]).row();
		if (signalIndex < 0 || signalIndex >= m_signalTable.signalCount())
		{
			continue;
		}

		PS::Signal* pSignal = m_signalTable.signalPtr(signalIndex);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
		{
			continue;
		}

		if (pSignal->valueData() == nullptr)
		{
			continue;
		}

		if (pSignal->signalType() != pFirstSignal->signalType())
		{
			continue;
		}

		if (pSignal->isAnalog() == true)
		{
			if (compareDouble(pSignal->lowEngineeringUnits(), pFirstSignal->lowEngineeringUnits()) == false || compareDouble(pSignal->highEngineeringUnits(), pFirstSignal->highEngineeringUnits()) == false)
			{
				continue;
			}

			if (pSignal->unit() != pFirstSignal->unit())
			{
				continue;
			}
		}

		double prevState = pSignal->state();
		bool result = pSignal->setState(dialog.state());
		if (result == true)
		{
			m_pscore.history().append(SignalForLog(pSignal, prevState, pSignal->state()));
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::initSignalsState()
{
	int sourceCount = m_pscore.sourceBase().count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_pscore.sourceBase().sourcePtr(s);
		if (pSource == nullptr)
		{
			continue;
		}

		pSource->initSignalsState();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::history()
{
	SignalHistoryDialog dialog(&m_pscore.history(), this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveSignalsState()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save signals state"), "SignalState.csv", tr("CSV files (*.csv)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
	{
		return;
	}

	int sourceCount = m_pscore.sourceBase().count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_pscore.sourceBase().sourcePtr(s);
		if (pSource == nullptr)
		{
			continue;
		}

		for(PS::Signal& signal : pSource->signalList())
		{
			if (signal.regValueAddr().offset() == BAD_ADDRESS || signal.regValueAddr().bit() == BAD_ADDRESS)
			{
				continue;
			}

			file.write(signal.appSignalID().toLocal8Bit());
			file.write(";");

			file.write(QString::number(signal.state(), 'f', 10).toLocal8Bit());
			file.write(";");

			file.write("\n");
		}
	}

	file.close();

	QMessageBox::information(this, windowTitle(), tr("Save completed"));

	m_options.build().setSignalsStatePath(fileName);
	m_options.build().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::restoreSignalsState()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open signals state"), m_options.build().signalsStatePath(), tr("CSV files (*.csv);;All files (*.*)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	if (fileName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file: Empty file name"));
		return;
	}

	if (QFile::exists(fileName) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file: \"%1\"\nfile is not found!").arg(fileName));
		return;
	}

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
		return;
	}

	// read data
	//
	QTextStream in(&file);
	while (in.atEnd() == false)
	{
		QStringList line = in.readLine().split(";");
		if (line.count() < 2)
		{
			continue;
		}

		PS::Signal* pSignal = m_pscore.signalBase().signalPtr(line[0]);
		if (pSignal == nullptr)
		{
			continue;
		}

		pSignal->setState(line[1].toDouble());
	}

	file.close();

	QMessageBox::information(this, windowTitle(), tr("Restore completed"));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllSignals()
{
	if (m_pSignalView == nullptr)
	{
		return;
	}

	m_pSignalView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onOptions()
{
	OptionsDialog dialog(m_options.build(), this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	clearViews();

	// save new options
	//
	m_options.setBuild(dialog.buildOption());
	m_options.build().save();

	// restart
	//
	m_pscore.clear();
	m_pscore.setBuildOption(m_options.build());

	// recconect
	//
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	m_pConfigSocket->reconncect();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::copyText(QTableView* pView)
{
	if (pView == nullptr)
	{
		return;
	}

	QString textClipboard;

	int rowCount = pView->model()->rowCount();
	int columnCount = pView->model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (pView->isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(pView->model()->data(pView->model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::copySourceText()
{
	copyText(m_pSourceView);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::copySignalText()
{
	copyText(m_pSignalView);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::clearViews()
{
	m_sourceTable.clear();
	m_signalTable.clear();

	if (m_pFrameDataPanel != nullptr)
	{
		m_pFrameDataPanel->clear();
	}

	if (m_pFindSignalPanel != nullptr)
	{
		m_pFindSignalPanel->clear();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateSignalList(PS::Source* pSource)
{
	if (pSource == nullptr)
	{
		return;
	}

	std::vector<PS::Signal*> signalList;

	m_signalTable.clear();

	for(PS::Signal& signal : pSource->signalList())
	{
		signalList.push_back(&signal);
	}

	m_signalTable.set(signalList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateFrameDataList(PS::Source* pSource)
{
	if (pSource == nullptr)
	{
		return;
	}

	if (m_pFrameDataPanel == nullptr)
	{
		return;
	}

	std::vector<PS::FrameData*> frameDataList;

	m_pFrameDataPanel->clear();

	int count = pSource->frameBase().count();
	for(int i = 0; i < count; i++)
	{
		PS::FrameData* pFrameData = pSource->frameBase().frameDataPtr(i);
		if (pFrameData == nullptr)
		{
			continue;
		}

		frameDataList.push_back(pFrameData);
	}

	m_pFrameDataPanel->set(frameDataList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSourceContextMenu(QPoint)
{
	if (m_sourceContextMenu == nullptr)
	{
		return;
	}

	m_sourceContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSourceHeaderContextMenu(QPoint)
{
	if (m_sourceHeaderContextMenu == nullptr)
	{
		return;
	}

	m_sourceHeaderContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSourceColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
	{
		if (m_pSourceColumnAction[column] == action)
		{
			hideSourceColumn(column, !action->isChecked());
			break;
		}
	}
}
// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSignalContextMenu(QPoint)
{
	if(m_pSignalView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSignalProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pSignalView->model());
	if(pSignalProxyModel == nullptr)
	{
		return;
	}

	m_signalContextMenu = new QMenu(this);
	if (m_signalContextMenu == nullptr)
	{
		return;
	}

	//
	//
	QModelIndexList rows = m_pSignalView->selectionModel()->selectedRows();

	PS::Signal* pFirstSignal = nullptr;
	bool appendMenuItem = false;

	if (rows.count() > 0)
	{
		int signalIndex = pSignalProxyModel->mapToSource(rows.first()).row();
		if (signalIndex >= 0 && signalIndex < m_signalTable.signalCount())
		{
			pFirstSignal = m_signalTable.signalPtr(signalIndex);
			if (pFirstSignal != nullptr)
			{
				if (pFirstSignal->regValueAddr().offset() != BAD_ADDRESS && pFirstSignal->regValueAddr().bit() != BAD_ADDRESS)
				{
					if (pFirstSignal->valueData() != nullptr)
					{
						appendMenuItem = true;
					}
				}
			}
		}
	}

	if (pFirstSignal != nullptr)
	{
		int rowCount = rows.count();
		for (int r = 1; r < rowCount; r++)
		{
			int signalIndex = pSignalProxyModel->mapToSource(rows[r]).row();
			if (signalIndex < 0 || signalIndex >= m_signalTable.signalCount())
			{
				appendMenuItem = false;
				continue;
			}

			PS::Signal* pSignal = m_signalTable.signalPtr(signalIndex);
			if (pSignal == nullptr)
			{
				appendMenuItem = false;
				continue;
			}

			if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
			{
				appendMenuItem = false;
				continue;
			}

			if (pSignal->valueData() == nullptr)
			{
				appendMenuItem = false;
				continue;
			}

			if (pSignal->signalType() != pFirstSignal->signalType())
			{
				appendMenuItem = false;
				continue;
			}

			if (pSignal->isAnalog() == true)
			{
				if (compareDouble(pSignal->lowEngineeringUnits(), pFirstSignal->lowEngineeringUnits()) == false || compareDouble(pSignal->highEngineeringUnits(), pFirstSignal->highEngineeringUnits()) == false)
				{
					appendMenuItem = false;
					continue;
				}

				if (pSignal->unit() != pFirstSignal->unit() )
				{
					appendMenuItem = false;
					continue;
				}
			}
		}
	}

	//
	//
	if (appendMenuItem == true)
	{
		if (rows.count() == 1)
		{
			m_signalSetStateAction->setText( tr("Set state of signal: %1 ...").arg(pFirstSignal->customAppSignalID()));
		}
		else
		{
			m_signalSetStateAction->setText( tr("Set state for %1 signals ...").arg(rows.count()));
		}

		m_signalContextMenu->addAction(m_signalSetStateAction);
		m_signalContextMenu->addSeparator();
	}
	m_signalContextMenu->addAction(m_signalTextCopyAction);

	//
	//
	m_signalContextMenu->exec(QCursor::pos());

	//
	//
	m_signalSetStateAction->setText("Set state ...");

	//
	//
	delete m_signalContextMenu;
	m_signalContextMenu = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSignalHeaderContextMenu(QPoint)
{
	if (m_signalHeaderContextMenu == nullptr)
	{
		return;
	}

	m_signalHeaderContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSignalColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		if (m_pSignalColumnAction[column] == action)
		{
			hideSignalColumn(column, !action->isChecked());
			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutApp()
{
	DialogAbout::show(this, tr(""), ":/Images/logo.png");
	return;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSourceListClicked(const QModelIndex& index)
{
	if(m_pSourceView == nullptr)
	{
		return;
	}

	QSortFilterProxyModel* pSourceProxyModel = dynamic_cast<QSortFilterProxyModel*>(m_pSourceView->model());
	if(pSourceProxyModel == nullptr)
	{
		return;
	}

	int sourceIndex = pSourceProxyModel->mapToSource(index).row();
	if (sourceIndex < 0 || sourceIndex >= m_pscore.sourceBase().count())
	{
		return;
	}

	PS::Source*	pSource = m_pscore.sourceBase().sourcePtr(sourceIndex);
	if (pSource == nullptr)
	{
		return;
	}

	updateSignalList(pSource);
	updateFrameDataList(pSource);

	if(m_pFindSignalPanel !=nullptr)
	{
		m_pFindSignalPanel->find();
	}

	m_selectedSourceIndex = index;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSignalListDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index)

	setSignalState();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::ualTesterSocketConnect(bool isConnect)
{
	if (m_statusUalTesterClient == nullptr)
	{
		return;
	}

	if (isConnect == true)
	{
		m_statusUalTesterClient->setText(tr(" UalTester: on "));
	}
	else
	{
		m_statusUalTesterClient->setText(tr(" UalTester: off "));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveWindowState()
{
	m_options.windows().m_mainWindowPos = pos();
	m_options.windows().m_mainWindowGeometry = saveGeometry();
	m_options.windows().m_mainWindowState = saveState();

	m_options.windows().save();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::restoreWindowState()
{
	if (m_options.windows().m_mainWindowPos.x() == -1 && m_options.windows().m_mainWindowPos.y() == -1)
	{
		resize(1024, 768);
	}
	else
	{
		move(m_options.windows().m_mainWindowPos);
		restoreGeometry(m_options.windows().m_mainWindowGeometry);
		restoreState(m_options.windows().m_mainWindowState);
	}

	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	return;
}


// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	stopConfigSocket();

	clearViews();

	m_pscore.clear();

	saveWindowState();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
