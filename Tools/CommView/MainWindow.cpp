#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include "SerialPortDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	// init interface
	//
	createInterface();

	// run SerialPortThreads
	//
	runSerialPortThreads();

	// run timers for update lists
	//
	startCommStateTimer();
	startCommHeaderTimer();
	startCommDataTimer();
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
	setWindowIcon(QIcon(":/icons/CommView.png"));
	setWindowTitle(tr("CommView"));
	resize(700, 750);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	createActions();
	createMenu();
	createToolBars();
	createViews();
	createContextMenu();
	createHeaderContexMenu();
	createStatusBar();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createActions()
{
	// Ports
	//
	m_portReconnectAction = new QAction(tr("Reconnect"), this);
	m_portReconnectAction->setIcon(QIcon(":/icons/Reconnect.png"));
	m_portReconnectAction->setToolTip(tr("Reconnect selected serial port"));
	connect(m_portReconnectAction, &QAction::triggered, this, &MainWindow::reconnectSerialPort);

	m_portReconnectAllAction = new QAction(tr("Reconnect all"), this);
	m_portReconnectAllAction->setToolTip(tr("Reconnect all serial ports"));
	connect(m_portReconnectAllAction, &QAction::triggered, this, &MainWindow::reconnectAllSerialPort);

	m_portOptionAction = new QAction(tr("&Options ..."), this);
	m_portOptionAction->setShortcut(Qt::CTRL + Qt::Key_O);
	m_portOptionAction->setIcon(QIcon(":/icons/Options.png"));
	m_portOptionAction->setToolTip(tr("Options of selected serial port"));
	connect(m_portOptionAction, &QAction::triggered, this, &MainWindow::optionSerialPort);

	// View
	//
	m_showHeaderAction = new QAction(tr("Show header"), this);
	m_showHeaderAction->setCheckable(true);
	m_showHeaderAction->setChecked(theOptions.view().showHeader());
	connect(m_showHeaderAction, &QAction::triggered, this, &MainWindow::showHeader);

	m_showInWordAction = new QAction(tr("Show data in words"), this);
	m_showInWordAction->setCheckable(true);
	m_showInWordAction->setChecked(theOptions.view().showInWord());
	m_showInWordAction->setIcon(QIcon(":/icons/toWord.png"));
	connect(m_showInWordAction, &QAction::triggered, this, &MainWindow::showInWord);

	m_showInHexAction = new QAction(tr("Show data in Hex"), this);
	m_showInHexAction->setCheckable(true);
	m_showInHexAction->setChecked(theOptions.view().showInHex());
	m_showInHexAction->setIcon(QIcon(":/icons/toHex.png"));
	connect(m_showInHexAction, &QAction::triggered, this, &MainWindow::showInHex);

	m_showInFloatAction = new QAction(tr("Show data in Float"), this);
	m_showInFloatAction->setCheckable(true);
	m_showInFloatAction->setChecked(theOptions.view().showInFloat());
	m_showInFloatAction->setIcon(QIcon(":/icons/toFloat.png"));
	connect(m_showInFloatAction, &QAction::triggered, this, &MainWindow::showInFloat);

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
	m_portsMenu = pMenuBar->addMenu(tr("&Ports"));

	m_portsMenu->addAction(m_portReconnectAction);
	m_portsMenu->addAction(m_portReconnectAllAction);
	m_portsMenu->addSeparator();
	m_portsMenu->addAction(m_portOptionAction);

	//
	//
	m_viewMenu = pMenuBar->addMenu(tr("&View"));

	m_viewMenu->addAction(m_showHeaderAction);
	m_viewMenu->addSeparator();
	m_viewMenu->addAction(m_showInWordAction);
	m_viewMenu->addAction(m_showInHexAction);
	m_viewMenu->addAction(m_showInFloatAction);

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
	m_mainToolBar->setWindowTitle(tr("Control panel of serial ports"));
	m_mainToolBar->setObjectName(m_mainToolBar->windowTitle());
	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(m_mainToolBar);

	m_mainToolBar->addAction(m_portReconnectAction);
	m_mainToolBar->addSeparator();
	m_mainToolBar->addAction(m_showInWordAction);
	m_mainToolBar->addAction(m_showInHexAction);
	m_mainToolBar->addAction(m_showInFloatAction);
	m_mainToolBar->addSeparator();
	m_mainToolBar->addAction(m_portOptionAction);
	m_mainToolBar->addSeparator();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createViews()
{
	// View of state
	//
	m_pCommStateView = new QTableView(this);
	m_pCommStateView->setModel(&m_commStateTable);
	m_pCommStateView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < COMM_STATE_LIST_COLUMN_COUNT; column++)
	{
		m_pCommStateView->setColumnWidth(column, LIST_COLUMN_WITDH);
	}

	m_pCommStateView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pCommStateView->setFixedHeight(150);

	connect(m_pCommStateView, &QTableView::doubleClicked , this, &MainWindow::optionSerialPort);

	updateCommStateList();


	// View of header
	//
	m_pCommHeaderView = new QTableView(this);
	m_pCommHeaderView->setModel(&m_commHeaderTable);
	m_pCommHeaderView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SERIAL_PORT_COUNT; column++)
	{
		m_pCommHeaderView->setColumnWidth(column, LIST_COLUMN_WITDH);
	}

	m_pCommHeaderView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pCommHeaderView->setFixedHeight(210);

	if (theOptions.view().showHeader() == false)
	{
		m_pCommHeaderView->hide();
	}

	updateCommHeaderList();

	// View of data
	//
	m_pCommDataView = new QTableView(this);
	m_pCommDataView->setModel(&m_commDataTable);
	m_pCommDataView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SERIAL_PORT_COUNT; column++)
	{
		m_pCommDataView->setColumnWidth(column, LIST_COLUMN_WITDH);
	}

	updateCommDataList();

	// Layouts
	//
	QWidget* pWidget = new QWidget(this);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pCommStateView);
	mainLayout->addWidget(m_pCommHeaderView);
	mainLayout->addWidget(m_pCommDataView);

	pWidget->setLayout(mainLayout);

	setCentralWidget(pWidget);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createContextMenu()
{
	// View of state
	//
	m_commStateContextMenu = new QMenu(this);

	m_commStateContextMenu->addAction(m_portReconnectAction);
	m_commStateContextMenu->addAction(m_portReconnectAllAction);
	m_commStateContextMenu->addSeparator();
	m_commStateContextMenu->addAction(m_portOptionAction);

	m_pCommStateView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pCommStateView, &QTableView::customContextMenuRequested, this, &MainWindow::onContextCommStateMenu);

	// View of data
	//
	m_commDataContextMenu = new QMenu(this);

	m_commDataContextMenu->addAction(m_showInWordAction);
	m_commDataContextMenu->addAction(m_showInHexAction);
	m_commDataContextMenu->addAction(m_showInFloatAction);

	m_pCommDataView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pCommDataView, &QTableView::customContextMenuRequested, this, &MainWindow::onContextCommDataMenu);

}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createHeaderContexMenu()
{
	// init header context menu for View of state
	//
	m_pCommStateView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pCommStateView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pCommStateView);

	for(int column = 0; column < COMM_STATE_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(CommStateColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MainWindow::onColumnAction);
		}
	}

    hideColumn(COMM_STATE_LIST_COLUMN_PACKETS, true);
	hideColumn(COMM_STATE_LIST_COLUMN_QUEUE, true);
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
	m_statusPortConnectedCount = new QLabel(m_statusBar);
	m_statusBar->addWidget(m_statusPortConnectedCount);
	m_statusBar->addWidget(m_statusEmpty);

	m_statusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::runSerialPortThread(int index)
{
	SerialPortOption* portOption = theOptions.serialPorts().port(index);
	if (portOption == nullptr)
	{
		return false;
	}

	connect(portOption, SIGNAL(connectChanged()), this, SLOT(portConnectedChanged()), Qt::QueuedConnection);

	SerialPortWorker* pWorker = new SerialPortWorker(portOption);
	if (pWorker == nullptr)
	{
		return false;
	}

	QThread* pThread = new QThread;
	if (pThread == nullptr)
	{
		delete pWorker;
		return false;
	}

	pWorker->moveToThread(pThread);

	connect(pThread, SIGNAL(started()), pWorker, SLOT(process()));	// on start thread run method process()
	connect(pWorker, SIGNAL(finished()), pThread, SLOT(quit()));	// on finish() run slot quit()

	connect(pWorker, SIGNAL(finished()), pWorker, SLOT(deleteLater()));
	connect(pThread, SIGNAL(finished()), pThread, SLOT(deleteLater()));

	pThread->start();												// run thread that runs process()

	m_pWorker[index] = pWorker;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runSerialPortThreads()
{
	for(int i = 0; i < SERIAL_PORT_COUNT; i++ )
	{
		runSerialPortThread(i);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopSerialPortThreads()
{
	for(int i = 0; i < SERIAL_PORT_COUNT; i++)
	{
		m_pWorker[i]->finish();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommStateList()
{
	m_commStateTable.clear();

	QList<SerialPortOption*> portOptionList;

	for(int i = 0; i < SERIAL_PORT_COUNT; i++)
	{
		SerialPortOption* portOption = theOptions.serialPorts().port(i);
		if (portOption == nullptr)
		{
			continue;
		}

		portOptionList.append(portOption);
	}

	m_commStateTable.set(portOptionList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommHeaderList()
{

}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommDataList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= COMM_STATE_LIST_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pCommStateView->hideColumn(column);
		m_pColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pCommStateView->showColumn(column);
		m_pColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startCommStateTimer()
{
	if (m_updateCommStateTimer == nullptr)
	{
		m_updateCommStateTimer = new QTimer(this);
		connect(m_updateCommStateTimer, &QTimer::timeout, this, &MainWindow::updateCommState);
	}

	m_updateCommStateTimer->start(UPDATE_COMM_STATE_TIMEOUT);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopCommStateTimer()
{
	if (m_updateCommStateTimer != nullptr)
	{
		m_updateCommStateTimer->stop();
		delete m_updateCommStateTimer;
		m_updateCommStateTimer = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startCommHeaderTimer()
{
	if (m_updateCommHeaderTimer == nullptr)
	{
		m_updateCommHeaderTimer = new QTimer(this);
		connect(m_updateCommHeaderTimer, &QTimer::timeout, this, &MainWindow::updateCommHeader);
	}

	m_updateCommHeaderTimer->start(UPDATE_COMM_HEADER_TIMEOUT);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopCommHeaderTimer()
{
	if (m_updateCommHeaderTimer != nullptr)
	{
		m_updateCommHeaderTimer->stop();
		delete m_updateCommHeaderTimer;
		m_updateCommHeaderTimer = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startCommDataTimer()
{
	if (m_updateCommDataTimer == nullptr)
	{
		m_updateCommDataTimer = new QTimer(this);
		connect(m_updateCommDataTimer, &QTimer::timeout, this, &MainWindow::updateCommData);
	}

	m_updateCommDataTimer->start(UPDATE_COMM_DATA_TIMEOUT);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopCommDataTimer()
{
	if (m_updateCommDataTimer != nullptr)
	{
		m_updateCommDataTimer->stop();
		delete m_updateCommDataTimer;
		m_updateCommDataTimer = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::reconnectSerialPort()
{
	int index = m_pCommStateView->currentIndex().row();
	if (index < 0 || index >= m_commStateTable.portCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select port for reconnect!"));
		return;
	}

	if (m_pWorker[index] == nullptr)
	{
		return;
	}

	m_pWorker[index]->reopenSerialPort();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::reconnectAllSerialPort()
{
	for(int i = 0; i < SERIAL_PORT_COUNT; i++ )
	{
		if (m_pWorker[i] == nullptr)
		{
			continue;
		}

		m_pWorker[i]->reopenSerialPort();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::optionSerialPort()
{
	int index = m_pCommStateView->currentIndex().row();
	if (index < 0 || index >= SERIAL_PORT_COUNT)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select port for change options!"));
		return;
	}

	SerialPortOption* portOption = theOptions.serialPorts().port(index);
	if (portOption == nullptr)
	{
		return;
	}

	SerialPortDialog dialog(*portOption, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	SerialPortWorker* pWorker = m_pWorker[index];
	if (pWorker == nullptr)
	{
		return;
	}

	pWorker->finish();

	theOptions.serialPorts().setPort(index, dialog.option());
	theOptions.serialPorts().port(index)->save(index);
	theOptions.serialPorts().recalcDataSize();

	runSerialPortThread(index);

	m_commStateTable.updateColumns();
	m_commDataTable.reset();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showHeader()
{
	theOptions.view().setShowHeader(!theOptions.view().showHeader());
	theOptions.view().save();

	if (theOptions.view().showHeader() == true)
	{
		m_pCommHeaderView->show();
	}
	else
	{
		m_pCommHeaderView->hide();
	}

	m_commDataTable.reset();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showInWord()
{
	theOptions.view().setShowInWord(!theOptions.view().showInWord());
	theOptions.view().save();

	m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_SIZE);
	m_commDataTable.reset();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showInHex()
{
	theOptions.view().setShowInHex(!theOptions.view().showInHex());
	theOptions.view().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showInFloat()
{
	theOptions.view().setShowInFloat(!theOptions.view().showInFloat());
	theOptions.view().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextCommStateMenu(QPoint)
{
	m_commStateContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextCommDataMenu(QPoint)
{
	m_commDataContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < COMM_STATE_LIST_COLUMN_COUNT; column++)
	{
		if (m_pColumnAction[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutApp()
{
	QDialog aboutDialog(this);
	aboutDialog.setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	aboutDialog.setFixedSize(200, 50);
	aboutDialog.setWindowIcon(QIcon(":/icons/About.png"));
	aboutDialog.setWindowTitle(tr("CommView"));

		QVBoxLayout *mainLayout = new QVBoxLayout;

		QLabel* versionLabel = new QLabel(tr("Version 1.2"), &aboutDialog);
		versionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		mainLayout->addWidget(versionLabel);

	aboutDialog.setLayout(mainLayout);
	aboutDialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommState()
{
    m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_PACKETS);
    m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_RECEIVED);
	m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_SKIPPED);
	m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_QUEUE);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommHeader()
{
	m_commHeaderTable.updateColumns();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateCommData()
{
	m_commDataTable.updateColumns();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::portConnectedChanged()
{
	m_commStateTable.updateColumn(COMM_STATE_LIST_COLUMN_PORT);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	stopCommStateTimer();
	stopCommHeaderTimer();
	stopCommDataTimer();

	stopSerialPortThreads();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------


