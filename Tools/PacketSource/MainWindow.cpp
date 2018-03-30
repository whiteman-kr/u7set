#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QCloseEvent>

#include "SourceOptionDialog.h"

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
	setWindowTitle(tr("Packet source 1.0"));
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
	// Sources
	//
	m_sourceStartAction = new QAction(tr("Start"), this);
	m_sourceStartAction->setIcon(QIcon(":/icons/Start.png"));
	m_sourceStartAction->setToolTip(tr("Start all sources"));
	connect(m_sourceStartAction, &QAction::triggered, this, &MainWindow::startSource);

	m_sourceStopAction = new QAction(tr("Stop"), this);
	m_sourceStopAction->setIcon(QIcon(":/icons/Stop.png"));
	m_sourceStopAction->setToolTip(tr("Stop all sources"));
	m_sourceStopAction->setEnabled(false);
	connect(m_sourceStopAction, &QAction::triggered, this, &MainWindow::stopSource);

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
	m_pSourceView->setModel(&m_sourceTable);
	m_pSourceView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
	{
		m_pSourceView->setColumnWidth(column, LIST_COLUMN_WITDH);
	}

	m_pSourceView->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Layouts
	//
	QWidget* pWidget = new QWidget(this);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pSourceView);

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
	connect(m_pSourceView, &QTableView::customContextMenuRequested, this, &MainWindow::onContextSourceMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createHeaderContexMenu()
{
	// init header context menu for View of sources
	//
	m_pSourceView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSourceView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &MainWindow::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pSourceView);

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(SourceListColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &MainWindow::onColumnAction);
		}
	}

	hideColumn(SOURCE_LIST_COLUMN_DATA_TYPE, true);
	hideColumn(SOURCE_LIST_COLUMN_MODULE_TYPE, true);
	hideColumn(SOURCE_LIST_COLUMN_SUB_SYSTEM, true);
	hideColumn(SOURCE_LIST_COLUMN_CAPTION, true);
	hideColumn(SOURCE_LIST_COLUMN_FRAME_COUNT, true);
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
	m_statusSourceCount = new QLabel(m_statusBar);
	m_statusBar->addWidget(m_statusSourceCount);
	m_statusBar->addWidget(m_statusEmpty);

	m_statusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= SOURCE_LIST_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pSourceView->hideColumn(column);
		m_pColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pSourceView->showColumn(column);
		m_pColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startUpdateSourceListTimer()
{
	if (m_updateSourceListTimer == nullptr)
	{
		m_updateSourceListTimer = new QTimer(this);
		//connect(m_updateSourceListTimer, &QTimer::timeout, this, &MainWindow::updateSource);
	}

	m_updateSourceListTimer->start(UPDATE_SOURCE_TIMEOUT);
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
	if (theSourceBase.count() == 0)
	{
		return;
	}

	m_sourceStartAction->setEnabled(false);
	m_sourceStopAction->setEnabled(true);

	theSourceBase.runAllSoureces();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopSource()
{
	m_sourceStartAction->setEnabled(true);
	m_sourceStopAction->setEnabled(false);

	theSourceBase.stopAllSoureces();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::optionSource()
{
	SourceOptionDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	stopSource();

	int sourceCount = theSourceBase.readFromXml();
	if (sourceCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("This file does not contain any sources!"));
	}

	m_statusSourceCount->setText(tr(" Sources count: %1  ").arg(sourceCount));

	QVector<SourceItem*> ptrSourceList;
	for(int i = 0; i < sourceCount; i++)
	{
		ptrSourceList.append(theSourceBase.sourcePtr(i));
	}

	m_sourceTable.clear();
	m_sourceTable.set(ptrSourceList);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextSourceMenu(QPoint)
{
	m_sourceContextMenu->exec(QCursor::pos());
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

	for(int column = 0; column < SOURCE_LIST_COLUMN_COUNT; column++)
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
	aboutDialog.setWindowTitle(tr("Packet Source"));

		QVBoxLayout *mainLayout = new QVBoxLayout;

		QLabel* versionLabel = new QLabel(tr("Version 1.0"), &aboutDialog);
		versionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		mainLayout->addWidget(versionLabel);

	aboutDialog.setLayout(mainLayout);
	aboutDialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{

	if (theSourceBase.sourcesIsRunning() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, stop all sources!"));
		e->ignore();
		return;
	}

	stopUpdateSourceListTimer();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
