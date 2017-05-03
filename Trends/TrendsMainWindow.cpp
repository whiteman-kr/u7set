#include "TrendsMainWindow.h"
#include "ui_TrendsMainWindow.h"
#include "Settings.h"
#include "TrendWidget.h"

TrendsMainWindow::TrendsMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TrendsMainWindow)
{
	ui->setupUi(this);

	m_trendWidget = new TrendLib::TrendWidget(&m_signalSet);
	setCentralWidget(m_trendWidget);

	m_trendWidget->setView(static_cast<TrendLib::TrendView>(theSettings.m_viewType));
	m_trendWidget->setLaneCount(theSettings.m_laneCount);

	//--
	//

	connect(ui->actionOpen, &QAction::triggered, this, &TrendsMainWindow::actionOpenTriggered);
	connect(ui->actionSave, &QAction::triggered, this, &TrendsMainWindow::actionSaveTriggered);
	connect(ui->actionPrint, &QAction::triggered, this, &TrendsMainWindow::actionPrintTriggered);
	connect(ui->actionExit, &QAction::triggered, this, &TrendsMainWindow::actionExitTriggered);
	connect(ui->actionAbout, &QAction::triggered, this, &TrendsMainWindow::actionAboutTriggered);

	createToolBar();

	connect(m_viewCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendsMainWindow::viewComboCurrentIndexChanged);
	connect(m_lanesCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendsMainWindow::laneCountComboCurrentIndexChanged);

	setMinimumSize(500, 300);
	restoreWindowState();

	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	TrendLib::TrendSignal s1;
	s1.setEquipmnetId("SIGNAL001");
	s1.setCaption("Tren Signal 001");
	s1.setType(E::SignalType::Analog);
	s1.setLowLimit(10.0);
	s1.setHighLimit(105.0);

	TrendLib::TrendSignal s2;
	s2.setEquipmnetId("SIGNAL002");
	s2.setCaption("Tren Signal 002");
	s2.setType(E::SignalType::Discrete);

	m_signalSet.addSignal(s1);
	m_signalSet.addSignal(s2);
	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return;
}

TrendsMainWindow::~TrendsMainWindow()
{
	delete ui;
}

void TrendsMainWindow::createToolBar()
{
	m_toolBar = new QToolBar(this);
	m_toolBar->setObjectName("TrendToolBar");

	m_toolBar->setMovable(false);
	m_toolBar->setIconSize(QSize(28, 28));


	m_toolBar->addAction(ui->actionOpen);
	m_toolBar->addAction(ui->actionSave);
	m_toolBar->addAction(ui->actionPrint);

	m_toolBar->addSeparator();

	// Lane Count
	//
	QLabel* lanesLabel = new QLabel("Lanes:");
	lanesLabel->setAlignment(Qt::AlignCenter);
	m_toolBar->addWidget(lanesLabel);

	m_lanesCombo = new QComboBox(m_toolBar);
	m_lanesCombo->addItem(tr("1"), QVariant::fromValue(1));
	m_lanesCombo->addItem(tr("2"), QVariant::fromValue(2));
	m_lanesCombo->addItem(tr("3"), QVariant::fromValue(3));
	m_lanesCombo->addItem(tr("4"), QVariant::fromValue(4));
	m_lanesCombo->addItem(tr("5"), QVariant::fromValue(5));
	m_lanesCombo->addItem(tr("6"), QVariant::fromValue(6));
	m_lanesCombo->setCurrentText(QString::number(theSettings.m_laneCount));
	m_toolBar->addWidget(m_lanesCombo);


	// View Type
	//
	QLabel* viewLabel = new QLabel("View:");
	viewLabel->setAlignment(Qt::AlignCenter);
	m_toolBar->addWidget(viewLabel);

	m_viewCombo = new QComboBox(m_toolBar);
	m_viewCombo->addItem(tr("Separated"), QVariant::fromValue(TrendLib::TrendView::Separated));
	m_viewCombo->addItem(tr("Overlapped"), QVariant::fromValue(TrendLib::TrendView::Overlapped));
	m_toolBar->addWidget(m_viewCombo);

	this->addToolBar(Qt::TopToolBarArea, m_toolBar);

	return;
}

void TrendsMainWindow::saveWindowState()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theSettings.m_viewType = m_viewCombo->currentIndex();
	theSettings.m_laneCount = m_lanesCombo->currentIndex() + 1;

	theSettings.writeUserScope();
	return;
}

void TrendsMainWindow::restoreWindowState()
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

	assert(m_viewCombo);
	m_viewCombo->setCurrentIndex(theSettings.m_viewType);

	return;
}

void TrendsMainWindow::closeEvent(QCloseEvent* e)
{
	saveWindowState();
	e->accept();
	return;
}

void TrendsMainWindow::timerEvent(QTimerEvent*)
{

}

void TrendsMainWindow::showEvent(QShowEvent*)
{

}

void TrendsMainWindow::actionOpenTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionSaveTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionPrintTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionExitTriggered()
{
	close();
}

void TrendsMainWindow::actionAboutTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionLaneCountTriggered()
{

}

void TrendsMainWindow::viewComboCurrentIndexChanged(int index)
{
	qDebug() << Q_FUNC_INFO << " index = " << index;

	TrendLib::TrendView view = m_viewCombo->itemData(index).value<TrendLib::TrendView>();
	m_trendWidget->setView(view);

	m_trendWidget->updateWidget();
}

void TrendsMainWindow::laneCountComboCurrentIndexChanged(int index)
{
	qDebug() << Q_FUNC_INFO << " index = " << index;

	int laneCount = m_lanesCombo->itemData(index).value<int>();
	m_trendWidget->setLaneCount(laneCount);

	m_trendWidget->updateWidget();
}

