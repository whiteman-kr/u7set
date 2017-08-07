#include "TrendMainWindow.h"
#include "ui_TrendsMainWindow.h"
#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QFileDialog>
#include <QPageSize>
#include <QPageLayout>
#include <QComboBox>
#include "TrendSettings.h"
#include "TrendWidget.h"
#include "TrendSignal.h"
#include "DialogTrendSignalProperties.h"
#include "../Proto/serialization.pb.h"

namespace TrendLib
{

	TrendMainWindow::TrendMainWindow(QWidget* parent) :
		QMainWindow(parent),
		ui(new Ui::TrendsMainWindow)
	{
		ui->setupUi(this);

		setAttribute(Qt::WA_DeleteOnClose);
		setAcceptDrops(true);

		// --
		//
		QWidget* centarlWidget = new QWidget;
		setCentralWidget(centarlWidget);

		QGridLayout* layout = new QGridLayout();
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		centarlWidget->setLayout(layout);

		// read settings
		//
		theSettings.load();

		// TrendWidget
		//
		m_trendWidget = new TrendLib::TrendWidget(this);
		layout->addWidget(m_trendWidget, 0, 0);

		m_trendWidget->setView(static_cast<TrendLib::TrendView>(theSettings.m_viewType));
		m_trendWidget->setTimeType(static_cast<TimeType>(theSettings.m_timeType));
		m_trendWidget->setLaneCount(theSettings.m_laneCount);

		// Slider Widged
		//
		m_trendSlider = new TrendSlider(&m_trendWidget->rullerSet());

		layout->setRowStretch(0, 1);
		layout->addWidget(m_trendSlider, 1, 0);

		//--
		//
		connect(ui->actionOpen, &QAction::triggered, this, &TrendMainWindow::actionOpenTriggered);
		connect(ui->actionSave, &QAction::triggered, this, &TrendMainWindow::actionSaveTriggered);
		connect(ui->actionPrint, &QAction::triggered, this, &TrendMainWindow::actionPrintTriggered);
		connect(ui->actionExit, &QAction::triggered, this, &TrendMainWindow::actionExitTriggered);
		connect(ui->actionAbout, &QAction::triggered, this, &TrendMainWindow::actionAboutTriggered);

		createToolBar();

		connect(m_timeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::timeComboCurrentIndexChanged);
		connect(m_viewCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::viewComboCurrentIndexChanged);
		connect(m_lanesCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::laneCountComboCurrentIndexChanged);
		connect(m_timeTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::timeTypeComboCurrentIndexChanged);
		connect(m_signalsButton, &QPushButton::clicked, this, &TrendMainWindow::signalsButton);

		setMinimumSize(500, 300);
		restoreWindowState();

		// Init Slider with some params from ToolBar, connect signals before setting min/max/current time
		//
		connect(m_trendSlider, &TrendSlider::valueChanged, this, &TrendMainWindow::sliderValueChanged);
		connect(m_trendWidget, &TrendWidget::startTimeChanged, this, &TrendMainWindow::startTimeChanged);

		// On click on signal description
		//
		connect(m_trendWidget, &TrendWidget::showSignalProperties, this, &TrendMainWindow::signalProperties);

		// --
		//
		TimeStamp ts(QDateTime::currentDateTime());
		ts.timeStamp -= 1_hour * theSettings.m_laneCount;

		m_trendSlider->setMin(ts);
		m_trendSlider->setValue(ts);

		TimeStamp max(m_trendSlider->min().timeStamp + theSettings.m_laneCount * 1_hour);
		m_trendSlider->setMax(max);

		qint64 t = m_timeCombo->currentData().value<qint64>();
		m_trendSlider->setSingleStep(t / singleStepSliderDivider);
		m_trendSlider->setPageStep(t);

		m_trendSlider->setLaneDuration(t * theSettings.m_laneCount);

		// Refresh Action
		//
		m_refreshAction = new QAction(tr("Refresh"), this);
		m_refreshAction->setShortcut(QKeySequence::Refresh);
		connect(m_refreshAction, &QAction::triggered, this, &TrendMainWindow::actionRefreshTriggered);
		addAction(m_refreshAction);

		// Contect Menu
		//
		setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(this, &QWidget::customContextMenuRequested, this, &TrendMainWindow::contextMenuRequested);

		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		//	COLORREF StdColors[] = {	RGB(0x80, 0x00, 0x00), RGB(0x00, 0x80, 0x00), RGB(0x00, 0x00, 0x80), RGB(0x00, 0x80, 0x80), RGB(0x80, 0x00, 0x80),
		//								RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0xFF), RGB(0x00, 0x00, 0x00) };


//		TrendLib::TrendSignalParam s1;
//		s1.setSignalId("ASIGNAL001");
//		s1.setAppSignalId("#ASIGNAL001");
//		s1.setCaption("ATrend Signal 001");
//		s1.setType(E::SignalType::Analog);
//		s1.setLowLimit(10.0);
//		s1.setHighLimit(105.0);
//		s1.setColor(qRgb(0x80, 0x00, 0x00));

//		TrendLib::TrendSignalParam s11;
//		s11.setSignalId("ASIGNAL011");
//		s11.setAppSignalId("#ASIGNAL011");
//		s11.setCaption("ATren Signal 011");
//		s11.setType(E::SignalType::Analog);
//		s11.setLowLimit(400.0);
//		s11.setHighLimit(25000.0);
//		s11.setColor(qRgb(0x80, 0x00, 0x80));

//		TrendLib::TrendSignalParam s2;
//		s2.setSignalId("SIGNAL002");
//		s2.setAppSignalId("#SIGNAL002");
//		s2.setCaption("Tren Signal 002");
//		s2.setType(E::SignalType::Discrete);
//		s2.setColor(qRgb(0x00, 0x80, 0x00));

//		TrendLib::TrendSignalParam s3;
//		s3.setSignalId("SIGNAL003");
//		s3.setAppSignalId("#SIGNAL003");
//		s3.setCaption("Tren Signal 003");
//		s3.setType(E::SignalType::Discrete);
//		s3.setColor(qRgb(0x00, 0x00, 0x80));

//		TrendLib::TrendSignalParam s4;
//		s4.setSignalId("SIGNAL004");
//		s4.setAppSignalId("#SIGNAL004");
//		s4.setCaption("Tren Signal 004");
//		s4.setType(E::SignalType::Discrete);
//		s4.setColor(qRgb(0x00, 0x80, 0x80));

//		TrendLib::TrendSignalParam s6;
//		s6.setSignalId("SIGNAL006");
//		s6.setCaption("Tren Signal 006");
//		s6.setType(E::SignalType::Discrete);
//		s6.setColor(qRgb(0x00, 0x80, 0x80));

//		TrendLib::TrendSignalParam s7;
//		s7.setSignalId("SIGNAL007");
//		s7.setCaption("Tren Signal 007");
//		s7.setType(E::SignalType::Discrete);
//		s7.setColor(qRgb(0x00, 0x80, 0x80));

//		signalSet().addSignal(s1);
//		signalSet().addSignal(s2);
//		signalSet().addSignal(s3);
//		signalSet().addSignal(s4);
////		signalSet().addSignal(s6);
////		signalSet().addSignal(s7);
//		signalSet().addSignal(s11);
		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		return;
	}

