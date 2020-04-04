#include "TrendMainWindow.h"
#include "ui_TrendsMainWindow.h"
#include "TrendSettings.h"
#include "TrendWidget.h"
#include "TrendSignal.h"
#include "TrendScale.h"
#include "DialogTrendSignalProperties.h"
#include "../Proto/serialization.pb.h"
#include "../lib/Types.h"

namespace TrendLib
{

	TrendMainWindow::TrendMainWindow(QWidget* parent) :
		QMainWindow(parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
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

		m_trendWidget->setViewMode(static_cast<TrendLib::TrendViewMode>(theSettings.m_viewType));
		m_trendWidget->setTimeType(static_cast<E::TimeType>(theSettings.m_timeType));
		m_trendWidget->setLaneCount(theSettings.m_laneCount);

		// Slider Widged
		//
		m_trendSlider = new TrendSlider(&m_trendWidget->rulerSet());

		layout->setRowStretch(0, 1);
		layout->addWidget(m_trendSlider, 1, 0);

		// Refresh Action
		//
		m_refreshAction = new QAction(tr("Refresh"), this);
		m_refreshAction->setShortcut(QKeySequence::Refresh);
		connect(m_refreshAction, &QAction::triggered, this, &TrendMainWindow::actionRefreshTriggered);
		addAction(m_refreshAction);

		//--
		//
		connect(ui->actionOpen, &QAction::triggered, this, &TrendMainWindow::actionOpenTriggered);
		connect(ui->actionSave, &QAction::triggered, this, &TrendMainWindow::actionSaveTriggered);
		connect(ui->actionPrint, &QAction::triggered, this, &TrendMainWindow::actionPrintTriggered);
		connect(ui->actionExit, &QAction::triggered, this, &TrendMainWindow::actionExitTriggered);
		connect(ui->actionAbout, &QAction::triggered, this, &TrendMainWindow::actionAboutTriggered);
		connect(ui->actionAutoScale, &QAction::triggered, this, &TrendMainWindow::actionAutoSclaeTriggered);

		createToolBar();

		connect(m_timeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::timeComboCurrentIndexChanged);
		connect(m_viewCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::viewComboCurrentIndexChanged);
		connect(m_scaleTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::scaleTypeComboCurrentIndexChanged);
		connect(m_lanesCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::laneCountComboCurrentIndexChanged);
		connect(m_timeTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TrendMainWindow::timeTypeComboCurrentIndexChanged);
		connect(m_realtimeModeButton, &QPushButton::toggled, this, &TrendMainWindow::realtimeModeToggled);
		connect(m_realtimeAutoShiftButton, &QPushButton::toggled, this, &TrendMainWindow::realtimeAutoShiftClicked);

		connect(m_refreshButton, &QPushButton::clicked, m_refreshAction, &QAction::triggered);
		connect(m_signalsButton, &QPushButton::clicked, this, &TrendMainWindow::signalsButton);

		setMinimumSize(500, 300);
		restoreWindowState();

		// Init Slider with some params from ToolBar, connect signals before setting min/max/current time
		//
		connect(m_trendSlider, &TrendSlider::valueChanged, this, &TrendMainWindow::sliderValueChanged);
		connect(m_trendWidget, &TrendWidget::startTimeChanged, this, &TrendMainWindow::startTimeChanged);
		connect(m_trendWidget, &TrendWidget::durationChanged, this, &TrendMainWindow::durationChanged);

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

		// Contect Menu
		//
		setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(this, &QWidget::customContextMenuRequested, this, &TrendMainWindow::contextMenuRequested);

		return;
	}

	TrendMainWindow::~TrendMainWindow()
	{
		delete ui;
	}

