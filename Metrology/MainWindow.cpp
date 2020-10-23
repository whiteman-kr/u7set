#include "MainWindow.h"

#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QCloseEvent>

#include "Options.h"
#include "OptionsDialog.h"
#include "CalibratorBase.h"
#include "Database.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"
#include "ExportData.h"
#include "RackList.h"
#include "SignalList.h"
#include "ComparatorList.h"
#include "TuningSignalList.h"
#include "SignalConnectionList.h"
#include "ObjectProperties.h"

#include "../lib/Ui/DialogAbout.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget *parent)
	: QMainWindow(parent)
	, m_softwareInfo(softwareInfo)
{
	// open database
	//
	theDatabase.open();

	// init calibration base
	//
	theCalibratorBase.init(this);
	connect(&theCalibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::calibratorConnectedChanged, Qt::QueuedConnection);
	connect(&theCalibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);

	// load signal base
	//
	theSignalBase.racks().groups().load();		// load rack groups for multichannel measuring
	theSignalBase.signalConnections().load();	// load signal connections base
	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(&theSignalBase.tuning().signalBase(), &TuningSignalBase::signalsCreated, this, &MainWindow::tuningSignalsCreated, Qt::QueuedConnection);

	// load measurement base
	//
	for (int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType++)
	{
		theMeasureBase.load(measureType);
	}

	//
	//
	createInterface();		// init interface

	runMeasureThread();		// init measure thread
	runConfigSocket();		// init config socket thread
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
	setWindowTitle(tr("Metrology"));
	move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());

	createActions();
	createMenu();
	createToolBars();
	createPanels();
	createMeasureViews();
	createStatusBar();
	createContextMenu();

	loadSettings();

	setMeasureType(MEASURE_TYPE_LINEARITY);

	m_pCalculator = new Calculator(this);	// calculator

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createActions()
{
	// Measure
	//
	m_pStartMeasureAction = new QAction(tr("Start"), this);
	m_pStartMeasureAction->setShortcut(Qt::Key_F5);
	m_pStartMeasureAction->setIcon(QIcon(":/icons/Start.png"));
	m_pStartMeasureAction->setToolTip(tr("To start the measurement process"));
	connect(m_pStartMeasureAction, &QAction::triggered, this, &MainWindow::startMeasure);

	m_pStopMeasureAction = new QAction(tr("Stop"), this);
	m_pStopMeasureAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
	m_pStopMeasureAction->setIcon(QIcon(":/icons/Stop.png"));
	m_pStopMeasureAction->setToolTip(tr("To stop the measurement process"));
	connect(m_pStopMeasureAction, &QAction::triggered, this, &MainWindow::stopMeasure);

	m_pExportMeasureAction = new QAction(tr("&Export ..."), this);
	m_pExportMeasureAction->setShortcut(Qt::CTRL + Qt::Key_E);
	m_pExportMeasureAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportMeasureAction->setToolTip(tr("Export measurements"));
	connect(m_pExportMeasureAction, &QAction::triggered, this, &MainWindow::exportMeasure);

	m_pPreviousSignalAction = new QAction(tr("Previous signal"), this);
	m_pPreviousSignalAction->setShortcut(Qt::CTRL + Qt::Key_Left);
	m_pPreviousSignalAction->setIcon(QIcon(":/icons/PreviousSignal.png"));
	m_pPreviousSignalAction->setToolTip(tr("Select previous signal"));
	connect(m_pPreviousSignalAction, &QAction::triggered, this, &MainWindow::previousMeasureSignal);

	m_pNextSignalAction = new QAction(tr("Next signal"), this);
	m_pNextSignalAction->setShortcut(Qt::CTRL + Qt::Key_Right);
	m_pNextSignalAction->setIcon(QIcon(":/icons/NextSignal.png"));
	m_pNextSignalAction->setToolTip(tr("Select next signal"));
	connect(m_pNextSignalAction, &QAction::triggered, this, &MainWindow::nextMeasureSignal);

	// Edit
	//
	m_pCopyMeasureAction = new QAction(tr("&Copy"), this);
	m_pCopyMeasureAction->setShortcut(Qt::CTRL + Qt::Key_C);
	m_pCopyMeasureAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyMeasureAction->setToolTip(tr("Copy of the measurements"));
	connect(m_pCopyMeasureAction, &QAction::triggered, this, &MainWindow::copyMeasure);

	m_pRemoveMeasureAction = new QAction(tr("&Delete"), this);
	m_pRemoveMeasureAction->setShortcut(Qt::CTRL + Qt::Key_Delete);
	m_pRemoveMeasureAction->setIcon(QIcon(":/icons/Remove.png"));
	m_pRemoveMeasureAction->setToolTip(tr("Delete the selected measurements"));
	connect(m_pRemoveMeasureAction, &QAction::triggered, this, &MainWindow::removeMeasure);

	m_pSelectAllMeasureAction = new QAction(tr("Select &All"), this);
	m_pSelectAllMeasureAction->setShortcut(Qt::CTRL + Qt::Key_A);
	m_pSelectAllMeasureAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllMeasureAction->setToolTip(tr("Select all measurements"));
	connect(m_pSelectAllMeasureAction, &QAction::triggered, this, &MainWindow::selectAllMeasure);

	// View
	//
	m_pShowRackListAction = new QAction(tr("Racks ..."), this);
	m_pShowRackListAction->setIcon(QIcon(":/icons/Rack.png"));
	m_pShowRackListAction->setToolTip("");
	connect(m_pShowRackListAction, &QAction::triggered, this, &MainWindow::showRackList);

	m_pShowSignalListAction = new QAction(tr("&Signals ..."), this);
	m_pShowSignalListAction->setIcon(QIcon(":/icons/Signal.png"));
	m_pShowSignalListAction->setToolTip("");
	connect(m_pShowSignalListAction, &QAction::triggered, this, &MainWindow::showSignalList);

	m_pShowComparatorsListAction = new QAction(tr("&Comparators ..."), this);
	m_pShowComparatorsListAction->setIcon(QIcon(":/icons/Comparator.png"));
	m_pShowComparatorsListAction->setToolTip("");
	connect(m_pShowComparatorsListAction, &QAction::triggered, this, &MainWindow::showComparatorsList);

	m_pShowTuningSignalListAction = new QAction(tr("Tuning signals ..."), this);
	m_pShowTuningSignalListAction->setIcon(QIcon(":/icons/Tuning.png"));
	m_pShowTuningSignalListAction->setToolTip("");
	connect(m_pShowTuningSignalListAction, &QAction::triggered, this, &MainWindow::showTuningSignalList);

	m_pShowSignalConnectionListAction = new QAction(tr("Signal connections ..."), this);
	m_pShowSignalConnectionListAction->setIcon(QIcon(":/icons/Connection.png"));
	m_pShowSignalConnectionListAction->setToolTip("");
	connect(m_pShowSignalConnectionListAction, &QAction::triggered, this, &MainWindow::showSignalConnectionList);

	m_pShowStatisticAction = new QAction(tr("Sta&tistics (Checklist) ..."), this);
	m_pShowStatisticAction->setIcon(QIcon(":/icons/Statistics.png"));
	m_pShowStatisticAction->setToolTip("");
	connect(m_pShowStatisticAction, &QAction::triggered, this, &MainWindow::showStatistic);

	// Tools
	//
	m_pCalibratorsAction = new QAction(tr("&Calibrations ..."), this);
	m_pCalibratorsAction->setIcon(QIcon(":/icons/Calibrators.png"));
	m_pCalibratorsAction->setToolTip(tr("Connecting and configuring calibrators"));
	connect(m_pCalibratorsAction, &QAction::triggered, this, &MainWindow::showCalibrators);

	m_pShowCalculatorAction = new QAction(tr("Metrological &calculator ..."), this);
	m_pShowCalculatorAction->setShortcut(Qt::ALT + Qt::Key_C);
	m_pShowCalculatorAction->setIcon(QIcon(":/icons/Calculator.png"));
	m_pShowCalculatorAction->setToolTip(tr("Calculator for converting metrological quantities"));
	connect(m_pShowCalculatorAction, &QAction::triggered, this, &MainWindow::showCalculator);

	m_pOptionsAction = new QAction(tr("&Options ..."), this);
	m_pOptionsAction->setShortcut(Qt::CTRL + Qt::Key_O);
	m_pOptionsAction->setIcon(QIcon(":/icons/Options.png"));
	m_pOptionsAction->setToolTip(tr("Editing application settings"));
	connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::showOptions);

	// ?
	//
	m_pAboutConnectionAction = new QAction(tr("About connect to server ..."), this);
	m_pAboutConnectionAction->setIcon(QIcon(":/icons/About Сonnection.png"));
	m_pAboutConnectionAction->setToolTip("");
	connect(m_pAboutConnectionAction, &QAction::triggered, this, &MainWindow::aboutConnection);

	m_pAboutAppAction = new QAction(tr("About Metrology ..."), this);
	m_pAboutAppAction->setIcon(QIcon(":/icons/About Application.png"));
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

	// Measure
	//
	m_pMeasureMenu = pMenuBar->addMenu(tr("&Measure"));

	m_pMeasureMenu->addAction(m_pStartMeasureAction);
	m_pMeasureMenu->addAction(m_pStopMeasureAction);
	m_pMeasureMenu->addSeparator();
	m_pMeasureMenu->addAction(m_pExportMeasureAction);

	// Edit
	//
	m_pEditMenu = pMenuBar->addMenu(tr("&Edit"));

	m_pEditMenu->addAction(m_pCopyMeasureAction);
	m_pEditMenu->addAction(m_pRemoveMeasureAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pSelectAllMeasureAction);
	m_pEditMenu->addSeparator();

	// View
	//
	m_pViewMenu = pMenuBar->addMenu(tr("&View"));
	m_pViewPanelMenu = new QMenu(tr("&Panels"), m_pViewMenu);

	m_pViewMenu->addMenu(m_pViewPanelMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addAction(m_pShowRackListAction);
	m_pViewMenu->addAction(m_pShowSignalListAction);
	m_pViewMenu->addAction(m_pShowComparatorsListAction);
	m_pViewMenu->addAction(m_pShowTuningSignalListAction);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addAction(m_pShowSignalConnectionListAction);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addAction(m_pShowStatisticAction);

	// Tools
	//
	m_pToolsMenu = pMenuBar->addMenu(tr("&Tools"));

	m_pToolsMenu->addAction(m_pCalibratorsAction);
	m_pToolsMenu->addSeparator();
	m_pToolsMenu->addAction(m_pShowCalculatorAction);
	m_pToolsMenu->addSeparator();
	m_pToolsMenu->addAction(m_pOptionsAction);

	// ?
	//
	m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

	m_pInfoMenu->addAction(m_pAboutConnectionAction);
	m_pInfoMenu->addAction(m_pAboutAppAction);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createToolBars()
{
	// Control panel measure process
	//
	m_pMeasureControlToolBar = new QToolBar(this);
	if (m_pMeasureControlToolBar != nullptr)
	{
		m_pMeasureControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pMeasureControlToolBar->setWindowTitle(tr("Control panel measure process"));
		m_pMeasureControlToolBar->setObjectName(m_pMeasureControlToolBar->windowTitle());
		addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(m_pMeasureControlToolBar);

		m_pMeasureControlToolBar->addAction(m_pStartMeasureAction);
		m_pMeasureControlToolBar->addAction(m_pStopMeasureAction);
		m_pMeasureControlToolBar->addSeparator();
		m_pMeasureControlToolBar->addAction(m_pCopyMeasureAction);
		m_pMeasureControlToolBar->addAction(m_pRemoveMeasureAction);
		m_pMeasureControlToolBar->addSeparator();
	}

	// Control panel measure timeout
	//
	m_pMeasureTimeout = new QToolBar(this);
	if (m_pMeasureTimeout != nullptr)
	{
		m_pMeasureTimeout->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pMeasureTimeout->setWindowTitle(tr("Control panel measure timeout"));
		m_pMeasureTimeout->setObjectName(m_pMeasureTimeout->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pMeasureTimeout);

		QLabel* measureTimeoutLabel = new QLabel(m_pMeasureTimeout);
		QComboBox* measureTimeoutList = new QComboBox(m_pMeasureTimeout);
		QLabel* measureTimeoutUnitLabel = new QLabel(m_pMeasureTimeout);
		QRegExp rx("^[0-9]*[.]{1}[0-9]*$");
		QValidator *validator = new QRegExpValidator(rx, m_pMeasureTimeout);

		m_pMeasureTimeout->addWidget(measureTimeoutLabel);
		m_pMeasureTimeout->addWidget(measureTimeoutList);
		m_pMeasureTimeout->addWidget(measureTimeoutUnitLabel);

		measureTimeoutLabel->setText(tr(" Measure timeout "));
		measureTimeoutLabel->setEnabled(false);

		measureTimeoutList->setEditable(true);
		measureTimeoutList->setValidator(validator);

		for(int t = 0; t < MeasureTimeoutCount; t++)
		{
			measureTimeoutList->addItem(QString::number(MeasureTimeout[t], 'f', 1));
		}

		measureTimeoutList->setCurrentText(QString::number(double(theOptions.toolBar().measureTimeout()) / 1000, 'f', 1));

		measureTimeoutUnitLabel->setText(tr(" sec."));
		measureTimeoutUnitLabel->setEnabled(false);

		connect(measureTimeoutList, &QComboBox::currentTextChanged, this, &MainWindow::setMeasureTimeout);
	}


	// Control panel measure kind
	//
	m_pMeasureKind = new QToolBar(this);
	if (m_pMeasureKind != nullptr)
	{
		m_pMeasureKind->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pMeasureKind->setWindowTitle(tr("Control panel measure kind"));
		m_pMeasureKind->setObjectName(m_pMeasureKind->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pMeasureKind);

		QLabel* measureKindLabel = new QLabel(m_pMeasureKind);
		m_measureKindList = new QComboBox(m_pMeasureKind);

		m_pMeasureKind->addWidget(measureKindLabel);
		m_pMeasureKind->addWidget(m_measureKindList);

		measureKindLabel->setText(tr(" Measure kind "));
		measureKindLabel->setEnabled(false);

		for(int k = 0; k < MEASURE_KIND_COUNT; k++)
		{
			m_measureKindList->addItem(qApp->translate("MeasureBase.h", MeasureKind[k]));
		}

		m_measureKindList->setCurrentIndex(theOptions.toolBar().measureKind());

		connect(m_measureKindList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setMeasureKind);
	}


	// Control panel signal connections
	//
	m_pSignalConnectionToolBar = new QToolBar(this);
	if (m_pSignalConnectionToolBar != nullptr)
	{
		m_pSignalConnectionToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pSignalConnectionToolBar->setWindowTitle(tr("Control panel signal connections"));
		m_pSignalConnectionToolBar->setObjectName(m_pSignalConnectionToolBar->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pSignalConnectionToolBar);

		QLabel* signalConnectionLabel = new QLabel(m_pSignalConnectionToolBar);
		m_signalConnectionTypeList = new QComboBox(m_pSignalConnectionToolBar);

		m_pSignalConnectionToolBar->addWidget(signalConnectionLabel);
		signalConnectionLabel->setText(tr(" Signal connections "));
		signalConnectionLabel->setEnabled(false);

		m_pSignalConnectionToolBar->addWidget(m_signalConnectionTypeList);

		for(int s = 0; s < SIGNAL_CONNECTION_TYPE_COUNT; s++)
		{
			m_signalConnectionTypeList->addItem(qApp->translate("SignalConnectionBase.h", SignalConnectionType[s]));
		}

		m_signalConnectionTypeList->setCurrentIndex(theOptions.toolBar().signalConnectionType());

		connect(m_signalConnectionTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setSignalConnectionType);
	}

	// Control panel selecting analog signal
	//
	m_pAnalogSignalToolBar = new QToolBar(this);
	if (m_pAnalogSignalToolBar != nullptr)
	{
		m_pAnalogSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pAnalogSignalToolBar->setWindowTitle(tr("Control panel selecting analog signal"));
		m_pAnalogSignalToolBar->setObjectName(m_pAnalogSignalToolBar->windowTitle());
		addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(m_pAnalogSignalToolBar);

		QLabel* asRackLabel = new QLabel(m_pAnalogSignalToolBar);
		m_pAnalogSignalToolBar->addWidget(asRackLabel);
		asRackLabel->setText(tr(" Rack "));
		asRackLabel->setEnabled(false);

		m_rackCombo = new QComboBox(m_pAnalogSignalToolBar);
		m_pAnalogSignalToolBar->addWidget(m_rackCombo);
		m_rackCombo->setEnabled(false);
		m_rackCombo->setFixedWidth(100);
		connect(m_rackCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setRack);

		m_pAnalogSignalToolBar->addSeparator();

		QLabel* asSignalLabel = new QLabel(m_pAnalogSignalToolBar);
		m_pAnalogSignalToolBar->addWidget(asSignalLabel);
		asSignalLabel->setText(tr(" Signal "));
		asSignalLabel->setEnabled(false);

		m_selectSignalWidget = new SelectSignalWidget(this);
		m_pAnalogSignalToolBar->addWidget(m_selectSignalWidget);
		m_selectSignalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_selectSignalWidget->setEnabled(false);
		m_selectSignalWidget->setFixedWidth(530);
		connect(m_selectSignalWidget, &SelectSignalWidget::selectionChanged, this, &MainWindow::setAcitiveMeasureSignal);
		connect(&theSignalBase, &SignalBase::activeSignalChanged, m_selectSignalWidget, &SelectSignalWidget::activeSignalChanged, Qt::QueuedConnection);

		m_pAnalogSignalToolBar->addAction(m_pPreviousSignalAction);
		m_pAnalogSignalToolBar->addAction(m_pNextSignalAction);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
	// Search measurements panel
	//
	m_pFindMeasurePanel = new FindMeasurePanel(this);
	if (m_pFindMeasurePanel != nullptr)
	{
		m_pFindMeasurePanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::RightDockWidgetArea, m_pFindMeasurePanel);

		m_pFindMeasurePanel->hide();

		QAction* findAction = m_pFindMeasurePanel->toggleViewAction();
		if (findAction != nullptr)
		{
			findAction->setText(tr("&Find ..."));
			findAction->setShortcut(Qt::CTRL + Qt::Key_F);
			findAction->setIcon(QIcon(":/icons/Find.png"));
			findAction->setToolTip(tr("Find data in list of measurements"));

			if (m_pEditMenu != nullptr)
			{
				m_pEditMenu->addAction(findAction);
			}

			if (m_pMeasureControlToolBar != nullptr)
			{
				m_pMeasureControlToolBar->addAction(findAction);
			}
		}

		connect(this, &MainWindow::changedMeasureType, m_pFindMeasurePanel, &FindMeasurePanel::clear, Qt::QueuedConnection);
	}

	// Panel statistics
	//
	m_pStatisticPanel = new StatisticPanel(this);
	if (m_pStatisticPanel != nullptr)
	{
		m_pStatisticPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::RightDockWidgetArea, m_pStatisticPanel);

		if (m_pViewPanelMenu != nullptr)
		{
			m_pViewPanelMenu->addAction(m_pStatisticPanel->toggleViewAction());
		}

		m_pStatisticPanel->hide();

		connect(&theSignalBase, &SignalBase::activeSignalChanged, m_pStatisticPanel, &StatisticPanel::activeSignalChanged, Qt::QueuedConnection);
		connect(&theMeasureBase, &MeasureBase::updatedMeasureBase, m_pStatisticPanel, &StatisticPanel::updateSignalInList, Qt::QueuedConnection);

		connect(this, &MainWindow::changedMeasureType, m_pStatisticPanel, &StatisticPanel::changedMeasureType, Qt::QueuedConnection);
		connect(this, &MainWindow::changedSignalConnectionType, m_pStatisticPanel, &StatisticPanel::changedSignalConnectionType, Qt::QueuedConnection);

		connect(m_pStatisticPanel, &StatisticPanel::setSignalConnectionType, m_signalConnectionTypeList, &QComboBox::setCurrentIndex);
		connect(m_pStatisticPanel, &StatisticPanel::setRack, m_rackCombo, &QComboBox::setCurrentIndex);
		connect(m_pStatisticPanel, &StatisticPanel::setMeasureSignal, this, &MainWindow::setAcitiveMeasureSignal);
	}


	// Separator
	//
	if (m_pViewPanelMenu != nullptr)
	{
		m_pViewPanelMenu->addSeparator();
	}

	// Panel signal information
	//
	m_pSignalInfoPanel = new SignalInfoPanel(this);
	if (m_pSignalInfoPanel != nullptr)
	{
		m_pSignalInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_pSignalInfoPanel);

		if (m_pViewPanelMenu != nullptr)
		{
			m_pViewPanelMenu->addAction(m_pSignalInfoPanel->toggleViewAction());
		}

		m_pSignalInfoPanel->hide();

		connect(m_pSignalInfoPanel, &SignalInfoPanel::updateActiveOutputSignal, this, &MainWindow::updateActiveOutputSignal, Qt::QueuedConnection);
	}

	// Panel comparator information
	//
	m_pComparatorInfoPanel = new ComparatorInfoPanel(this);
	m_pComparatorInfoPanel->setObjectName("Panel comparator information");
	if (m_pComparatorInfoPanel != nullptr)
	{
		m_pComparatorInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_pComparatorInfoPanel);

		if (m_pViewPanelMenu != nullptr)
		{
			m_pViewPanelMenu->addAction(m_pComparatorInfoPanel->toggleViewAction());
		}

		m_pComparatorInfoPanel->hide();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMeasureViews()
{
	m_pMainTab = new QTabWidget();
	m_pMainTab->setTabPosition(QTabWidget::South);

	for(int measureType = 0; measureType < MEASURE_TYPE_COUNT; measureType++)
	{
		MeasureView* pView = new MeasureView(measureType, this);
		if (pView == nullptr)
		{
			continue;
		}

		pView->loadMeasureList();

		m_pMainTab->addTab(pView, qApp->translate("MeasureBase.h", MeasureType[measureType]));

		pView->setFrameStyle(QFrame::NoFrame);

		appendMeasureView(measureType, pView);

		connect(this, &MainWindow::appendMeasure, pView, &MeasureView::appendMeasure, Qt::QueuedConnection);
	}

	setCentralWidget(m_pMainTab);

	connect(m_pMainTab, &QTabWidget::currentChanged, this, &MainWindow::setMeasureType);
}

