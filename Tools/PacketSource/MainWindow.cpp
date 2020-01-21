#include "MainWindow.h"
#include "../../lib/Ui/DialogAbout.h"
#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QMainWindow* parent)
	: QMainWindow(parent)
{
	createInterface();					// init interface
	restoreWindowState();

	runUalTesterServerThread();

	connect(&m_sourceBase, &SourceBase::sourcesLoaded, this, &MainWindow::loadSignals, Qt::QueuedConnection);
	connect(&m_signalBase, &SignalBase::signalsLoaded, this, &MainWindow::loadSignalsInSources, Qt::QueuedConnection);

	startUpdateSourceListTimer();		// run timers for update lists

	loadSources();

#if RUP_VERSION != PS_SUPPORT_VERSION

#error Current version of Rup packets is unknown

//	if (Rup::VERSION != PS::SUPPORT_VERSION)
//	{
//		QMessageBox::information(this, windowTitle(), tr("Attention!\n%1 transmits RUP packages of version %2\nLast known version of RUP packages is %3").arg(windowTitle()).arg(PS::SUPPORT_VERSION).arg(Rup::VERSION));
//	}

#endif
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
	m_sourceStartAction->setIcon(QIcon(":/icons/Start.png"));
	m_sourceStartAction->setToolTip(tr("Start all sources"));
	connect(m_sourceStartAction, &QAction::triggered, this, &MainWindow::startSource);

	m_sourceStopAction = new QAction(tr("Stop"), this);
	m_sourceStopAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
	m_sourceStopAction->setIcon(QIcon(":/icons/Stop.png"));
	m_sourceStopAction->setToolTip(tr("Stop all sources"));
	connect(m_sourceStopAction, &QAction::triggered, this, &MainWindow::stopSource);

	m_sourceReloadAction = new QAction(tr("Reload"), this);
	m_sourceReloadAction->setIcon(QIcon(":/icons/Reload.png"));
	m_sourceReloadAction->setToolTip(tr("Reload all sources and signals"));
	connect(m_sourceReloadAction, &QAction::triggered, this, &MainWindow::reloadSource);

	m_sourceSelectAllAction = new QAction(tr("Select all"), this);
	m_sourceSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_sourceSelectAllAction->setToolTip(tr("Select all sources"));
	connect(m_sourceSelectAllAction, &QAction::triggered, this, &MainWindow::selectAllSources);

	// Signals
	//
	m_signalSetStateAction = new QAction(tr("Set state ..."), this);
	m_signalSetStateAction->setToolTip(tr("Set signal state"));
	connect(m_signalSetStateAction, &QAction::triggered, this, &MainWindow::setSignalState);

	m_signalInitAction = new QAction(tr("Initialization"), this);
	m_signalInitAction->setShortcut(Qt::CTRL + Qt::Key_I);
	m_signalInitAction->setIcon(QIcon(":/icons/Init.png"));
	m_signalInitAction->setToolTip(tr("Initialization all signals"));
	connect(m_signalInitAction, &QAction::triggered, this, &MainWindow::initSignalsState);

	m_signalHistoryAction = new QAction(tr("History ..."), this);
	m_signalHistoryAction->setShortcut(Qt::CTRL + Qt::Key_H);
	m_signalHistoryAction->setIcon(QIcon(":/icons/History.png"));
	m_signalHistoryAction->setToolTip(tr("Hystory of signals state"));
	connect(m_signalHistoryAction, &QAction::triggered, this, &MainWindow::history);

	m_signalSaveStatesAction = new QAction(tr("Save signals state ..."), this);
	m_signalSaveStatesAction->setShortcut(Qt::CTRL + Qt::Key_S);
	m_signalSaveStatesAction->setIcon(QIcon(":/icons/Import.png"));
	m_signalSaveStatesAction->setToolTip(tr("Save signals state"));
	connect(m_signalSaveStatesAction, &QAction::triggered, this, &MainWindow::saveSignalsState);

	m_signalRestoreStatesAction = new QAction(tr("Restore signals state ..."), this);
	m_signalRestoreStatesAction->setShortcut(Qt::CTRL + Qt::Key_R);
	m_signalRestoreStatesAction->setIcon(QIcon(":/icons/Export.png"));
	m_signalRestoreStatesAction->setToolTip(tr("Restore signals state"));
	connect(m_signalRestoreStatesAction, &QAction::triggered, this, &MainWindow::restoreSignalsState);


	m_signalSelectAllAction = new QAction(tr("Select all"), this);
	m_signalSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_signalSelectAllAction->setToolTip(tr("Select all signals"));
	connect(m_signalSelectAllAction, &QAction::triggered, this, &MainWindow::selectAllSignals);

	// ?
	//
	m_optionAction = new QAction(tr("&Options"), this);
	m_optionAction->setShortcut(Qt::CTRL + Qt::Key_O);
	m_optionAction->setIcon(QIcon(":/icons/Options.png"));
	m_optionAction->setToolTip(tr("Options of sources"));
	connect(m_optionAction, &QAction::triggered, this, &MainWindow::onOptions);

	m_pAboutAppAction = new QAction(tr("About ..."), this);
	m_pAboutAppAction->setIcon(QIcon(":/icons/About.png"));
	m_pAboutAppAction->setToolTip("");
	connect(m_pAboutAppAction, &QAction::triggered, this, &MainWindow::aboutApp);

	// source contex menu
	//
	m_sourceTextCopyAction = new QAction(tr("&Copy"), this);
	m_sourceTextCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	connect(m_sourceTextCopyAction, &QAction::triggered, this, &MainWindow::copySourceText);

	// signal contex menu
	//
	m_signalTextCopyAction = new QAction(tr("&Copy"), this);
	m_signalTextCopyAction->setIcon(QIcon(":/icons/Copy.png"));
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
	m_sourceMenu->addAction(m_sourceReloadAction);
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

	// ?
	//
	m_infoMenu = pMenuBar->addMenu(tr("?"));

	m_infoMenu->addAction(m_optionAction);
	m_infoMenu->addSeparator();
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
	m_mainToolBar->addAction(m_sourceReloadAction);
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
			dataAction->setIcon(QIcon(":/icons/FrameData.png"));
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
			findAction->setIcon(QIcon(":/icons/Find.png"));
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
	m_sourceContextMenu->addAction(m_sourceReloadAction);
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
	hideSourceColumn(SOURCE_LIST_COLUMN_SERVER_IP, true);
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
	m_statusUalTesterClient = new QLabel(m_statusBar);
	m_statusBar->addWidget(m_statusUalTesterClient);
	m_statusBar->addWidget(m_statusEmpty);

	m_statusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());

	m_statusUalTesterClient->setText(tr(" UalTester: off "));
	m_statusUalTesterClient->setStyleSheet("color: rgb(160, 160, 160);");

	m_statusEmpty->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runUalTesterServerThread()
{
	SoftwareInfo si;
	si.init(E::SoftwareType::TestClient, "TEST_SERVER_ID", 1, 0);

	m_ualTesterSever = new UalTesterServer(si, &m_signalBase);
	if (m_ualTesterSever == nullptr)
	{
		return;
	}

	HostAddressPort ualTesterIP = HostAddressPort(theOptions.build().ualTesterIP(), PORT_TUNING_SERVICE_CLIENT_REQUEST);

	m_ualTesterServerThread = new UalTesterServerThread(ualTesterIP, m_ualTesterSever, nullptr);
	if (m_ualTesterServerThread == nullptr)
	{
		return;
	}

	connect(m_ualTesterSever, &UalTesterServer::connectionChanged, this, &MainWindow::ualTesterSocketConnect, Qt::QueuedConnection);
	connect(m_ualTesterSever, &UalTesterServer::signalStateChanged, this, &MainWindow::signalStateChanged, Qt::QueuedConnection);

	m_ualTesterServerThread->start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopUalTesterServerThread()
{
	if (m_ualTesterServerThread == nullptr)
	{
		return;
	}

	m_ualTesterServerThread->quitAndWait();
	delete m_ualTesterServerThread;
	m_ualTesterServerThread = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSources()
{
	// clear signals
	//
	m_signalTable.clear();
	m_signalBase.clear();

	// clear data
	//
	if (m_pFrameDataPanel != nullptr)
	{
		m_pFrameDataPanel->clear();
	}

	// load sources
	//
	QVector<PS::Source*> ptrSourceList;

	int sourceCount = m_sourceBase.readFromFile();
	for(int i = 0; i < sourceCount; i++)
	{
		ptrSourceList.append(m_sourceBase.sourcePtr(i));
	}

	qDebug() << "Loaded sources:" << sourceCount;

	m_sourceTable.clear();
	m_sourceTable.set(ptrSourceList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSignals()
{
	int signalCount = m_signalBase.readFromFile();
	if (signalCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("No single uploaded!"));
		return;
	}

	qDebug() << "Loaded signals:" << signalCount;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSignalsInSources()
{
	int sourceCount = m_sourceBase.count();
	for(int i = 0; i < sourceCount; i++)
	{
		PS::Source* pSource = m_sourceBase.sourcePtr(i);
		if (pSource == nullptr)
		{
			continue;
		}

		pSource->loadSignals(m_signalBase);
		pSource->initSignalsState();
	}

	m_signalBase.restoreSignalsState();
	m_sourceBase.restoreSourcesState();

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

void MainWindow::startUpdateSourceListTimer()
{
	if (m_updateSourceListTimer == nullptr)
	{
		m_updateSourceListTimer = new QTimer(this);
		connect(m_updateSourceListTimer, &QTimer::timeout, this, &MainWindow::updateSourceState);
	}

	m_updateSourceListTimer->start(UPDATE_SOURCE_STATE_TIMEOUT);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopUpdateSourceListTimer()
{
	if (m_updateSourceListTimer != nullptr)
	{
		m_updateSourceListTimer->stop();
		delete m_updateSourceListTimer;
		m_updateSourceListTimer = nullptr;
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

	if (m_sourceBase.count() == 0)
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
		m_sourceBase.runSourece(sourceIndex);
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
		m_sourceBase.stopSourece(sourceIndex);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::reloadSource()
{
	// save states
	//
	int sourceCount = m_sourceBase.count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_sourceBase.sourcePtr(s);
		if (pSource == nullptr)
		{
			continue;
		}

		if(pSource->isRunning() == true)
		{
			m_sourceBase.saveSourceState(pSource);
		}

		int signalCount = pSource->signalList().count();
		for(int i = 0; i < signalCount; i++)
		{
			PS::Signal* pSignal = &pSource->signalList()[i];
			if ( pSignal == nullptr)
			{
				continue;
			}

			if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
			{
				continue;
			}

			m_signalBase.saveSignalState(pSignal);
		}
	}

	// reload
	//
	loadSources();

	m_pSourceView->setCurrentIndex(m_selectedSourceIndex);
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
			m_signalSateLog.append( SignalForLog(pSignal, prevState, pSignal->state()) );
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::initSignalsState()
{
	int sourceCount = m_sourceBase.count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_sourceBase.sourcePtr(s);
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
	SignalStateLogDialog dialog(&m_signalSateLog, this);
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

	QFile file;
	file.setFileName(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
	{
		return;
	}

	int sourceCount = m_sourceBase.count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_sourceBase.sourcePtr(s);
		if (pSource == nullptr)
		{
			continue;
		}

		int sigalCount = pSource->signalList().count();
		for(int i = 0; i < sigalCount; i++)
		{
			PS::Signal* pSignal = &pSource->signalList()[i];
			if ( pSignal == nullptr)
			{
				continue;
			}

			if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
			{
				continue;
			}

			file.write(pSignal->appSignalID().toLocal8Bit());
			file.write(";");

			file.write(QString::number(pSignal->state(), 'f', 10).toLocal8Bit());
			file.write(";");

			file.write("\n");
		}
	}

	file.close();

	QMessageBox::information(this, windowTitle(), tr("Save completed"));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::restoreSignalsState()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open signals state"), "SignalState.csv", "CSV files (*.csv);;All files (*.*)");
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

		PS::Signal* pSignal = m_signalBase.signalPtr(line[0]);
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
	QString ualTesterIP = theOptions.build().ualTesterIP();

	OptionsDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	loadSources();

	if (ualTesterIP != theOptions.build().ualTesterIP())
	{
		stopUalTesterServerThread();
		runUalTesterServerThread();
	}
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

void MainWindow::updateSignalList(PS::Source* pSource)
{
	if (pSource == nullptr)
	{
		return;
	}

	QVector<PS::Signal*> signalList;

	m_signalTable.clear();

	int count = pSource->signalList().count();
	for(int i = 0; i < count; i++)
	{
		PS::Signal* pSignal = &pSource->signalList()[i];
		if ( pSignal == nullptr)
		{
			continue;
		}

		signalList.append(pSignal);
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

	QVector<PS::FrameData*> frameDataList;

	m_pFrameDataPanel->clear();

	int count = pSource->frameBase().count();
	for(int i = 0; i < count; i++)
	{
		PS::FrameData* pFrameData = pSource->frameBase().frameDataPtr(i);
		if (pFrameData == nullptr)
		{
			continue;
		}

		frameDataList.append(pFrameData);
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

void MainWindow::updateSourceState()
{
	m_sourceTable.updateColumn(SOURCE_LIST_COLUMN_STATE);
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
	if (sourceIndex < 0 || sourceIndex >= m_sourceBase.count())
	{
		return;
	}

	PS::Source*	pSource = m_sourceBase.sourcePtr(sourceIndex);
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
	Q_UNUSED(index);

	setSignalState();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::ualTesterSocketConnect(bool isConnect)
{
	if (isConnect == true)
	{
		m_statusUalTesterClient->setText(tr(" UalTester: on "));
		m_statusUalTesterClient->setStyleSheet("color: rgb(0, 0, 0);");
	}
	else
	{
		m_statusUalTesterClient->setText(tr(" UalTester: off "));
		m_statusUalTesterClient->setStyleSheet("color: rgb(160, 160, 160);");
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalStateChanged(Hash hash, double prevState, double state)
{
	if (hash == UNDEFINED_HASH)
	{
		return;
	}

	PS::Signal* pSignal = m_signalBase.signalPtr(hash);
	if (pSignal == nullptr)
	{
		return;
	}

	m_signalSateLog.append( SignalForLog(pSignal, prevState, state) );
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveWindowState()
{
	theOptions.windows().m_mainWindowPos = pos();
	theOptions.windows().m_mainWindowGeometry = saveGeometry();
	theOptions.windows().m_mainWindowState = saveState();

	theOptions.windows().save();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::restoreWindowState()
{
	if (theOptions.windows().m_mainWindowPos.x() == -1 && theOptions.windows().m_mainWindowPos.y() == -1)
	{
		resize(1024, 768);
	}
	else
	{
		move(theOptions.windows().m_mainWindowPos);
		restoreGeometry(theOptions.windows().m_mainWindowGeometry);
		restoreState(theOptions.windows().m_mainWindowState);
	}

	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	stopUpdateSourceListTimer();
	stopUalTesterServerThread();

	m_signalBase.clear();
	m_sourceBase.clear();

	saveWindowState();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