	void TrendMainWindow::ensureVisible()
	{
		setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

		if (isMinimized() == true)
		{
			showNormal();
		}

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
			resize(static_cast<int>(screenRect.width() * 0.7),
				   static_cast<int>(screenRect.height() * 0.7));
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

		if (discreteSignals.size() + analogSignals.size() > 16)
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
		m_timeCombo->setDuplicatesEnabled(false);

		m_timeCombo->addItem(tr("2 sec"), QVariant::fromValue(2_sec));
		m_timeCombo->addItem(tr("5 sec"), QVariant::fromValue(5_sec));
		m_timeCombo->addItem(tr("10 sec"), QVariant::fromValue(10_sec));
		m_timeCombo->addItem(tr("30 sec"), QVariant::fromValue(30_sec));
		m_timeCombo->addItem(tr("1 min"), QVariant::fromValue(1_min));
		m_timeCombo->addItem(tr("5 min"), QVariant::fromValue(5_min));
		m_timeCombo->addItem(tr("10 min"), QVariant::fromValue(10_min));
		m_timeCombo->addItem(tr("30 min"), QVariant::fromValue(30_min));
		m_timeCombo->addItem(tr("1 hour"), QVariant::fromValue(1_hour));
		m_timeCombo->addItem(tr("3 hour"), QVariant::fromValue(3_hours));
		m_timeCombo->addItem(tr("6 hour"), QVariant::fromValue(6_hours));
		m_timeCombo->addItem(tr("12 hour"), QVariant::fromValue(12_hours));
		m_timeCombo->addItem(tr("24 hour"), QVariant::fromValue(24_hours));

		int currentDuartionIndex = 8;

		m_timeCombo->setCurrentIndex(currentDuartionIndex);
		m_trendWidget->setLaneDuration(m_timeCombo->itemData(currentDuartionIndex).toLongLong());

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
		m_viewCombo->addItem(tr("Separated"), QVariant::fromValue(TrendLib::TrendViewMode::Separated));
		m_viewCombo->addItem(tr("Overlapped"), QVariant::fromValue(TrendLib::TrendViewMode::Overlapped));
		m_toolBar->addWidget(m_viewCombo);

		this->addToolBar(Qt::TopToolBarArea, m_toolBar);

		// Scale Type
		//
		QLabel* scaleTypeLabel = new QLabel("  Scale: ");
		scaleTypeLabel->setAlignment(Qt::AlignCenter);
		m_toolBar->addWidget(scaleTypeLabel);

		m_scaleTypeCombo = new QComboBox(m_toolBar);
		m_scaleTypeCombo->addItem(tr("Generic"), QVariant::fromValue(TrendLib::TrendScaleType::Generic));
		m_scaleTypeCombo->addItem(tr("Logarithmic"), QVariant::fromValue(TrendLib::TrendScaleType::Logarithmic));
		m_scaleTypeCombo->addItem(tr("Period"), QVariant::fromValue(TrendLib::TrendScaleType::Period));
		m_toolBar->addWidget(m_scaleTypeCombo);

		this->addToolBar(Qt::TopToolBarArea, m_toolBar);

		// Time Type
		//
		QLabel* timeTypeLabel = new QLabel("  Time Type: ");
		timeTypeLabel->setAlignment(Qt::AlignCenter);
		m_toolBar->addWidget(timeTypeLabel);

		m_timeTypeCombo = new QComboBox(m_toolBar);
		m_timeTypeCombo->addItem(tr("Server Time"), QVariant::fromValue(E::TimeType::Local));
		m_timeTypeCombo->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(E::TimeType::System));
		m_timeTypeCombo->addItem(tr("Plant Time"), QVariant::fromValue(E::TimeType::Plant));
		m_toolBar->addWidget(m_timeTypeCombo);

		// 	AutoScale
		//
		m_toolBar->addSeparator();
		m_toolBar->addAction(ui->actionAutoScale);

		m_toolBar->addSeparator();

		// TrendMode
		//
		m_realtimeModeButton = new QPushButton(tr("Realtime"));
		m_realtimeModeButton->setCheckable(true);
		m_toolBar->addWidget(m_realtimeModeButton);

		m_realtimeAutoShiftButton = new QPushButton(tr(">>>"));
		m_realtimeAutoShiftButton->setEnabled(false);
		m_realtimeAutoShiftButton->setCheckable(true);
		//m_realtimeAutoShiftButton->(QSizePolicy::Fixed, QSizePolicy::Minimum);
		m_toolBar->addWidget(m_realtimeAutoShiftButton);

		m_toolBar->addSeparator();

		// Add stretecher
		//
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		m_toolBar->addWidget(empty);

		// Signals
		//
		m_refreshButton = new QPushButton("Refresh");
		m_toolBar->addWidget(m_refreshButton);

		// Signals
		//
		m_signalsButton = new QPushButton("Signals...");
		m_toolBar->addWidget(m_signalsButton);