// -------------------------------------------------------------------------------------------------------------------

MeasureView* MainWindow::measureView(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		assert(false);
		return nullptr;
	}

	if (m_measureViewMap.contains(measureType) == false)
	{
		assert(false);
		return nullptr;
	}

	return m_measureViewMap[measureType];
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::appendMeasureView(int measureType, MeasureView* pView)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	if (pView == nullptr)
	{
		assert(false);
		return;
	}

	if (m_measureViewMap.contains(measureType) == true)
	{
		assert(false);
		return;
	}

	m_measureViewMap[measureType] = pView;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createStatusBar()
{
	QStatusBar* pStatusBar = statusBar();
	if (pStatusBar == nullptr)
	{
		return;
	}

	m_statusEmpty = new QLabel(pStatusBar);
	m_statusMeasureThreadInfo = new QLabel(pStatusBar);
	m_statusMeasureTimeout = new QProgressBar(pStatusBar);
	m_statusMeasureThreadState = new QLabel(pStatusBar);
	m_statusCalibratorCount = new QLabel(pStatusBar);
	m_statusConnectToConfigServer = new QLabel(pStatusBar);
	m_statusConnectToAppDataServer = new QLabel(pStatusBar);
	m_statusConnectToTuningServer = new QLabel(pStatusBar);

	m_statusMeasureTimeout->setTextVisible(false);
	m_statusMeasureTimeout->setRange(0, 100);
	m_statusMeasureTimeout->setFixedWidth(100);
	m_statusMeasureTimeout->setFixedHeight(10);
	m_statusMeasureTimeout->setLayoutDirection(Qt::LeftToRight);

	pStatusBar->addWidget(m_statusConnectToTuningServer);
	pStatusBar->addWidget(m_statusConnectToAppDataServer);
	pStatusBar->addWidget(m_statusConnectToConfigServer);
	pStatusBar->addWidget(m_statusCalibratorCount);
	pStatusBar->addWidget(m_statusMeasureThreadState);
	pStatusBar->addWidget(m_statusMeasureTimeout);
	pStatusBar->addWidget(m_statusMeasureThreadInfo);
	pStatusBar->addWidget(m_statusEmpty);

	pStatusBar->setLayoutDirection(Qt::RightToLeft);

	m_statusEmpty->setText(QString());

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: off "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToConfigServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));

	m_statusConnectToAppDataServer->setText(tr(" AppDataService: off "));
	m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToAppDataServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));

	m_statusConnectToTuningServer->setText(tr(" TuningService: off "));
	m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToTuningServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(this);

	m_pContextMenu->addAction(m_pCopyMeasureAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pRemoveMeasureAction);

	// init context menu
	//
	for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
	{
		MeasureView* pView = measureView(type);
		if (pView == nullptr)
		{
			continue;
		}

		pView->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(pView, &QTableView::customContextMenuRequested, this, &MainWindow::onContextMenu);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateRacksOnToolBar()
{
	if (m_rackCombo == nullptr)
	{
		return;
	}

	m_rackCombo->clear();
	m_rackCombo->setEnabled(false);

	int measureKind = theOptions.toolBar().measureKind();
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	int signalConnectionType = theOptions.toolBar().signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	int rackCount = theSignalBase.createRackListForMeasure(measureKind, signalConnectionType);
	if (rackCount == 0)
	{
		return;
	}

	int currentRackIndex = 0;

	// fill racks or rackGroups on Tool bar
	//
	m_rackCombo->blockSignals(true);

	for(int r = 0; r < rackCount; r++)
	{
		const Metrology::RackParam& rack = theSignalBase.rackForMeasure(r);
		if (rack.isValid() == false)
		{
			continue;
		}

		QString caption = rack.caption();
		if (caption.isEmpty() == true)
		{
			continue;
		}

		if (caption == theOptions.toolBar().defaultRack())
		{
			currentRackIndex = r;
		}

		// append rack index
		//
		m_rackCombo->addItem(caption, rack.index());
	}

	m_rackCombo->blockSignals(false);

	m_rackCombo->setCurrentIndex(currentRackIndex);
	m_rackCombo->setEnabled(true);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateSignalsOnToolBar()
{
	if (m_rackCombo == nullptr || m_selectSignalWidget == nullptr)
	{
		return;
	}

	int measureKind = theOptions.toolBar().measureKind();
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	int signalConnectionType = theOptions.toolBar().signalConnectionType();
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	// get rackIndex or rackGroupIndex, it depend from measureKind
	//
	int rackIndex = m_rackCombo->currentData().toInt();
	if (rackIndex == -1)
	{
		return;
	}

	// fill signal on Tool bar
	//
	std::vector<SelectSignalItem> signalList;

	int signalCount = theSignalBase.createSignalListForMeasure(measureKind, signalConnectionType, rackIndex);
	for(int index = 0; index < signalCount; index++)
	{
		const MeasureSignal& measureSignal = theSignalBase.signalForMeasure(index);
		if (measureSignal.isEmpty() == true)
		{
			continue;
		}

		SelectSignalItem signal(index, signalConnectionType, measureSignal);
		if (signal.isValid() == false)
		{
			continue;
		}

		signalList.push_back(signal);
	}

	int currentSignalID = m_selectSignalWidget->setSignalList(signalList, theOptions.toolBar().defaultSignalId());
	setAcitiveMeasureSignal(currentSignalID);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureType(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	MeasureView* pView = measureView(measureType);
	if (pView == nullptr)
	{
		return;
	}

	switch(measureType)
	{
		case MEASURE_TYPE_LINEARITY:
		case MEASURE_TYPE_COMPARATOR:

			m_pMeasureKind->show();
			m_pSignalConnectionToolBar->show();
			m_pAnalogSignalToolBar->show();

			m_pSignalInfoPanel->show();
			m_pComparatorInfoPanel->show();

			break;

		default:
			assert(0);
			break;
	}

	m_measureType = measureType;

	emit changedMeasureType(m_measureType);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::signalSocketIsConnected()
{
	if (m_pSignalSocket == nullptr || m_pSignalSocketThread == nullptr)
	{
		return false;
	}

	if (m_pSignalSocket->isConnected() == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::tuningSocketIsConnected()
{
	if (m_pTuningSocket == nullptr || m_pTuningSocketThread == nullptr)
	{
		return false;
	}

	if (m_pTuningSocket->isConnected() == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::signalSourceIsValid(bool showMsg)
{
	bool result = false;

	switch (theOptions.toolBar().signalConnectionType())
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:
		case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
		case SIGNAL_CONNECTION_TYPE_INPUT_OUTPUT:
		case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:
		case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:

			if (theCalibratorBase.connectedCalibratorsCount() == 0)
			{
				if (showMsg == true)
				{
					QMessageBox::critical(this, windowTitle(), tr("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators"));
				}
				break;
			}

			result = true;

			break;

		case SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT:

			if (tuningSocketIsConnected() == false)
			{
				if (showMsg == true)
				{
					QMessageBox::critical(this, windowTitle(), tr("No connect to Tuning Service!"));
				}
				break;
			}

			result = true;

			break;

		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startMeasure()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	if (m_measureThread.isRunning() == true)
	{
		QMessageBox::critical(this, windowTitle(), tr("Measurement process is already running"));
		return;
	}

	if (signalSocketIsConnected() == false)
	{
		QMessageBox::critical(this, windowTitle(), tr("No connect to Application Data Service!"));
		return;
	}

	if (signalSourceIsValid(true) == false)
	{
		return;
	}

	m_measureThread.setMeasureType(m_measureType);

	m_measureThread.start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopMeasure()
{
	if (m_measureThread.isFinished() == true)
	{
		return;
	}

	m_measureThread.stop();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::exportMeasure()
{
	MeasureView* pMeasureView = activeMeasureView();
	if (pMeasureView == nullptr)
	{
		return;
	}

	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	ExportData* dialog = new ExportData(pMeasureView, false, MeasureFileName[m_measureType]);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::copyMeasure()
{
	MeasureView* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->copy();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::removeMeasure()
{
	MeasureView* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->removeMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllMeasure()
{
	MeasureView* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showRackList()
{
	RackListDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	theSignalBase.updateRackParam();

	if (theSignalBase.racks().groups().save() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Attempt to save rack groups was unsuccessfully!"));
		return;
	}

	if (theOptions.toolBar().measureKind() == MEASURE_KIND_MULTI_RACK)
	{
		updateRacksOnToolBar();
		updateSignalsOnToolBar();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showSignalList()
{
	SignalListDialog dialog(false, this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showComparatorsList()
{
	ComparatorListDialog dialog(this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showTuningSignalList()
{
	TuningSignalListDialog dialog(this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showSignalConnectionList()
{
	SignalConnectionDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	if (theSignalBase.signalConnections().save() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Attempt to save signal connections was unsuccessfully!"));
		return;
	}

	updateSignalsOnToolBar();

	theSignalBase.statistic().createStatisticSignalList();
	theSignalBase.statistic().createStatisticComparatorList();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showStatistic()
{
	if (m_pStatisticPanel == nullptr)
	{
		return;
	}

	m_pStatisticPanel->show();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showCalibrators()
{
	theCalibratorBase.showInitDialog();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showCalculator()
{
	if (m_pCalculator == nullptr)
	{
		return;
	}

	m_pCalculator->show();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showOptions()
{
	Options options(theOptions);

	OptionsDialog dialog(this);
	dialog.exec();

	// reconnect ConfigSocket
	//
	if (options.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY) != theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY) ||
		options.socket().client(SOCKET_TYPE_CONFIG).address(SOCKET_SERVER_TYPE_PRIMARY) != theOptions.socket().client(SOCKET_TYPE_CONFIG).address(SOCKET_SERVER_TYPE_PRIMARY))
	{
		stopSignalSocket();
		stopTuningSocket();

		if (m_pConfigSocket != nullptr)
		{
			m_pConfigSocket->reconncect(theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY),
										theOptions.socket().client(SOCKET_TYPE_CONFIG).address(SOCKET_SERVER_TYPE_PRIMARY));
		}
	}

	// update columns in the measure views
	//
	for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
	{
		if (theOptions.measureView().updateColumnView(type) == false)
		{
			continue;
		}

		MeasureView* pView = measureView(type);
		if (pView == nullptr)
		{
			continue;
		}

		pView->updateColumn();
	}

	// update timeouts
	//
	if (options.signalInfo().timeForUpdate() != theOptions.signalInfo().timeForUpdate())
	{
		if (m_pSignalInfoPanel != nullptr)
		{
			m_pSignalInfoPanel->restartSignalStateTimer();
		}
	}

	if (options.comparatorInfo().timeForUpdate() != theOptions.comparatorInfo().timeForUpdate())
	{
		if (m_pComparatorInfoPanel != nullptr)
		{
			m_pComparatorInfoPanel->restartComparatorStateTimer();
		}
	}

	// if changed error type or limitType
	//
	if (options.linearity().errorType() != theOptions.linearity().errorType() || options.linearity().limitType() != theOptions.linearity().limitType() ||
		options.comparator().errorType() != theOptions.comparator().errorType() || options.comparator().limitType() != theOptions.comparator().limitType())
	{
		m_pStatisticPanel->updateList();
	}

	if(theOptions.language().languageType() == LANGUAGE_TYPE_RU)
	{
		QString languageFilePath = QApplication::applicationDirPath() + LANGUAGE_OPTIONS_DIR + "/" + LANGUAGE_OPTIONS_FILE_RU;
		if (QFile::exists(languageFilePath) == false)
		{
			QMessageBox::information(this, windowTitle(), tr("Russian language could not be installed.\nFile of russian language: %1 - was not found!").arg(languageFilePath));
			theOptions.language().setLanguageType(LANGUAGE_TYPE_EN);
			theOptions.language().save();
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutConnection()
{
	ProjectPropertyDialog dialog(theOptions.projectInfo());
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutApp()
{
	DialogAbout::show(this, tr(""), ":/Images/logo.png");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureKind(int index)
{
	int kind = index;
	if (kind < 0 || kind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	if (kind == MEASURE_KIND_MULTI_RACK)
	{
		if (theSignalBase.racks().groups().count() == 0)
		{
			m_measureKindList->blockSignals(true);
			m_measureKindList->setCurrentIndex(theOptions.toolBar().measureKind());
			m_measureKindList->blockSignals(false);

			QMessageBox::information(this, windowTitle(), tr("For measurements in several racks simultaneously, "
															 "you need to combine several racks into groups."
															 "Currently, no groups have been found.\n"
															 "To create a group of racks, click menu \"View\" - \"Racks ...\" ."));


			return;
		}
	}

	theOptions.toolBar().setMeasureKind(kind);
	theOptions.toolBar().save();

	updateRacksOnToolBar();
	updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureTimeout(QString value)
{
	theOptions.toolBar().setMeasureTimeout(static_cast<int>(value.toDouble() * 1000));
	theOptions.toolBar().save();

	m_statusMeasureTimeout->setRange(0, theOptions.toolBar().measureTimeout());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setSignalConnectionType(int index)
{
	if (index == -1)
	{
		return;
	}

	int type = index;
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	theOptions.toolBar().setSignalConnectionType(type);
	theOptions.toolBar().save();

	emit changedSignalConnectionType(type);

	updateRacksOnToolBar();
	updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setRack(int index)
{
	if (index < 0 || index >= theSignalBase.rackForMeasureCount())
	{
		return;
	}

	// set
	//
	updateSignalsOnToolBar();

	// save
	//
	const Metrology::RackParam& rack = theSignalBase.rackForMeasure(index);
	if (rack.isValid() == false)
	{
		return;
	}

	if (rack.caption().isEmpty() == true)
	{
		return;
	}

	theOptions.toolBar().setDefaultRack(rack.caption());
	theOptions.toolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setAcitiveMeasureSignal(int index)
{
	updatePrevNextSignalActions(index);

	if (index < 0 || index >= theSignalBase.signalForMeasureCount())
	{
		theSignalBase.clearActiveSignal();
		return;
	}

	const MeasureSignal& measureSignal = theSignalBase.signalForMeasure(index);
	if (measureSignal.isEmpty() == true)
	{
		assert(false);
		theSignalBase.clearActiveSignal();
		return;
	}

	// set
	//
	theSignalBase.setActiveSignal(measureSignal);

	// save
	//
	const MultiChannelSignal& signal = measureSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);
	if (signal.isEmpty() == true)
	{
		return;
	}

	if (signal.signalID().isEmpty() == true)
	{
		return;
	}

	theOptions.toolBar().setDefaultSignalId(signal.signalID());
	theOptions.toolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::previousMeasureSignal()
{
	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	int currentASignalIndex = m_selectSignalWidget->currentSignalIndex();
	if (currentASignalIndex < 0 || currentASignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentASignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return;
	}

	int previousSignalIndex = currentASignalIndex - 1;
	if (previousSignalIndex < 0 || previousSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	const MeasureSignal& previousActiveSignal = theSignalBase.signalForMeasure(previousSignalIndex);
	if (previousActiveSignal.isEmpty() == true)
	{
		return;
	}

	setAcitiveMeasureSignal(previousSignalIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::nextMeasureSignal()
{
	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	int currentASignalIndex = m_selectSignalWidget->currentSignalIndex();
	if (currentASignalIndex < 0 || currentASignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentASignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return;
	}

	int nextSignalIndex = currentASignalIndex + 1;
	if (nextSignalIndex < 0 || nextSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	const MeasureSignal& nextActiveSignal = theSignalBase.signalForMeasure(nextSignalIndex);
	if (nextActiveSignal.isEmpty() == true)
	{
		return;
	}

	setAcitiveMeasureSignal(nextSignalIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateActiveOutputSignal(int channel, Metrology::Signal* pOutputSignal)
{
	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	int index = m_selectSignalWidget->currentSignalIndex();
	if(index < 0 || index > theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal measureSignal = theSignalBase.signalForMeasure(index);
	if (measureSignal.isEmpty() == true)
	{
		return;
	}

	int measureKind = theOptions.toolBar().measureKind();
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	if (channel < 0 || channel >= measureSignal.channelCount())
	{
		return;
	}

	if (pOutputSignal == nullptr || pOutputSignal->param().isValid() == false)
	{
		return;
	}

	// change
	//
	MultiChannelSignal multiChannelSignal = measureSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);

	multiChannelSignal.setMetrologySignal(measureKind, channel, pOutputSignal);

	measureSignal.setMultiSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT, multiChannelSignal);

	// set
	//
	bool result = theSignalBase.setSignalForMeasure(index, measureSignal);

	if (result == false)
	{
		return;
	}

	m_selectSignalWidget->updateActiveOutputSignal(measureSignal);

	setAcitiveMeasureSignal(index);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::calibratorConnectedChanged(int count)
{
	m_statusCalibratorCount->setText(tr(" Connected calibrators: %1 ").arg(count));

	if (count == 0)
	{
		m_statusCalibratorCount->setStyleSheet("background-color: rgb(255, 160, 160);");
		m_statusCalibratorCount->setToolTip(tr("Please, connect Calibrators\nclick menu \"Tool\" - \"Calibrators...\""));
	}
	else
	{
		m_statusCalibratorCount->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");

		QString calibratorInfo;

		int calibratorCount = theCalibratorBase.calibratorCount();
		for(int i  = 0; i < calibratorCount; i++)
		{
			if (theCalibratorBase.calibratorManager(i) == nullptr)
			{
				continue;
			}

			Calibrator* pCalibrator = theCalibratorBase.calibratorManager(i)->calibrator();
			if (pCalibrator == nullptr || pCalibrator->isConnected() == false)
			{
				continue;
			}

			calibratorInfo.append(tr("Calibrator %1: %2, %3\n").arg(i+1).arg(pCalibrator->typeStr()).arg(pCalibrator->serialNo()));
		}

		m_statusCalibratorCount->setToolTip(calibratorInfo);
	}
}

// -------------------------------------------------------------------------------------------------------------------

QString MainWindow::configSocketConnectedStateStr()
{
	if (m_pConfigSocket == nullptr)
	{
		return QString();
	}

	QString connectedState;

	HostAddressPort configSocketAddress = m_pConfigSocket->address();

	connectedState = tr("Connected: %1 : %2\n\n").arg(configSocketAddress.addressStr()).arg(configSocketAddress.port());

	int filesCount = m_pConfigSocket->loadedFiles().count();

	connectedState.append(tr("Loaded files: %1").arg(filesCount));

	for(int f = 0; f < filesCount; f++)
	{
		connectedState.append("\n" + m_pConfigSocket->loadedFiles().at(f));
	}

	connectedState.append(tr("\n\nLoaded signals: %1").arg(theSignalBase.signalCount()));

	if (CFG_FILE_VER_METROLOGY_ITEMS_XML != theOptions.projectInfo().cfgFileVersion())
	{
		connectedState.append(tr("\n\nFailed version of %1. Current version: %2. Received version: %3 ")
								.arg(Builder::FILE_METROLOGY_ITEMS_XML)
								.arg(CFG_FILE_VER_METROLOGY_ITEMS_XML)
								.arg(theOptions.projectInfo().cfgFileVersion()));
	}

	return connectedState;
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

	HostAddressPort configSocketAddress = m_pConfigSocket->address();

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: on "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	m_statusConnectToConfigServer->setToolTip(configSocketConnectedStateStr());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketDisconnected()
{
	if (m_statusConnectToConfigServer == nullptr)
	{
		return;
	}

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: off "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToConfigServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketConfigurationLoaded()
{
	if (m_statusConnectToConfigServer == nullptr)
	{
		return;
	}

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: on "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	m_statusConnectToConfigServer->setToolTip(configSocketConnectedStateStr());

	if (theSignalBase.signalCount() == 0)
	{
		m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(255, 255, 160);");
	}

	updateRacksOnToolBar();
	updateSignalsOnToolBar();

	//
	//
	runSignalSocket();
	runTuningSocket();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalSocketConnected()
{
	if (m_pSignalSocket == nullptr)
	{
		return;
	}

	if (m_statusConnectToAppDataServer == nullptr)
	{
		return;
	}

	int serverType = m_pSignalSocket->selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return;
	}

	HostAddressPort signalSocketAddress = theOptions.socket().client(SOCKET_TYPE_SIGNAL).address(serverType);

	m_statusConnectToAppDataServer->setText(tr(" AppDataService: on "));
	m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	m_statusConnectToAppDataServer->setToolTip(tr("Connected: %1 : %2\n").arg(signalSocketAddress.addressStr()).arg(signalSocketAddress.port()));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalSocketDisconnected()
{
	if (m_statusConnectToAppDataServer == nullptr)
	{
		return;
	}

	if (m_measureThread.isRunning() == true)
	{
		m_measureThread.stop();
	}

	m_statusConnectToAppDataServer->setText(tr(" AppDataService: off "));
	m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToAppDataServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

QString MainWindow::tuningSocketConnectedStateStr()
{
	if (m_pTuningSocket == nullptr)
	{
		return QString();
	}

	int serverType = m_pTuningSocket->selectedServerIndex();
	if (serverType < 0 || serverType >= SOCKET_SERVER_TYPE_COUNT)
	{
		return QString();
	}

	QString connectedState;

	HostAddressPort tuningSocketAddress = theOptions.socket().client(SOCKET_TYPE_TUNING).address(serverType);

	connectedState = tr("Connected: %1 : %2\n").arg(tuningSocketAddress.addressStr()).arg(tuningSocketAddress.port());

	connectedState.append(tr("\nTuning sources: %1").arg(theSignalBase.tuning().sourceBase().count()));
	connectedState.append(tr("\nTuning signals: %1").arg(theSignalBase.tuning().signalBase().count()));

	return connectedState;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::tuningSocketConnected()
{
	if (m_statusConnectToTuningServer == nullptr)
	{
		return;
	}

	m_statusConnectToTuningServer->setText(tr(" TuningService: on "));
	m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	m_statusConnectToTuningServer->setToolTip(tuningSocketConnectedStateStr());

	if (theSignalBase.tuning().sourceBase().count() == 0 && theSignalBase.tuning().signalBase().count() != 0)
	{
		m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 255, 160);");
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::tuningSocketDisconnected()
{
	if (m_statusConnectToTuningServer == nullptr)
	{
		return;
	}

	if (m_measureThread.isRunning() == true)
	{
		if (theOptions.toolBar().signalConnectionType() == SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
		{
			m_measureThread.stop();
		}
	}

	theSignalBase.tuning().sourceBase().clear();
	theSignalBase.tuning().signalBase().setNovalid();

	m_statusConnectToTuningServer->setText(tr(" TuningService: off "));
	m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToTuningServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::tuningSignalsCreated()
{
	if (m_pTuningSocket == nullptr)
	{
		return;
	}

	if (m_statusConnectToTuningServer == nullptr)
	{
		return;
	}

	if (m_pTuningSocket->isConnected() == true)
	{
		m_statusConnectToTuningServer->setText(tr(" TuningService: on "));
		m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");

		if (theSignalBase.tuning().sourceBase().count() == 0 && theSignalBase.tuning().signalBase().count() != 0)
		{
			m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 255, 160);");
		}
	}
	else
	{
		m_statusConnectToTuningServer->setText(tr(" TuningService: off "));
		m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	}

	m_statusConnectToTuningServer->setToolTip(tuningSocketConnectedStateStr());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadStarted()
{
	m_pMeasureKind->setDisabled(true);
	m_pSignalConnectionToolBar->setDisabled(true);
	m_pAnalogSignalToolBar->setDisabled(true);

	m_statusMeasureThreadInfo->setText(QString());
	m_statusMeasureThreadInfo->show();

	if (theOptions.toolBar().measureTimeout() != 0)
	{
		m_statusMeasureTimeout->show();
		m_statusMeasureTimeout->setRange(0, theOptions.toolBar().measureTimeout());
		m_statusMeasureTimeout->setValue(0);
	}
	else
	{
		 m_statusMeasureTimeout->hide();
	}

	m_statusMeasureThreadState->setText(tr(" Measure process is running "));
	m_statusMeasureThreadState->setStyleSheet("background-color: rgb(160, 255, 160);");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadStoped()
{
	m_pMeasureKind->setEnabled(true);
	m_pSignalConnectionToolBar->setEnabled(true);
	m_pAnalogSignalToolBar->setEnabled(true);

	m_statusMeasureThreadInfo->setText(QString());
	m_statusMeasureThreadInfo->hide();

	m_statusMeasureTimeout->hide();
	m_statusMeasureTimeout->setRange(0, theOptions.toolBar().measureTimeout());
	m_statusMeasureTimeout->setValue(0);

	m_statusMeasureThreadState->setText(tr(" Measure process is stopped "));
	m_statusMeasureThreadState->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureThreadInfo(QString msg)
{
	m_statusMeasureThreadInfo->setText(msg);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureThreadInfo(int timeout)
{
	m_statusMeasureTimeout->setValue(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadMsgBox(int type, QString text, int* result)
{
	switch (type)
	{
		case QMessageBox::Question:

			if (result == nullptr)
			{
				break;
			}

			*result = QMessageBox::question(this, windowTitle(), text);

			break;

		case QMessageBox::Information:

			QMessageBox::information(this, windowTitle(), text);

			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setNextMeasureSignal(bool& signalIsSelected)
{
	signalIsSelected = false;

	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	int currentASignalIndex = m_selectSignalWidget->currentSignalIndex();
	if (currentASignalIndex < 0 || currentASignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentASignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return;
	}

	int nextSignalIndex = currentASignalIndex + 1;
	if (nextSignalIndex < 0 || nextSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	const MeasureSignal& nextActiveSignal = theSignalBase.signalForMeasure(nextSignalIndex);
	if (nextActiveSignal.isEmpty() == true)
	{
		return;
	}

	// if module numbers not equal then disabling selection of next input
	//
	if (	currentActiveSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).location().chassis() != nextActiveSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).location().chassis() ||
			currentActiveSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).location().module() != nextActiveSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).location().module())
	{
		return;
	}

	setAcitiveMeasureSignal(nextSignalIndex);

	signalIsSelected = true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureComplite(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return;
	}

	int type = pMeasurement->measureType();
	if (type < 0 || type >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	emit appendMeasure(pMeasurement);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateStartStopActions()
{
	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	if (m_pStartMeasureAction == nullptr || m_pStopMeasureAction == nullptr)
	{
		return;
	}

	m_pStartMeasureAction->setEnabled(false);
	m_pStopMeasureAction->setEnabled(false);

	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	if (signalSocketIsConnected() == false)
	{
		return;
	}

	if (signalSourceIsValid(false) == false)
	{
		return;
	}

	if (m_selectSignalWidget == nullptr)
	{
		return;
	}

	int currentSignalIndex = m_selectSignalWidget->currentSignalIndex();
	if (currentSignalIndex < 0 || currentSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal activeSignal = theSignalBase.signalForMeasure(currentSignalIndex);
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	if (m_measureThread.isRunning() == false)
	{
		m_pStartMeasureAction->setEnabled(true);
		m_pStopMeasureAction->setEnabled(false);
	}
	else
	{
		m_pStartMeasureAction->setEnabled(false);
		m_pStopMeasureAction->setEnabled(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updatePrevNextSignalActions(int signalIndex)
{
	if(m_selectSignalWidget == nullptr)
	{
		return;
	}

	if (m_pPreviousSignalAction == nullptr || m_pNextSignalAction == nullptr)
	{
		return;
	}

	if (m_selectSignalWidget->count() == 0)
	{
		m_selectSignalWidget->setEnabled(false);
		m_pPreviousSignalAction->setEnabled(false);
		m_pNextSignalAction->setEnabled(false);

		return;
	}

	m_selectSignalWidget->setEnabled(true);
	m_pPreviousSignalAction->setEnabled(true);
	m_pNextSignalAction->setEnabled(true);

	if (signalIndex == 0)
	{
		m_pPreviousSignalAction->setEnabled(false);
	}

	if (signalIndex == m_selectSignalWidget->count() - 1)
	{
		m_pNextSignalAction->setEnabled(false);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runConfigSocket()
{
	// init config socket thread
	//
	HostAddressPort configSocketAddress = theOptions.socket().client(SOCKET_TYPE_CONFIG).address(SOCKET_SERVER_TYPE_PRIMARY);
	m_softwareInfo.setEquipmentID(theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY));
	m_pConfigSocket = new ConfigSocket(m_softwareInfo, configSocketAddress);

	connect(m_pConfigSocket, &ConfigSocket::socketConnected, this, &MainWindow::configSocketConnected, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::socketDisconnected, this, &MainWindow::configSocketDisconnected, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, m_pStatisticPanel, &StatisticPanel::updateList);
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &MainWindow::configSocketConfigurationLoaded);
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, &theSignalBase.statistic(), &StatisticBase::signalLoaded);

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

void MainWindow::runSignalSocket()
{
	// init signal socket thread
	//
	HostAddressPort signalSocketAddress1 = theOptions.socket().client(SOCKET_TYPE_SIGNAL).address(SOCKET_SERVER_TYPE_PRIMARY);
	HostAddressPort signalSocketAddress2 = theOptions.socket().client(SOCKET_TYPE_SIGNAL).address(SOCKET_SERVER_TYPE_RESERVE);
	m_softwareInfo.setEquipmentID(theOptions.socket().client(SOCKET_TYPE_SIGNAL).equipmentID(SOCKET_SERVER_TYPE_PRIMARY));

	m_pSignalSocket = new SignalSocket(m_softwareInfo, signalSocketAddress1, signalSocketAddress2);
	m_pSignalSocketThread = new SimpleThread(m_pSignalSocket);

	connect(m_pSignalSocket, &SignalSocket::socketConnected, this, &MainWindow::signalSocketConnected, Qt::QueuedConnection);
	connect(m_pSignalSocket, &SignalSocket::socketDisconnected, this, &MainWindow::signalSocketDisconnected, Qt::QueuedConnection);
	connect(m_pSignalSocket, &SignalSocket::socketDisconnected, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(m_pSignalSocket, &SignalSocket::socketDisconnected, &m_measureThread, &MeasureThread::signalSocketDisconnected, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, m_pSignalSocket, &SignalSocket::configurationLoaded, Qt::QueuedConnection);

	m_pSignalSocketThread->start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopSignalSocket()
{
	if (m_pSignalSocketThread == nullptr)
	{
		return;
	}

	m_pSignalSocketThread->quitAndWait(10000);
	delete m_pSignalSocketThread;
	m_pSignalSocketThread = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runTuningSocket()
{
	// init tuning socket thread
	//
	HostAddressPort tuningSocketAddress = theOptions.socket().client(SOCKET_TYPE_TUNING).address(SOCKET_SERVER_TYPE_PRIMARY);
	m_softwareInfo.setEquipmentID(theOptions.socket().client(SOCKET_TYPE_TUNING).equipmentID(SOCKET_SERVER_TYPE_PRIMARY));

	m_pTuningSocket = new TuningSocket(m_softwareInfo, tuningSocketAddress);
	m_pTuningSocketThread = new SimpleThread(m_pTuningSocket);

	connect(m_pTuningSocket, &TuningSocket::sourcesLoaded, this, &MainWindow::tuningSignalsCreated, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::socketConnected, this, &MainWindow::tuningSocketConnected, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::socketDisconnected, this, &MainWindow::tuningSocketDisconnected, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::socketDisconnected, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::socketDisconnected, &m_measureThread, &MeasureThread::tuningSocketDisconnected, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, m_pTuningSocket, &TuningSocket::configurationLoaded, Qt::QueuedConnection);

	m_pTuningSocketThread->start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::runMeasureThread()
{
	connect(&m_measureThread, &MeasureThread::started, this, &MainWindow::measureThreadStarted, Qt::QueuedConnection);
	connect(&m_measureThread, &MeasureThread::finished, this, &MainWindow::measureThreadStoped, Qt::QueuedConnection);
	connect(&m_measureThread, &MeasureThread::started, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(&m_measureThread, &MeasureThread::finished, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(&m_measureThread, static_cast<void (MeasureThread::*)(QString)>(&MeasureThread::measureInfo), this, static_cast<void (MainWindow::*)(QString)>(&MainWindow::setMeasureThreadInfo), Qt::QueuedConnection);
	connect(&m_measureThread, static_cast<void (MeasureThread::*)(int)>(&MeasureThread::measureInfo), this, static_cast<void (MainWindow::*)(int)>(&MainWindow::setMeasureThreadInfo), Qt::QueuedConnection);
	connect(&m_measureThread, &MeasureThread::msgBox, this, &MainWindow::measureThreadMsgBox, Qt::BlockingQueuedConnection);
	connect(&m_measureThread, &MeasureThread::setNextMeasureSignal, this, &MainWindow::setNextMeasureSignal, Qt::BlockingQueuedConnection);
	connect(&m_measureThread, &MeasureThread::measureComplite, this, &MainWindow::measureComplite, Qt::QueuedConnection);

	connect(&theSignalBase, &SignalBase::updatedSignalParam, &m_measureThread, &MeasureThread::updateSignalParam, Qt::QueuedConnection);

	measureThreadStoped();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopMeasureThread()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopTuningSocket()
{
	if (m_pTuningSocketThread == nullptr)
	{
		return;
	}

	m_pTuningSocketThread->quitAndWait(10000);
	delete m_pTuningSocketThread;
	m_pTuningSocketThread = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadSettings()
{
	QSettings s;

	QByteArray geometry = s.value(QString("%1MainWindow/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
	QByteArray state = s.value(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();

	restoreGeometry(geometry);
	restoreState(state);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1MainWindow/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
	s.setValue(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveState());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
	if (m_measureThread.isRunning() == true)
	{
		QMessageBox::critical(this, windowTitle(), m_statusMeasureThreadState->text());
		e->ignore();
		return;
	}

	if (m_pCalculator != nullptr)
	{
		delete m_pCalculator;
		m_pCalculator = nullptr;
	}

	stopConfigSocket();
	stopSignalSocket();
	stopTuningSocket();

	theMeasureBase.clear();
	theSignalBase.clear();
	theCalibratorBase.clear();

	theDatabase.close();

	saveSettings();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
