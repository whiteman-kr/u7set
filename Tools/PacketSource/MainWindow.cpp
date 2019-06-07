#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QClipboard>
#include <QCloseEvent>

#include "../../lib/Ui/DialogAbout.h"

#include "PathOptionDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QMainWindow* parent)
	: QMainWindow(parent)
{
	createInterface();					// init interface

	startUpdateSourceListTimer();		// run timers for update lists
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
	setWindowIcon(QIcon(":/icons/PacketSource.png"));
	setWindowTitle(tr("Packet Source"));
	resize(1000, 700);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	createActions();
	createMenu();
	createToolBars();
	createViews();
	createContextMenu();
	createHeaderContexMenu();
	createStatusBar();
	loadSignals();
	loadSources();

	if (Rup::VERSION != PS::SUPPORT_VERSION)
	{
		QMessageBox::information(this, windowTitle(), tr("Attention!\n%1 transmits RUP packages of version %2\nLast version of RUP packages is %3").arg(windowTitle()).arg(PS::SUPPORT_VERSION).arg(Rup::VERSION));
	}

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
	//m_sourceStopAction->setEnabled(false);
	connect(m_sourceStopAction, &QAction::triggered, this, &MainWindow::stopSource);

	m_sourceSelectAllAction = new QAction(tr("Select all"), this);
	m_sourceSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);
	m_sourceSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_sourceSelectAllAction->setToolTip(tr("Select all sources"));
	connect(m_sourceSelectAllAction, &QAction::triggered, this, &MainWindow::selectAllSource);

	m_sourceOptionAction = new QAction(tr("&Options"), this);
	m_sourceOptionAction->setShortcut(Qt::CTRL + Qt::Key_O);
	m_sourceOptionAction->setIcon(QIcon(":/icons/Options.png"));
	m_sourceOptionAction->setToolTip(tr("Options of sources"));
	connect(m_sourceOptionAction, &QAction::triggered, this, &MainWindow::optionSource);

	// ?
	//
	m_pAboutAppAction = new QAction(tr("About ..."), this);
	m_pAboutAppAction->setIcon(QIcon(":/icons/About.png"));
	m_pAboutAppAction->setToolTip("");
	connect(m_pAboutAppAction, &QAction::triggered, this, &MainWindow::aboutApp);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
	QMenuBar* pMenuBar = menuBar();
	if (pMenuBar == nullptr)
	{
		return;
	}

	//
	//
	m_sourceMenu = pMenuBar->addMenu(tr("&Sources"));

	m_sourceMenu->addAction(m_sourceStartAction);
	m_sourceMenu->addAction(m_sourceStopAction);
	m_sourceMenu->addSeparator();
	m_sourceMenu->addAction(m_sourceSelectAllAction);
	m_sourceMenu->addSeparator();
	m_sourceMenu->addAction(m_sourceOptionAction);

	//
	//
	m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

	m_pInfoMenu->addAction(m_pAboutAppAction);
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
	m_mainToolBar->addAction(m_sourceOptionAction);

	return true;
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

	m_pSourceView->setModel(&m_sourceTable);
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

	m_pSignalView->setModel(&m_signalTable);
	m_pSignalView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SIGNAL_LIST_COLUMN_COUNT; column++)
	{
		m_pSignalView->setColumnWidth(column, SignalListColumnWidth[column]);
	}

	m_pSignalView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(m_pSignalView, &QTableView::doubleClicked , this, &MainWindow::onSignalListDoubleClicked);


	// View of Frame Data
	//
	m_pFrameDataView = new QTableView(this);
	if (m_pFrameDataView == nullptr)
	{
		return;
	}

	m_pFrameDataView->setModel(&m_frameDataTable);
	m_pFrameDataView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < FRAME_LIST_COLUMN_COUNT; column++)
	{
		m_pFrameDataView->setColumnWidth(column, FrameListColumnWidth[column]);
	}

	m_pFrameDataView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pFrameDataView->setMaximumWidth(180);

	connect(m_pFrameDataView, &QTableView::doubleClicked , this, &MainWindow::onFrameDataListDoubleClicked);

	// Layouts
	//

	QVBoxLayout *ssLayout = new QVBoxLayout;

	ssLayout->addWidget(m_pSourceView);
	ssLayout->addWidget(m_pSignalView);

	QHBoxLayout *mainLayout = new QHBoxLayout;

	mainLayout->addLayout(ssLayout);
	mainLayout->addWidget(m_pFrameDataView);


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

	m_pSourceView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSourceView, &QTableView::customContextMenuRequested, this, &MainWindow::onSourceContextMenu);
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

	hideSignalColumn(SIGNAL_LIST_COLUMN_APP_ID, true);
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
	m_statusServer = new QLabel(m_statusBar);
	m_statusBar->addWidget(m_statusServer);
	m_statusBar->addWidget(m_statusEmpty);

	m_statusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSignals()
{
	int signalCount = m_signalBase.readFromFile(theOptions.path().signalPath());
	if (signalCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("No single uploaded!"));
		return;

	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSources()
{
	QVector<PS::Source*> ptrSourceList;

	int sourceCount = m_sourceBase.readFromFile(theOptions.path().sourcePath(), m_signalBase);
	for(int i = 0; i < sourceCount; i++)
	{
		ptrSourceList.append(m_sourceBase.sourcePtr(i));
	}

	m_sourceTable.clear();
	m_sourceTable.set(ptrSourceList);

	m_statusServer->setText(tr(""));
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
	if (m_sourceBase.count() == 0)
	{
		return;
	}

//	m_sourceStartAction->setEnabled(false);
//	m_sourceStopAction->setEnabled(true);

	int count = m_pSourceView->selectionModel()->selectedRows().count();
	if (count == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select source!"));
		return;
	}

	for( int i = 0; i < count; i++)
	{
		m_sourceBase.runSourece(m_pSourceView->selectionModel()->selectedRows().at(i).row());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopSource()
{
//	m_sourceStartAction->setEnabled(true);
//	m_sourceStopAction->setEnabled(false);

	int count = m_pSourceView->selectionModel()->selectedRows().count();
	if (count == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select source!"));
		return;
	}

	for( int i = 0; i < count; i++)
	{
		m_sourceBase.stopSourece(m_pSourceView->selectionModel()->selectedRows().at(i).row());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllSource()
{
	m_pSourceView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::optionSource()
{
	PathOptionDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	m_sourceBase.stopAllSoureces();

	loadSources();
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

	QVector<PS::FrameData*> frameDataList;

	m_frameDataTable.clear();

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

	m_frameDataTable.set(frameDataList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSourceContextMenu(QPoint)
{
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
	int sourceIndex = index.row();
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
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onSignalListDoubleClicked(const QModelIndex& index)
{
	int signalIndex = index.row();
	if (signalIndex < 0 || signalIndex >= m_signalTable.signalCount())
	{
		return;
	}

	PS::Signal* pSignal = m_signalTable.signalPtr(signalIndex);
	if (pSignal == nullptr)
	{
		return;
	}

	if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
	{
		return;
	}

	if (pSignal->valueData() == nullptr)
	{
		return;
	}

	SignalStateDialog dialog(pSignal);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	pSignal->setState(dialog.state());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onFrameDataListDoubleClicked(const QModelIndex& index)
{
	int byteIndex = index.row();
	if (byteIndex < 0 || byteIndex >= m_frameDataTable.dataSize())
	{
		return;
	}

	quint8 byte = m_frameDataTable.byte(byteIndex);

	FrameDataStateDialog dialog(byte);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	m_frameDataTable.setByte(byteIndex, dialog.byte());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	stopUpdateSourceListTimer();

	m_sourceBase.stopAllSoureces();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