		// --
		//
		this->addToolBar(Qt::TopToolBarArea, m_toolBar);

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
		theSettings.m_scaleType = m_scaleTypeCombo->currentIndex();
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

		Q_ASSERT(m_viewCombo);
		m_viewCombo->setCurrentIndex(theSettings.m_viewType);

		Q_ASSERT(m_scaleTypeCombo);
		m_scaleTypeCombo->setCurrentIndex(theSettings.m_scaleType);

		m_timeTypeCombo->setCurrentIndex(theSettings.m_timeTypeIndex);

		// Ensure widget is visible
		//
		ensureVisible();

		return;
	}

	void TrendMainWindow::setRealtimeAutoShift(const TimeStamp& ts)
	{
		Q_ASSERT(trendMode() == E::TrendMode::Realtime);
		Q_ASSERT(isRealtimeAutoShift() == true);

		m_lastRealtimeMaxValue = ts;

		if (ts < m_trendWidget->startTime())
		{
			m_trendWidget->setStartTime(ts);
			return;
		}

		if (ts > m_trendWidget->finishTime())
		{
			m_trendWidget->setStartTime(ts.timeStamp - m_trendWidget->duration() * m_trendWidget->laneCount());
			return;
		}

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
			Q_ASSERT(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
			event->setDropAction(Qt::DropAction::IgnoreAction);
			event->accept();
			return;
		}

		QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

		::Proto::AppSignalSet protoSetMessage;
		bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

		if (ok == false)
		{
			event->acceptProposedAction();
			return;
		}

		// Parse data
		//
		for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
		{
			const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

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
			Q_ASSERT(m_trendWidget);
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
			Q_ASSERT(ok);		// Signal must be in signal set
			return;
		}

		DialogTrendSignalProperties d(signal,
									  &signalSet(),
									  m_trendWidget->timeType(),
									  m_trendWidget->scaleType(),
									  m_trendWidget->trendMode(),
									  this);
#pragma warning( push )

#pragma warning (disable: 6326)
		connect(&d, &DialogTrendSignalProperties::signalPropertiesChanged, this,
				[&d, this]()
				{
					bool ok = signalSet().setSignalParam(d.trendSignal());
					Q_ASSERT(ok);
					this->updateWidget();
				});
#pragma warning( pop )

		d.exec();

		return;
	}

	void TrendMainWindow::actionOpenTriggered()
	{
		QString fileName = QFileDialog::getOpenFileName(this,
														tr("Open Trend File"),
														".",
														tr("Trend (*.u7trend);;All Files (*.*)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}

		Q_ASSERT(m_trendWidget);

		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QApplication::processEvents();

		QString errorMessage;
		bool ok = m_trendWidget->load(fileName, &errorMessage);

		QApplication::restoreOverrideCursor();
		QApplication::processEvents();

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Loading trend error: %1\n").arg(errorMessage));
			return;
		}

		// Set UI items to loaded state
		// start time, interval, +lanes, +view mode, time type
		//
		m_trendSlider->setValue(m_trendWidget->startTime());

		int index = m_lanesCombo->findData(QVariant::fromValue(m_trendWidget->laneCount()));
		if (index != -1)
		{
			m_lanesCombo->setCurrentIndex(index);
		}

		index = m_viewCombo->findData(QVariant::fromValue(m_trendWidget->viewMode()));
		if (index != -1)
		{
			m_viewCombo->setCurrentIndex(index);
		}

		index = m_scaleTypeCombo->findData(QVariant::fromValue(m_trendWidget->scaleType()));
		if (index != -1)
		{
			m_scaleTypeCombo->setCurrentIndex(index);
		}

		index = m_timeTypeCombo->findData(QVariant::fromValue(m_trendWidget->timeType()));
		if (index != -1)
		{
			m_timeTypeCombo->setCurrentIndex(index);
		}

		index = m_timeCombo->findData(QVariant::fromValue(m_trendWidget->duration()));
		if (index != -1)
		{
			m_timeCombo->setCurrentIndex(index);
		}

		QApplication::processEvents();

		updateWidget();

		return;
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
			Q_ASSERT(m_trendWidget);

			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			QApplication::processEvents();

			QTime timer;
			timer.start();

			QString errorMessage;
			bool ok = m_trendWidget->save(fileName, &errorMessage);

			qDebug() << "Save trend to file: " << fileName << ", result: " << ok << ", elapsed time: " << timer.elapsed();

			QApplication::restoreOverrideCursor();
			QApplication::processEvents();

			if (ok == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Saving trend error: %1\n").arg(errorMessage));
				return;
			}

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
static QPageLayout lastPageLayout;
static int lastResolution = -1;
static bool lastFullPage = false;
static int lastCopyCount = false;

		QPrintDialog d(this);

		d.setOption(QAbstractPrintDialog::PrintToFile, true);
		d.setOption(QAbstractPrintDialog::PrintSelection, false);
		d.setOption(QAbstractPrintDialog::PrintPageRange, false);
		d.setOption(QAbstractPrintDialog::PrintShowPageSize, false);
		d.setOption(QAbstractPrintDialog::PrintCollateCopies, false);
		d.setOption(QAbstractPrintDialog::PrintCurrentPage, false);

		if (lastPageLayout.isValid() == true)
		{
			d.printer()->setPageLayout(lastPageLayout);
			d.printer()->setResolution(lastResolution);
			d.printer()->setFullPage(lastFullPage);
			d.printer()->setCopyCount(lastCopyCount);
		}

		int result = d.exec();

		if (result == QDialog::Accepted)
		{
			QPrinter* printer = d.printer();

			lastPageLayout = printer->pageLayout();
			lastResolution = printer->resolution();
			lastFullPage = printer->fullPage();
			lastCopyCount = printer->copyCount();

			// Print
			//
			m_trendWidget->print(printer);
		}

		return;
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

	void TrendMainWindow::actionAutoSclaeTriggered()
	{
		qDebug() << "Autoscale trend";

		std::vector<TrendLib::TrendSignalParam> analogs = signalSet().analogSignals();

		QDateTime startTime = m_trendWidget->startTime().toDateTime();
		QDateTime finishTime = TimeStamp(m_trendWidget->startTime().timeStamp + m_trendWidget->duration()).toDateTime();

		qint64 startTimeValue = m_trendWidget->startTime().timeStamp;
		qint64 finishTimeValue = m_trendWidget->finishTime().timeStamp;

		E::TimeType timeType = m_trendWidget->timeType();

		for (TrendLib::TrendSignalParam& ts : analogs)
		{
			std::list<std::shared_ptr<OneHourData>> signalData;
			signalSet().getExistingTrendData(ts.appSignalId(), startTime, finishTime, timeType, &signalData);

			double minValue = 0;
			double maxValue = 0;
			bool firstValue = true;

			for (std::shared_ptr<OneHourData> hour : signalData)
			{
				const std::vector<TrendStateRecord>& data = hour->data;
				for (const TrendStateRecord& record : data)
				{
					for (const TrendStateItem& state : record.states)
					{
						if (state.isValid() == false)
						{
							continue;
						}

						TimeStamp ct = state.getTime(timeType);

						if (ct.timeStamp < startTimeValue)
						{
							continue;
						}

						if (ct.timeStamp > finishTimeValue)
						{
							break;
						}

						bool ok = false;

						double value = TrendScale::valueToScaleValue(state.value, m_trendWidget->scaleType(), &ok);

						if (ok == false)
						{
							continue;
						}

						if (firstValue == true)
						{
							minValue = value;
							maxValue = value;

							firstValue = false;
							continue;
						}

						minValue = qMin(minValue, value);
						maxValue = qMax(maxValue, value);
					}	// for (const TrendStateItem& state : record.states)
				}
			}

			if (firstValue == false)
			{
				if (fabs(maxValue - minValue) <= DBL_MIN)
				{
					minValue = minValue - 1.0;
					maxValue = maxValue + 1.0;
				}
				else
				{
					minValue = minValue - (maxValue - minValue) * 0.10;
					maxValue = maxValue + (maxValue - minValue) * 0.10;
				}

				bool ok = false;

				double newMinValue = TrendScale::limitFromScaleValue(minValue, m_trendWidget->scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				double newMaxValue = TrendScale::limitFromScaleValue(maxValue, m_trendWidget->scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				// Set limits and update param
				//
				ts.setViewLowLimit(newMinValue);
				ts.setViewHighLimit(newMaxValue);

				signalSet().setSignalParam(ts);
			}
		}

		updateWidget();
		return;
	}

	void TrendMainWindow::actionAddRuler(QPoint mousePos)
	{
		int laneIndex = -1;
		int rulerIndex = -1;
		TimeStamp timeStamp;
		TrendSignalParam outSignal;

		Trend::MouseOn mouseOn = m_trendWidget->mouseIsOver(mousePos, &laneIndex, &timeStamp, &rulerIndex, &outSignal);

		if (mouseOn != Trend::MouseOn::InsideTrendArea)
		{
			return;
		}

		qDebug() << "Add trend ruler on pos " << timeStamp.toDateTime();

		TrendRuler ruler(timeStamp);
		trend().rulerSet().addRuler(ruler);

		update();

		return;
	}

	void TrendMainWindow::actionDeleteRuler(int rulerIndex)
	{
		if (rulerIndex == -1)
		{
			Q_ASSERT(rulerIndex);
			return;
		}

		if (rulerIndex < 0 ||
			rulerIndex >= static_cast<int>(trend().rulerSet().rulers().size()))
		{
			Q_ASSERT(false);
			return;
		}

		trend().rulerSet().deleteRuler(trend().rulerSet().at(rulerIndex).timeStamp());
		m_trendWidget->resetRulerHighlight();

		update();

		return;
	}

	void TrendMainWindow::actionRulerProperties(int rulerIndex)
	{
		if (rulerIndex == -1)
		{
			Q_ASSERT(rulerIndex);
			return;
		}

		if (rulerIndex < 0 ||
			rulerIndex >= static_cast<int>(trend().rulerSet().rulers().size()))
		{
			Q_ASSERT(false);
			return;
		}

		// --
		//
		TrendRuler& mutableRuler = trend().rulerSet().at(rulerIndex);

		QDialog d(this);
		d.setWindowTitle(tr("Ruler Properties"));
		d.setWindowFlags((d.windowFlags() &
						~Qt::WindowMinimizeButtonHint &
						~Qt::WindowMaximizeButtonHint &
						~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

		QGridLayout* layout = new QGridLayout(&d);

		QLabel* dateLabel = new QLabel(tr("Date:"));
		QLabel* timeLabel = new QLabel(tr("Time:"));

		QDateEdit* dateEdit = new QDateEdit(mutableRuler.timeStamp().toDate());
		dateEdit->setCalendarPopup(true);

		QTimeEdit* timeEdit = new QTimeEdit(mutableRuler.timeStamp().toTime());
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
			mutableRuler.setTimeStamp(ts);

			update();
		}

		return;
	}

	void TrendMainWindow::timeComboCurrentIndexChanged(int /*index*/)
	{
		QVariant v = m_timeCombo->currentData();

		if (v.isValid() && v.type() == QVariant::LongLong)
		{
			qint64 t = v.value<qint64>();

			m_trendSlider->setSingleStep(t / singleStepSliderDivider);
			m_trendSlider->setPageStep(t);
			m_trendSlider->setLaneDuration(t * m_trendWidget->laneCount());

			m_trendWidget->setLaneDuration(t);
			m_trendWidget->updateWidget();
		}

		int customIndex = m_timeCombo->findText(tr("Custom"));
		if (customIndex != -1)
		{
			m_timeCombo->removeItem(customIndex);
		}

		return;
	}

	void TrendMainWindow::viewComboCurrentIndexChanged(int index)
	{
		TrendLib::TrendViewMode view = m_viewCombo->itemData(index).value<TrendLib::TrendViewMode>();
		m_trendWidget->setViewMode(view);

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::scaleTypeComboCurrentIndexChanged(int index)
	{
		TrendLib::TrendScaleType scale = m_scaleTypeCombo->itemData(index).value<TrendLib::TrendScaleType>();

		m_trendWidget->setScaleType(scale);

		m_trendWidget->adjustViewLimitsForScaleType();

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
		E::TimeType timeType = m_timeTypeCombo->itemData(index).value<E::TimeType>();
		m_trendWidget->setTimeType(timeType);
		theSettings.m_timeType = static_cast<int>(timeType);

		m_trendWidget->updateWidget();
		return;
	}

	void TrendMainWindow::realtimeModeToggled(bool state)
	{
		E::TrendMode tm = state ? E::TrendMode::Realtime : E::TrendMode::Archive;
		m_trendWidget->setTrendMode(tm);

		signalSet().addNonValidPoint();

		m_realtimeAutoShiftButton->setEnabled(tm == E::TrendMode::Realtime);
		m_realtimeAutoShiftButton->setChecked(true);

		return;
	}

	void TrendMainWindow::realtimeAutoShiftClicked(bool /*state*/)
	{
		//E::TrendMode tm = m_trendWidget->trendMode();
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

		if (trendMode() == E::TrendMode::Realtime &&
			isRealtimeAutoShift() == true)
		{
			if (m_lastRealtimeMaxValue > m_trendWidget->finishTime() ||
				m_lastRealtimeMaxValue < m_trendWidget->startTime())
			{
				// Exit realtime autoshift
				//
				m_realtimeAutoShiftButton->setChecked(false);
			}
		}

		return;
	}

	void TrendMainWindow::durationChanged(qint64 value)
	{
		m_trendSlider->setLaneDuration(value);

		m_trendSlider->setSingleStep(value / singleStepSliderDivider);
		m_trendSlider->setPageStep(value);

		m_timeCombo->blockSignals(true);		// Block changes, as tr("Custom") is deleting there

		if (m_timeCombo->findText(tr("Custom")) == -1)
		{
			m_timeCombo->addItem(tr("Custom"));		// Duplicates are disabled
		}
		m_timeCombo->setCurrentText(tr("Custom"));

		m_timeCombo->blockSignals(false);

		return;
	}

	void TrendMainWindow::contextMenuRequested(const QPoint& /*pos*/)
	{
		int analogsCount = signalSet().analogSignalsCount();
		int discretesCount = signalSet().discretesSignalsCount();

		int outLaneIndex = -1;
		int rulerIndex = -1;
		TimeStamp timeStamp;
		QPoint pos = m_trendWidget->mapFromGlobal(QCursor::pos());
		TrendSignalParam outSignal;

		Trend::MouseOn mouseOn = m_trendWidget->mouseIsOver(pos, &outLaneIndex, &timeStamp, &rulerIndex, &outSignal);

		if (mouseOn != Trend::MouseOn::InsideTrendArea &&
			mouseOn != Trend::MouseOn::OnRuler)
		{
			return;
		}

		QMenu menu(this);

		QAction* addRulerAction = menu.addAction(tr("Add Ruler"));
		connect(addRulerAction, &QAction::triggered, this,
				[&pos, this]()
				{
					this->TrendMainWindow::actionAddRuler(pos);
				});

		QAction* deleteRulerAction = menu.addAction(tr("Delete Ruler"));
		deleteRulerAction->setEnabled(mouseOn == Trend::MouseOn::OnRuler);
		connect(deleteRulerAction, &QAction::triggered, this,
				[rulerIndex, this]()
				{
					this->TrendMainWindow::actionDeleteRuler(rulerIndex);
				});

		QAction* rulerPropertiesAction = menu.addAction(tr("Ruler Properties..."));
		rulerPropertiesAction->setEnabled(mouseOn == Trend::MouseOn::OnRuler);
		connect(rulerPropertiesAction, &QAction::triggered, this,
				[rulerIndex, this]()
				{
					this->TrendMainWindow::actionRulerProperties(rulerIndex);
				});

		menu.addSeparator();
		QAction* chooseView = menu.addAction(tr("Select View..."));
		chooseView->setEnabled(analogsCount + discretesCount > 0);
		connect(chooseView, &QAction::triggered, m_trendWidget, &TrendLib::TrendWidget::startSelectionViewArea);

		Q_ASSERT(m_refreshAction);
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

	E::TrendMode TrendMainWindow::trendMode() const
	{
		return m_trendWidget->trendMode();
	}

	void TrendMainWindow::setTrendMode(E::TrendMode value)
	{
		m_trendWidget->setTrendMode(value);
	}

	bool TrendMainWindow::isRealtimeAutoShift() const
	{
		Q_ASSERT(trendMode() == E::TrendMode::Realtime);
		Q_ASSERT(m_realtimeAutoShiftButton);

		return m_realtimeAutoShiftButton->isChecked();
	}

}