	TrendMainWindow::~TrendMainWindow()
	{
		delete ui;
	}

	void TrendMainWindow::ensureVisible()
	{
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
	}

	bool TrendMainWindow::addSignals(const std::vector<TrendLib::TrendSignalParam>& trendSignals, bool redraw)
	{
		bool ok = true;
		for (const TrendLib::TrendSignalParam& tsp : trendSignals)
		{
			ok &= addSignal(tsp, false);
		}

		if (redraw == true)
		{
			updateWidget();
		}

		return ok;
	}

	bool TrendMainWindow::addSignal(const TrendLib::TrendSignalParam& trendSignal, bool redraw)
	{
		std::vector<TrendLib::TrendSignalParam> discreteSignals = signalSet().discreteSignals();
		std::vector<TrendLib::TrendSignalParam> analogSignals = signalSet().analogSignals();

		if (discreteSignals.size() + analogSignals.size() > 12)
		{
			return false;
		}

		auto dit = std::find_if(discreteSignals.begin(), discreteSignals.end(),
						[&trendSignal](const TrendLib::TrendSignalParam& t)
						{
							return t.appSignalId() == trendSignal.appSignalId();
						});

		auto ait = std::find_if(analogSignals.begin(), analogSignals.end(),
						[&trendSignal](const TrendLib::TrendSignalParam& t)
						{
							return t.appSignalId() == trendSignal.appSignalId();
						});

		if (dit != discreteSignals.end() ||
			ait != analogSignals.end())
		{
			return false;
		}

static const QRgb StdColors[] = { qRgb(0x80, 0x00, 0x00), qRgb(0x00, 0x80, 0x00), qRgb(0x00, 0x00, 0x80), qRgb(0x00, 0x80, 0x80),
								  qRgb(0x80, 0x00, 0x80), qRgb(0xFF, 0x00, 0x00), qRgb(0x00, 0x00, 0xFF), qRgb(0x00, 0x00, 0x00) };
static int stdColorIndex = 0;

		TrendLib::TrendSignalParam tsp(trendSignal);

		tsp.setColor(StdColors[stdColorIndex]);
		signalSet().addSignal(tsp);

		// --
		//
		stdColorIndex ++;
		if (stdColorIndex >= sizeof(StdColors) / sizeof(StdColors[0]))
		{
			stdColorIndex = 0;
		}

		if (redraw == true)
		{
			updateWidget();
		}

		return true;
	}

