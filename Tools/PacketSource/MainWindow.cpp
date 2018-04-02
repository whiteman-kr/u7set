#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QClipboard>
#include <QCloseEvent>

#include "SourceOptionDialog.h"
#include "version.h"

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
	resize(700, 750);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	createActions();
	createMenu();
	createToolBars();
	createViews();
	createContextMenu();
	createHeaderContexMenu();
	createStatusBar();

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
	hideColumn(SOURCE_LIST_COLUMN_FRAME_COUNT, true);
	hideColumn(SOURCE_LIST_COLUMN_SERVER_IP, true);
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

	m_sourceTable.clear();

	int sourceCount = theSourceBase.readFromXml();
	if (sourceCount == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("Sources is not loaded!"));
		return;
	}

	m_statusServer->setText(tr(""));

	QVector<SourceItem*> ptrSourceList;
	for(int i = 0; i < sourceCount; i++)
	{
		ptrSourceList.append(theSourceBase.sourcePtr(i));
	}


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

	QHBoxLayout* hl = new QHBoxLayout;

	QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(":/icons/Logo.png"));

	hl->addWidget(logo);

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() + ": version " + qApp->applicationVersion() + "</h3>";
#ifndef Q_DEBUG
	text += "Build: Release";
#else
	text += "Build: Debug";
#endif
	text += "<br>Commit date: " LAST_SERVER_COMMIT_DATE;
	text += "<br>Commit SHA1: " USED_SERVER_COMMIT_SHA;
	text += "<br>Qt version: " QT_VERSION_STR;

	QLabel* label = new QLabel(text, &aboutDialog);
	label->setIndent(10);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vl->addWidget(label);

	QPushButton* copyCommitSHA1Button = new QPushButton("Copy commit SHA1");
	connect(copyCommitSHA1Button, &QPushButton::clicked, [](){
		qApp->clipboard()->setText(USED_SERVER_COMMIT_SHA);
	});

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(copyCommitSHA1Button, QDialogButtonBox::ActionRole);
	buttonBox->addButton(QDialogButtonBox::Ok);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hl);
	mainLayout->addWidget(buttonBox);
	aboutDialog.setLayout(mainLayout);

	connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);

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
