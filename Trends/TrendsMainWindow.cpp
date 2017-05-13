#include "TrendsMainWindow.h"
#include "ui_TrendsMainWindow.h"
#include <QGridLayout>
#include "Settings.h"
#include "TrendWidget.h"

TrendsMainWindow::TrendsMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TrendsMainWindow)
{
	ui->setupUi(this);

	QWidget* centarlWidget = new QWidget;
	setCentralWidget(centarlWidget);

	QGridLayout* layout = new QGridLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	centarlWidget->setLayout(layout);

	// TrendWidget
	//
	m_trendWidget = new TrendLib::TrendWidget(&m_signalSet);
	layout->addWidget(m_trendWidget, 0, 0);

	m_trendWidget->setView(static_cast<TrendLib::TrendView>(theSettings.m_viewType));
	m_trendWidget->setLaneCount(theSettings.m_laneCount);

	// Slider Widged
	//
	m_trendSlider = new TrendSlider;

	layout->setRowStretch(0, 1);
	layout->addWidget(m_trendSlider, 1, 0);

	//--
	//
	connect(ui->actionOpen, &QAction::triggered, this, &TrendsMainWindow::actionOpenTriggered);
	connect(ui->actionSave, &QAction::triggered, this, &TrendsMainWindow::actionSaveTriggered);
	connect(ui->actionPrint, &QAction::triggered, this, &TrendsMainWindow::actionPrintTriggered);
	connect(ui->actionExit, &QAction::triggered, this, &TrendsMainWindow::actionExitTriggered);
	connect(ui->actionAbout, &QAction::triggered, this, &TrendsMainWindow::actionAboutTriggered);

	createToolBar();

	connect(m_timeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendsMainWindow::timeComboCurrentIndexChanged);
	connect(m_viewCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendsMainWindow::viewComboCurrentIndexChanged);
	connect(m_lanesCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendsMainWindow::laneCountComboCurrentIndexChanged);

	setMinimumSize(500, 300);
	restoreWindowState();

	// Init Slider with some params from ToolBar
	//
	TimeStamp ts(QDateTime::currentMSecsSinceEpoch());
	ts.timeStamp -= 1_hour * theSettings.m_laneCount;

	m_trendSlider->setMin(ts);
	m_trendSlider->setValue(ts);

	TimeStamp max(m_trendSlider->min().timeStamp + theSettings.m_laneCount * 1_hour);
	m_trendSlider->setMax(max);

	qint64 t = m_timeCombo->currentData().value<qint64>();
	m_trendSlider->setSingleStep(t / singleStepSliderDivider);
	m_trendSlider->setPageStep(t);

	connect(m_trendSlider, &TrendSlider::valueChanged, this, &TrendsMainWindow::sliderValueChanged);


	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//	COLORREF StdColors[] = {	RGB(0x80, 0x00, 0x00), RGB(0x00, 0x80, 0x00), RGB(0x00, 0x00, 0x80), RGB(0x00, 0x80, 0x80), RGB(0x80, 0x00, 0x80),
//								RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0xFF), RGB(0x00, 0x00, 0x00) };


	TrendLib::TrendSignal s1;
	s1.setSignalId("ASIGNAL001");
	s1.setCaption("ATrend Signal 001");
	s1.setType(E::SignalType::Analog);
	s1.setLowLimit(10.0);
	s1.setHighLimit(105.0);
	s1.setColor(qRgb(0x80, 0x00, 0x00));

	TrendLib::TrendSignal s11;
	s11.setSignalId("ASIGNAL011");
	s11.setCaption("ATren Signal 011");
	s11.setType(E::SignalType::Analog);
	s11.setLowLimit(400.0);
	s11.setHighLimit(25000.0);
	s11.setColor(qRgb(0x80, 0x00, 0x80));

	TrendLib::TrendSignal s2;
	s2.setSignalId("SIGNAL002");
	s2.setCaption("Tren Signal 002");
	s2.setType(E::SignalType::Discrete);
	s2.setColor(qRgb(0x00, 0x80, 0x00));

	TrendLib::TrendSignal s3;
	s3.setSignalId("SIGNAL003");
	s3.setCaption("Tren Signal 003");
	s3.setType(E::SignalType::Discrete);
	s3.setColor(qRgb(0x00, 0x00, 0x80));

	TrendLib::TrendSignal s4;
	s4.setSignalId("SIGNAL004");
	s4.setCaption("Tren Signal 004");
	s4.setType(E::SignalType::Discrete);
	s4.setColor(qRgb(0x00, 0x80, 0x80));

	m_signalSet.addSignal(s1);
	m_signalSet.addSignal(s2);
	m_signalSet.addSignal(s3);
	m_signalSet.addSignal(s4);
	m_signalSet.addSignal(s11);
	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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

	// Time ComboBox
	//
	QLabel* intervalLabel = new QLabel("Interval:");
	intervalLabel->setAlignment(Qt::AlignCenter);
	m_toolBar->addWidget(intervalLabel);

	m_timeCombo = new QComboBox(m_toolBar);
	m_timeCombo->addItem(tr("5 sec"), QVariant::fromValue(5_sec));
	m_timeCombo->addItem(tr("10 sec"), QVariant::fromValue(10_sec));
	m_timeCombo->addItem(tr("30 sec"), QVariant::fromValue(30_sec));
	m_timeCombo->addItem(tr("1 min"), QVariant::fromValue(1_min));
	m_timeCombo->addItem(tr("5 min"), QVariant::fromValue(5_min));
	m_timeCombo->addItem(tr("30 min"), QVariant::fromValue(30_min));
	m_timeCombo->addItem(tr("1 hour"), QVariant::fromValue(1_hour));
	m_timeCombo->addItem(tr("3 hour"), QVariant::fromValue(3_hours));
	m_timeCombo->addItem(tr("6 hour"), QVariant::fromValue(6_hours));
	m_timeCombo->addItem(tr("12 hour"), QVariant::fromValue(12_hours));
	m_timeCombo->addItem(tr("24 hour"), QVariant::fromValue(24_hours));
	m_timeCombo->setCurrentIndex(6);
	m_toolBar->addWidget(m_timeCombo);

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
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("About Trends"));

	QStringList args = qApp->arguments();
	args.pop_front();

	QStringList text;
	text << tr("JSC Radiy");
	text << tr("Trends");
	text << tr("Version: %1").arg(qApp->applicationVersion());
	text << tr("");
	text << tr("ProcessID: %1").arg(qApp->applicationPid());
	text << tr("Executable: %1").arg(qApp->applicationFilePath());
	text << tr("Arguments: %1").arg(args.join(' '));

	msgBox.setText(text.join('\n'));

	QPixmap image(":/Images/Images/RadiyLogo.png");
	msgBox.setIconPixmap(image);

	msgBox.exec();




//	mb.setParent(this);
//	mb.setText(tr("About"));

//	mb.exec();

	return;
}


void TrendsMainWindow::timeComboCurrentIndexChanged(int /*index*/)
{
	qint64 t = m_timeCombo->currentData().value<qint64>();

	m_trendSlider->setSingleStep(t / singleStepSliderDivider);
	m_trendSlider->setPageStep(t);

	m_trendWidget->setDuration(t);
	m_trendWidget->updateWidget();

	return;
}

void TrendsMainWindow::viewComboCurrentIndexChanged(int index)
{
	TrendLib::TrendView view = m_viewCombo->itemData(index).value<TrendLib::TrendView>();
	m_trendWidget->setView(view);

	m_trendWidget->updateWidget();
	return;
}

void TrendsMainWindow::laneCountComboCurrentIndexChanged(int index)
{
	int laneCount = m_lanesCombo->itemData(index).value<int>();
	m_trendWidget->setLaneCount(laneCount);

	m_trendWidget->updateWidget();
	return;
}

void TrendsMainWindow::sliderValueChanged(TimeStamp value)
{
	m_trendWidget->setStartTime(value);
	m_trendWidget->updateWidget();
	return;
}