	void TrendMainWindow::createToolBar()
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
		QLabel* intervalLabel = new QLabel("  Interval: ");
		intervalLabel->setAlignment(Qt::AlignCenter);
		m_toolBar->addWidget(intervalLabel);

		m_timeCombo = new QComboBox(m_toolBar);
		m_timeCombo->addItem(tr("2 sec"), QVariant::fromValue(2_sec));
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
		m_timeCombo->addItem(tr("7 days"), QVariant::fromValue(24_hours * 7));
		m_timeCombo->setCurrentIndex(6);
		m_toolBar->addWidget(m_timeCombo);

		// Lane Count
		//
		QLabel* lanesLabel = new QLabel("  Lanes: ");
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
		QLabel* viewLabel = new QLabel("  View: ");
		viewLabel->setAlignment(Qt::AlignCenter);
		m_toolBar->addWidget(viewLabel);

		m_viewCombo = new QComboBox(m_toolBar);
		m_viewCombo->addItem(tr("Separated"), QVariant::fromValue(TrendLib::TrendView::Separated));
		m_viewCombo->addItem(tr("Overlapped"), QVariant::fromValue(TrendLib::TrendView::Overlapped));
		m_toolBar->addWidget(m_viewCombo);

		this->addToolBar(Qt::TopToolBarArea, m_toolBar);

		// Tyme Type
		//
		QLabel* timeTypeLabel = new QLabel("  Time Type: ");
		timeTypeLabel->setAlignment(Qt::AlignCenter);
		m_toolBar->addWidget(timeTypeLabel);

		m_timeTypeCombo = new QComboBox(m_toolBar);
		m_timeTypeCombo->addItem(tr("Server Time"), QVariant::fromValue(TimeType::Local));
		m_timeTypeCombo->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(TimeType::System));
		m_timeTypeCombo->addItem(tr("Plant Time"), QVariant::fromValue(TimeType::Plant));
		m_toolBar->addWidget(m_timeTypeCombo);

		this->addToolBar(Qt::TopToolBarArea, m_toolBar);

		// Add stretecher
		//
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		m_toolBar->addWidget(empty);

		// Signals
		//
		m_signalsButton = new QPushButton("Signals...");
		m_toolBar->addWidget(m_signalsButton);

		return;
	}

	QStatusBar* TrendMainWindow::statusBar()
	{
		return ui->statusBar;
	}

	void TrendMainWindow::saveWindowState()
	{
		theSettings.m_mainWindowPos = pos();
		theSettings.m_mainWindowGeometry = saveGeometry();
		theSettings.m_mainWindowState = saveState();

		theSettings.m_viewType = m_viewCombo->currentIndex();
		theSettings.m_laneCount = m_lanesCombo->currentIndex() + 1;
		theSettings.m_timeTypeIndex = m_timeTypeCombo->currentIndex();

		theSettings.writeUserScope();
		return;
	}

	void TrendMainWindow::restoreWindowState()
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);

		assert(m_viewCombo);
		m_viewCombo->setCurrentIndex(theSettings.m_viewType);

		m_timeTypeCombo->setCurrentIndex(theSettings.m_timeTypeIndex);

		// Ensure widget is visible
		//
		ensureVisible();

		return;
	}

	void TrendMainWindow::closeEvent(QCloseEvent* e)
	{
		saveWindowState();
		e->accept();
		return;
	}

	void TrendMainWindow::timerEvent(QTimerEvent*)
	{
	}

	void TrendMainWindow::showEvent(QShowEvent*)
	{
	}

	void TrendMainWindow::dragEnterEvent(QDragEnterEvent* event)
	{
		if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
		{
			event->acceptProposedAction();
		}

		return;
	}

	void TrendMainWindow::dropEvent(QDropEvent* event)
	{
		if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
		{
			assert(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
			event->setDropAction(Qt::DropAction::IgnoreAction);
			event->accept();
			return;
		}

		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

		::Proto::AppSignalParamSet protoSetMessage;
		bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

		if (ok == false)
		{
			event->acceptProposedAction();
			return;
		}

		// Parse data
		//
		for (int i = 0; i < protoSetMessage.items_size(); i++)
		{
			const ::Proto::AppSignalParam& appSignalMessage = protoSetMessage.items(i);

			AppSignalParam appSignalParam;
			ok = appSignalParam.load(appSignalMessage);

			if (ok == true)
			{
				TrendSignalParam tsp(appSignalParam);
				addSignal(tsp, false);
			}
		}

		updateWidget();

		return;
	}

	void TrendMainWindow::signalsButton()
	{
		// Override in derived class to set signals
		//
	}

	void TrendMainWindow::updateWidget()
	{
		if (m_trendWidget == nullptr)
		{
			assert(m_trendWidget);
			return;
		}

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::signalProperties(QString appSignalId)
	{
		qDebug() << "Show signal " << appSignalId << " properties";

		bool ok = false;
		TrendLib::TrendSignalParam signal = signalSet().signalParam(appSignalId, &ok);

		if (ok == false)
		{
			assert(ok);		// Signal must be in signal set
			return;
		}

		DialogTrendSignalProperties d(signal, this);

		bool result = d.exec();
		if (result == QDialog::Accepted)
		{
			ok = signalSet().setSignalParam(d.trendSignal());
			assert(ok);
			updateWidget();
		}

		return;
	}

	void TrendMainWindow::actionOpenTriggered()
	{
		// todo
		//
		//assert(false);
	}

	void TrendMainWindow::actionSaveTriggered()
	{
		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save File"),
														"untitled.u7trend",
														tr("Trend (*.u7trend);;Images (*.png *.bmp *.jpg);;PDF files (*.pdf)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}

		QFileInfo fileInfo(fileName);
		QString extension = fileInfo.completeSuffix();

		if (extension.compare(QLatin1String("u7trend"), Qt::CaseInsensitive) == 0)
		{
			return;
		}

		if (extension.compare(QLatin1String("png"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("bmp"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("jpg"), Qt::CaseInsensitive) == 0 ||
			extension.compare(QLatin1String("jpeg"), Qt::CaseInsensitive) == 0)
		{
			bool ok = m_trendWidget->saveImageToFile(fileName);
			if (ok == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Writing file error. File %1").arg(fileName));
			}

			return;
		}

		if (extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
		{
			// Select paper size
			//
static QPageSize::PageSizeId m_defaultPageSize = QPageSize::A3;
static QPageLayout::Orientation m_defaultPageOrientation = QPageLayout::Orientation::Landscape;

			QDialog d(this);

			d.setWindowTitle(tr("Page Setup"));
			d.setWindowFlags((d.windowFlags() &
							~Qt::WindowMinimizeButtonHint &
							~Qt::WindowMaximizeButtonHint &
							~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

			QLabel* pageSizeLabel = new  QLabel(tr("Page Size"));

			QComboBox* pageSizeCombo = new QComboBox;
			pageSizeCombo->addItem(QLatin1String("A0"),  QVariant::fromValue(QPageSize::A0));
			pageSizeCombo->addItem(QLatin1String("A1"),  QVariant::fromValue(QPageSize::A1));
			pageSizeCombo->addItem(QLatin1String("A2"),  QVariant::fromValue(QPageSize::A2));
			pageSizeCombo->addItem(QLatin1String("A3"),  QVariant::fromValue(QPageSize::A3));
			pageSizeCombo->addItem(QLatin1String("A4"),  QVariant::fromValue(QPageSize::A4));
			pageSizeCombo->addItem(QLatin1String("A5"),  QVariant::fromValue(QPageSize::A5));
			pageSizeCombo->addItem(QLatin1String("Letter"),  QVariant::fromValue(QPageSize::Letter));
			pageSizeCombo->addItem(QLatin1String("AnsiA"),  QVariant::fromValue(QPageSize::AnsiA));
			pageSizeCombo->addItem(QLatin1String("AnsiB"),  QVariant::fromValue(QPageSize::AnsiB));
			pageSizeCombo->addItem(QLatin1String("AnsiC"),  QVariant::fromValue(QPageSize::AnsiC));
			pageSizeCombo->addItem(QLatin1String("AnsiD"),  QVariant::fromValue(QPageSize::AnsiD));
			pageSizeCombo->addItem(QLatin1String("AnsiE"),  QVariant::fromValue(QPageSize::AnsiE));
			int psi = pageSizeCombo->findData(QVariant::fromValue(m_defaultPageSize));
			if (psi != -1)
			{
				pageSizeCombo->setCurrentIndex(psi);
			}

			QLabel* orientationLabel = new  QLabel(tr("Orientation"));

			QComboBox* orientationCombo = new QComboBox;
			orientationCombo->addItem(tr("Portrait"),  QVariant::fromValue(QPageLayout::Portrait));
			orientationCombo->addItem(tr("Lanscape"),  QVariant::fromValue(QPageLayout::Landscape));
			int poi = orientationCombo->findData(QVariant::fromValue(m_defaultPageOrientation));
			if (poi != -1)
			{
				orientationCombo->setCurrentIndex(poi);
			}

			QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
			connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

			QGridLayout* layout = new QGridLayout;
			layout->addWidget(pageSizeLabel, 0, 0);
			layout->addWidget(pageSizeCombo, 0, 1);
			layout->addWidget(orientationLabel, 1, 0);
			layout->addWidget(orientationCombo, 1, 1);
			layout->addWidget(buttonBox, 2, 0, 1, 2);
			d.setLayout(layout);

			int result = d.exec();
			if (result == QDialog::Accepted)
			{
				m_defaultPageSize = pageSizeCombo->currentData().value<QPageSize::PageSizeId>();
				m_defaultPageOrientation = orientationCombo->currentData().value<QPageLayout::Orientation>();

				m_trendWidget->saveToPdf(fileName, m_defaultPageSize, m_defaultPageOrientation);
			}

			return;
		}

		QMessageBox::critical(this, qAppName(), tr("Unsupported file format."));

		return;
	}

	void TrendMainWindow::actionPrintTriggered()
	{
		// todo
		//
		//assert(false);
	}

	void TrendMainWindow::actionExitTriggered()
	{
		close();
	}

	void TrendMainWindow::actionAboutTriggered()
	{
		QMessageBox msgBox(this);

		msgBox.setWindowTitle(tr("About Trends"));
		QPixmap image(":/TrendImages/Images/RadiyLogo.png");
		msgBox.setIconPixmap(image);

		QStringList args = qApp->arguments();
		args.pop_front();

		QStringList text;
		text << tr("<b>JSC Radiy</b><br>");
		text << tr("<b>Trends</b><br>");
#ifdef _DEBUG
		text << tr("Version: %1 (Debug)<br>").arg(qApp->applicationVersion());
#else
		text << tr("Version: %1 (Release)<br>").arg(qApp->applicationVersion());
#endif
		text << tr("Build architecture: %1<br>").arg(QSysInfo::buildCpuArchitecture());

		text << tr("<br>");
		text << tr("ProcessID: %1<br>").arg(qApp->applicationPid());
		text << tr("Executable: %1<br>").arg(qApp->applicationFilePath());
		text << tr("Arguments: %1<br>").arg(args.join(' '));

		msgBox.setText(text.join('\n'));

		msgBox.exec();

		return;
	}

	void TrendMainWindow::actionRefreshTriggered()
	{
		qDebug() << "Refresh trend data (clear)";
		signalSet().clear(m_trendWidget->timeType());

		updateWidget();

		return;
	}

	void TrendMainWindow::actionAddRuller(QPoint mousePos)
	{
		int laneIndex = -1;
		int rullerIndex = -1;
		TimeStamp timeStamp;
		QString outSignalId;

		Trend::MouseOn mouseOn = m_trendWidget->mouseIsOver(mousePos, &laneIndex, &timeStamp, &rullerIndex, &outSignalId);

		if (mouseOn != Trend::MouseOn::InsideTrendArea)
		{
			return;
		}

		qDebug() << "Add trend ruller on pos " << timeStamp.toDateTime();

		TrendRuller ruller(timeStamp);
		trend().rullerSet().addRuller(ruller);

		update();

		return;
	}

	void TrendMainWindow::actionDeleteRuller(int rullerIndex)
	{
		if (rullerIndex == -1)
		{
			assert(rullerIndex);
			return;
		}

		if (rullerIndex < 0 ||
			rullerIndex >= static_cast<int>(trend().rullerSet().rullers().size()))
		{
			assert(false);
			return;
		}

		trend().rullerSet().deleteRuller(trend().rullerSet().at(rullerIndex).timeStamp());
		m_trendWidget->resetRullerHighlight();

		update();

		return;
	}

	void TrendMainWindow::actionRullerProperties(int rullerIndex)
	{
		if (rullerIndex == -1)
		{
			assert(rullerIndex);
			return;
		}

		if (rullerIndex < 0 ||
			rullerIndex >= static_cast<int>(trend().rullerSet().rullers().size()))
		{
			assert(false);
			return;
		}

		// --
		//
		TrendRuller& mutableRuller = trend().rullerSet().at(rullerIndex);

		QDialog d(this);
		d.setWindowTitle(tr("Ruller Properties"));
		d.setWindowFlags((d.windowFlags() &
						~Qt::WindowMinimizeButtonHint &
						~Qt::WindowMaximizeButtonHint &
						~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

		QGridLayout* layout = new QGridLayout(&d);

		QLabel* dateLabel = new QLabel(tr("Date:"));
		QLabel* timeLabel = new QLabel(tr("Time:"));

		QDateEdit* dateEdit = new QDateEdit(mutableRuller.timeStamp().toDate());
		dateEdit->setCalendarPopup(true);

		QTimeEdit* timeEdit = new QTimeEdit(mutableRuller.timeStamp().toTime());
		timeEdit->setDisplayFormat("hh:mm:ss.zzz");

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
		connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

		// --
		//
		layout->addWidget(dateLabel, 0, 0);
		layout->addWidget(dateEdit, 0, 1);

		layout->addWidget(timeLabel, 1, 0);
		layout->addWidget(timeEdit, 1, 1);
		layout->addWidget(buttonBox, 2, 0, 1, 2);

		d.setLayout(layout);

		// --
		//
		int dialogResult = d.exec();

		if (dialogResult ==  QDialog::Accepted)
		{
			QDateTime newDateTime;
			newDateTime.setDate(dateEdit->date());
			newDateTime.setTime(timeEdit->time());

			TimeStamp ts(newDateTime);
			mutableRuller.setTimeStamp(ts);

			update();
		}

		return;
	}

	void TrendMainWindow::timeComboCurrentIndexChanged(int /*index*/)
	{
		qint64 t = m_timeCombo->currentData().value<qint64>();

		m_trendSlider->setSingleStep(t / singleStepSliderDivider);
		m_trendSlider->setPageStep(t);
		m_trendSlider->setLaneDuration(t * m_trendWidget->laneCount());

		m_trendWidget->setDuration(t);
		m_trendWidget->updateWidget();

		return;
	}

	void TrendMainWindow::viewComboCurrentIndexChanged(int index)
	{
		TrendLib::TrendView view = m_viewCombo->itemData(index).value<TrendLib::TrendView>();
		m_trendWidget->setView(view);

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::laneCountComboCurrentIndexChanged(int index)
	{
		int laneCount = m_lanesCombo->itemData(index).value<int>();
		m_trendWidget->setLaneCount(laneCount);

		qint64 t = m_timeCombo->currentData().value<qint64>();
		m_trendSlider->setLaneDuration(t * m_trendWidget->laneCount());

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::timeTypeComboCurrentIndexChanged(int index)
	{
		TimeType timeType = m_timeTypeCombo->itemData(index).value<TimeType>();
		m_trendWidget->setTimeType(timeType);
		theSettings.m_timeType = static_cast<int>(timeType);

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::sliderValueChanged(TimeStamp value)
	{
		m_trendWidget->setStartTime(value);
		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::startTimeChanged(TimeStamp value)
	{
		m_trendSlider->setValueShiftMinMax(value);
	}

	void TrendMainWindow::contextMenuRequested(const QPoint& /*pos*/)
	{
		int outLaneIndex = -1;
		int rullerIndex = -1;
		TimeStamp timeStamp;
		QPoint pos = m_trendWidget->mapFromGlobal(QCursor::pos());
		QString outSignalId;

		Trend::MouseOn mouseOn = m_trendWidget->mouseIsOver(pos, &outLaneIndex, &timeStamp, &rullerIndex, &outSignalId);

		if (mouseOn != Trend::MouseOn::InsideTrendArea &&
			mouseOn != Trend::MouseOn::OnRuller)
		{
			return;
		}

		QMenu menu(this);

		QAction* addRullerAction = menu.addAction(tr("Add Ruller"));
		connect(addRullerAction, &QAction::triggered, this,
				[&pos, this]()
				{
					this->TrendMainWindow::actionAddRuller(pos);
				});

		QAction* deleteRullerAction = menu.addAction(tr("Delete Ruller"));
		deleteRullerAction->setEnabled(mouseOn == Trend::MouseOn::OnRuller);
		connect(deleteRullerAction, &QAction::triggered, this,
				[rullerIndex, this]()
				{
					this->TrendMainWindow::actionDeleteRuller(rullerIndex);
				});

		QAction* rullerPropertiesAction = menu.addAction(tr("Ruller Properties..."));
		rullerPropertiesAction->setEnabled(mouseOn == Trend::MouseOn::OnRuller);
		connect(rullerPropertiesAction, &QAction::triggered, this,
				[rullerIndex, this]()
				{
					this->TrendMainWindow::actionRullerProperties(rullerIndex);
				});

		menu.addSeparator();
		QAction* chooseView = menu.addAction(tr("Choose View..."));
		chooseView->setEnabled(false);		// Not implemented yet

		assert(m_refreshAction);
		menu.addAction(m_refreshAction->text(), this, &TrendMainWindow::actionRefreshTriggered, QKeySequence::Refresh);

		menu.addSeparator();

		std::vector<TrendLib::TrendSignalParam> discrets = signalSet().discreteSignals();
		std::vector<TrendLib::TrendSignalParam> analogs = signalSet().analogSignals();

		QMenu* signalPropsMenu = menu.addMenu(tr("Signals Properties"));
		signalPropsMenu->setEnabled(discrets.size() + analogs.size() > 0);

		for (const TrendLib::TrendSignalParam& s : discrets)
		{
			QAction* signalPropertiesAction = signalPropsMenu->addAction(s.signalId() + " - " + s.caption());
			connect(signalPropertiesAction, &QAction::triggered, this,
					[this, s]()
					{
						signalProperties(s.appSignalId());
					});
		}

		for (const TrendLib::TrendSignalParam& s : analogs)
		{
			QAction* signalPropertiesAction = signalPropsMenu->addAction(s.signalId() + " - " + s.caption());
			connect(signalPropertiesAction, &QAction::triggered, this,
					[this, s]()
					{
						signalProperties(s.appSignalId());
					});
		}

		QAction* signalAction = menu.addAction(tr("Signals..."));
		connect(signalAction, &QAction::triggered, this, &TrendMainWindow::signalsButton);

		menu.exec(QCursor::pos());

		return;
	}

	TrendLib::TrendSignalSet& TrendMainWindow::signalSet()
	{
		return m_trendWidget->signalSet();
	}

	const TrendLib::TrendSignalSet& TrendMainWindow::signalSet() const
	{
		return m_trendWidget->signalSet();
	}

	TrendLib::Trend& TrendMainWindow::trend()
	{
		return m_trendWidget->trend();
	}

	const TrendLib::Trend& TrendMainWindow::trend() const
	{
		return m_trendWidget->trend();
	}

}
