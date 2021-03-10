#include "MainWindow.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QCloseEvent>

#include "../lib/Ui/DialogAbout.h"
#include "../lib/ConstStrings.h"

#include "CalibratorBase.h"
#include "Database.h"
#include "SignalBase.h"
#include "ProcessData.h"
#include "DialogRackList.h"
#include "DialogSignalList.h"
#include "DialogComparatorList.h"
#include "DialogTuningSignalList.h"
#include "DialogMetrologyConnection.h"
#include "DialogObjectProperties.h"
#include "Options.h"
#include "DialogOptions.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent)
	: QMainWindow(parent)
	, m_softwareInfo(softwareInfo)
{
	// open database
	//
	theDatabase.setDatabaseOption(theOptions.database());
	theDatabase.open();
	connect(this, &MainWindow::appendMeasure, &theDatabase, &Database::appendToBase, Qt::QueuedConnection);


	// init calibration base
	//
	m_calibratorBase.init(theOptions.calibrators(), this);
	connect(&m_calibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::calibratorConnectedChanged, Qt::QueuedConnection);
	connect(&m_calibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);

	// load signal base
	//
	theSignalBase.racks().groups().load();		// load rack groups for multichannel measuring
	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &MainWindow::updateStartStopActions, Qt::QueuedConnection);
	connect(&theSignalBase.tuning().signalBase(), &TuningSignalBase::signalsCreated, this, &MainWindow::tuningSignalsCreated, Qt::QueuedConnection);

	// load measurement base
	//
	theOptions.linearity().points().load();

	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		m_measureBase.load(static_cast<Measure::Type>(measureType));
	}
	connect(this, &MainWindow::appendMeasure, &m_measureBase, &Measure::Base::appendToBase, Qt::QueuedConnection);
	connect(&m_measureBase, &Measure::Base::updateMeasureView, this, &MainWindow::updateMeasureView, Qt::QueuedConnection);

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

	setMeasureType(Measure::Type::Linearity);

	m_pCalculator = new DialogCalculator(this);	// calculator

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
	m_pShowRackListAction->setToolTip(QString());
	connect(m_pShowRackListAction, &QAction::triggered, this, &MainWindow::showRackList);

	m_pShowSignalListAction = new QAction(tr("&Signals ..."), this);
	m_pShowSignalListAction->setIcon(QIcon(":/icons/Signal.png"));
	m_pShowSignalListAction->setToolTip(QString());
	connect(m_pShowSignalListAction, &QAction::triggered, this, &MainWindow::showSignalList);

	m_pShowComparatorsListAction = new QAction(tr("&Comparators ..."), this);
	m_pShowComparatorsListAction->setIcon(QIcon(":/icons/Comparator.png"));
	m_pShowComparatorsListAction->setToolTip(QString());
	connect(m_pShowComparatorsListAction, &QAction::triggered, this, &MainWindow::showComparatorsList);

	m_pShowTuningSignalListAction = new QAction(tr("Tuning signals ..."), this);
	m_pShowTuningSignalListAction->setIcon(QIcon(":/icons/Tuning.png"));
	m_pShowTuningSignalListAction->setToolTip(QString());
	connect(m_pShowTuningSignalListAction, &QAction::triggered, this, &MainWindow::showTuningSignalList);

	m_pShowConnectionListAction = new QAction(tr("Metrology connections ..."), this);
	m_pShowConnectionListAction->setIcon(QIcon(":/icons/Connection.png"));
	m_pShowConnectionListAction->setToolTip(QString());
	connect(m_pShowConnectionListAction, &QAction::triggered, this, &MainWindow::showConnectionList);

	m_pShowGraphLinElAction = new QAction(tr("Linearity: electric range ..."), this);
	m_pShowGraphLinElAction->setIcon(QIcon(":/icons/Graph.png"));
	m_pShowGraphLinElAction->setToolTip(tr("Show linearity graph"));
	connect(m_pShowGraphLinElAction, &QAction::triggered, this, &MainWindow::showGraphLinEl);

	m_pShowGraphLinEnAction = new QAction(tr("Linearity: engineering range ..."), this);
	m_pShowGraphLinEnAction->setIcon(QIcon(":/icons/Graph.png"));
	m_pShowGraphLinEnAction->setToolTip(tr("Show linearity graph"));
	connect(m_pShowGraphLinEnAction, &QAction::triggered, this, &MainWindow::showGraphLinEn);

	m_pShowGraph20ElAction = new QAction(tr("Detail in the point: electric range ..."), this);
	m_pShowGraph20ElAction->setIcon(QIcon(":/icons/Graph.png"));
	m_pShowGraph20ElAction->setToolTip(tr("Show linearity graph"));
	connect(m_pShowGraph20ElAction, &QAction::triggered, this, &MainWindow::showGraph20El);

	m_pShowGraph20EnAction = new QAction(tr("Detail in the point: engineering range ..."), this);
	m_pShowGraph20EnAction->setIcon(QIcon(":/icons/Graph.png"));
	m_pShowGraph20EnAction->setToolTip(tr("Show linearity graph"));
	connect(m_pShowGraph20EnAction, &QAction::triggered, this, &MainWindow::showGraph20En);

	m_pShowStatisticsAction = new QAction(tr("Sta&tistics (Checklist) ..."), this);
	m_pShowStatisticsAction->setIcon(QIcon(":/icons/Statistics.png"));
	m_pShowStatisticsAction->setToolTip(QString());
	connect(m_pShowStatisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);

	// Tools
	//
	m_pCalibratorsAction = new QAction(tr("&Calibrators ..."), this);
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
	m_pAboutConnectionAction->setToolTip(QString());
	connect(m_pAboutConnectionAction, &QAction::triggered, this, &MainWindow::aboutConnection);

	m_pAboutAppAction = new QAction(tr("About Metrology ..."), this);
	m_pAboutAppAction->setIcon(QIcon(":/icons/About Application.png"));
	m_pAboutAppAction->setToolTip(QString());
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
	m_pViewGraphMenu = new QMenu(tr("&Graphs of linearity"), m_pViewMenu);

	m_pViewMenu->addMenu(m_pViewPanelMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addAction(m_pShowRackListAction);
	m_pViewMenu->addAction(m_pShowSignalListAction);
	m_pViewMenu->addAction(m_pShowComparatorsListAction);
	m_pViewMenu->addAction(m_pShowTuningSignalListAction);
	m_pViewMenu->addAction(m_pShowConnectionListAction);
	m_pViewMenu->addSeparator();
	m_pViewGraphMenu->addAction(m_pShowGraphLinElAction);
	m_pViewGraphMenu->addAction(m_pShowGraphLinEnAction);
	m_pViewGraphMenu->addSeparator();
	m_pViewGraphMenu->addAction(m_pShowGraph20ElAction);
	m_pViewGraphMenu->addAction(m_pShowGraph20EnAction);
	m_pViewMenu->addMenu(m_pViewGraphMenu);
	m_pViewMenu->addAction(m_pShowStatisticsAction);

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
	m_pMeasureTimeoutToolBar = new QToolBar(this);
	if (m_pMeasureTimeoutToolBar != nullptr)
	{
		m_pMeasureTimeoutToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pMeasureTimeoutToolBar->setWindowTitle(tr("Control panel measure timeout"));
		m_pMeasureTimeoutToolBar->setObjectName(m_pMeasureTimeoutToolBar->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pMeasureTimeoutToolBar);

		QLabel* pMeasureTimeoutLabel = new QLabel(m_pMeasureTimeoutToolBar);
		QComboBox* pMeasureTimeoutList = new QComboBox(m_pMeasureTimeoutToolBar);
		QLabel* pMeasureTimeoutUnitLabel = new QLabel(m_pMeasureTimeoutToolBar);
		QRegExp rx("^[0-9]*[.]{1}[0-9]*$");
		QValidator* validator = new QRegExpValidator(rx, m_pMeasureTimeoutToolBar);

		m_pMeasureTimeoutToolBar->addWidget(pMeasureTimeoutLabel);
		m_pMeasureTimeoutToolBar->addWidget(pMeasureTimeoutList);
		m_pMeasureTimeoutToolBar->addWidget(pMeasureTimeoutUnitLabel);

		pMeasureTimeoutLabel->setText(tr(" Measure timeout "));
		pMeasureTimeoutLabel->setEnabled(false);

		pMeasureTimeoutList->setEditable(true);
		pMeasureTimeoutList->setValidator(validator);

		for(int t = 0; t < Measure::TimeoutCount; t++)
		{
			pMeasureTimeoutList->addItem(QString::number(Measure::Timeout[t], 'f', 1));
		}

		m_measureTimeout = theOptions.toolBar().measureTimeout();
		pMeasureTimeoutList->setCurrentText(QString::number(double(m_measureTimeout) / 1000, 'f', 1));

		pMeasureTimeoutUnitLabel->setText(tr(" sec."));
		pMeasureTimeoutUnitLabel->setEnabled(false);

		connect(pMeasureTimeoutList, &QComboBox::currentTextChanged, this, &MainWindow::setMeasureTimeout);
	}


	// Control panel measure kind
	//
	m_pMeasureKindToolBar = new QToolBar(this);
	if (m_pMeasureKindToolBar != nullptr)
	{
		m_pMeasureKindToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pMeasureKindToolBar->setWindowTitle(tr("Control panel measure kind"));
		m_pMeasureKindToolBar->setObjectName(m_pMeasureKindToolBar->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pMeasureKindToolBar);

		QLabel* pMeasureKindLabel = new QLabel(m_pMeasureKindToolBar);
		m_pMeasureKindList = new QComboBox(m_pMeasureKindToolBar);

		if (m_pMeasureKindList != nullptr)
		{
			m_pMeasureKindToolBar->addWidget(pMeasureKindLabel);
			m_pMeasureKindToolBar->addWidget(m_pMeasureKindList);

			pMeasureKindLabel->setText(tr(" Measure kind "));
			pMeasureKindLabel->setEnabled(false);

			loadOnToolBar_MeasureKind();

			connect(m_pMeasureKindList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this, &MainWindow::setMeasureKind);

			m_calibratorBase.measureKindChanged(m_measureKind);
			connect(this, &MainWindow::measureKindChanged, &m_calibratorBase, &CalibratorBase::measureKindChanged, Qt::QueuedConnection);
		}
	}


	// Control panel metrology connections
	//
	m_pConnectionToolBar = new QToolBar(this);
	if (m_pConnectionToolBar != nullptr)
	{
		m_pConnectionToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		m_pConnectionToolBar->setWindowTitle(tr("Control panel metrology connections"));
		m_pConnectionToolBar->setObjectName(m_pConnectionToolBar->windowTitle());
		addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(m_pConnectionToolBar);

		QLabel* pConnectionLabel = new QLabel(m_pConnectionToolBar);
		m_pConnectionTypeList = new QComboBox(m_pConnectionToolBar);
		if (m_pConnectionTypeList != nullptr)
		{
			m_pConnectionToolBar->addWidget(pConnectionLabel);
			pConnectionLabel->setText(tr(" Metrology connections "));
			pConnectionLabel->setEnabled(false);

			m_pConnectionToolBar->addWidget(m_pConnectionTypeList);

			loadOnToolBar_Connection();

			connect(m_pConnectionTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this, &MainWindow::setConnectionType);
		}
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

		QLabel* pRackLabel = new QLabel(m_pAnalogSignalToolBar);
		m_pAnalogSignalToolBar->addWidget(pRackLabel);
		pRackLabel->setText(tr(" Rack "));
		pRackLabel->setEnabled(false);

		m_pRackCombo = new QComboBox(m_pAnalogSignalToolBar);
		if (m_pRackCombo != nullptr)
		{
			m_pAnalogSignalToolBar->addWidget(m_pRackCombo);
			m_pRackCombo->setEnabled(false);
			m_pRackCombo->setFixedWidth(100);

			connect(m_pRackCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setRack);
		}

		m_pAnalogSignalToolBar->addSeparator();

		QLabel* pSignalLabel = new QLabel(m_pAnalogSignalToolBar);
		m_pAnalogSignalToolBar->addWidget(pSignalLabel);
		pSignalLabel->setText(tr(" Signal "));
		pSignalLabel->setEnabled(false);

		m_pSelectSignalWidget = new SelectSignalWidget(this);
		if (m_pSelectSignalWidget != nullptr)
		{
			m_pAnalogSignalToolBar->addWidget(m_pSelectSignalWidget);
			m_pSelectSignalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			m_pSelectSignalWidget->setEnabled(false);
			m_pSelectSignalWidget->setFixedWidth(530);

			connect(m_pSelectSignalWidget, &SelectSignalWidget::selectionChanged, this, &MainWindow::setAcitiveMeasureSignal);
			connect(&theSignalBase, &SignalBase::activeSignalChanged,
					m_pSelectSignalWidget, &SelectSignalWidget::activeSignalChanged, Qt::QueuedConnection);
		}

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
	m_pFindMeasurePanel = new PanelFindMeasure(this);
	if (m_pFindMeasurePanel != nullptr)
	{
		m_pFindMeasurePanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_pFindMeasurePanel->setViewFont(theOptions.measureView().font());

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

		connect(this, &MainWindow::measureViewChanged, m_pFindMeasurePanel, &PanelFindMeasure::measureViewChanged, Qt::QueuedConnection);
	}

	// Panel statistics
	//
	m_pStatisticsPanel = new PanelStatistics(this);
	if (m_pStatisticsPanel != nullptr)
	{
		m_pStatisticsPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
		m_pStatisticsPanel->setViewFont(theOptions.measureView().font());

		addDockWidget(Qt::RightDockWidgetArea, m_pStatisticsPanel);

		if (m_pViewPanelMenu != nullptr)
		{
			m_pViewPanelMenu->addAction(m_pStatisticsPanel->toggleViewAction());
		}

		m_pStatisticsPanel->hide();

		m_pStatisticsPanel->setMeasureBase(&m_measureBase);
		m_pStatisticsPanel->measureKindChanged(m_measureKind);
		m_pStatisticsPanel->connectionTypeChanged(m_connectionType);

		connect(&theSignalBase, &SignalBase::activeSignalChanged, m_pStatisticsPanel, &PanelStatistics::activeSignalChanged, Qt::QueuedConnection);
		connect(&m_measureBase, &Measure::Base::updatedMeasureBase, m_pStatisticsPanel, &PanelStatistics::updateSignalInList, Qt::QueuedConnection);

		connect(this, &MainWindow::measureTypeChanged, m_pStatisticsPanel, &PanelStatistics::measureTypeChanged, Qt::QueuedConnection);
		connect(this, &MainWindow::measureKindChanged, m_pStatisticsPanel, &PanelStatistics::measureKindChanged, Qt::QueuedConnection);
		connect(this, &MainWindow::connectionTypeChanged, m_pStatisticsPanel, &PanelStatistics::connectionTypeChanged, Qt::QueuedConnection);

		connect(m_pStatisticsPanel, &PanelStatistics::setConnectionType, this, &MainWindow::setConnectionTypeFromStatistic);
		connect(m_pStatisticsPanel, &PanelStatistics::setRack, m_pRackCombo, &QComboBox::setCurrentIndex);
		connect(m_pStatisticsPanel, &PanelStatistics::setMeasureSignal, this, &MainWindow::setAcitiveMeasureSignal);
		connect(m_pStatisticsPanel, &PanelStatistics::showFindMeasurePanel, this, &MainWindow::showFindMeasurePanel, Qt::QueuedConnection);
	}


	// Separator
	//
	if (m_pViewPanelMenu != nullptr)
	{
		m_pViewPanelMenu->addSeparator();
	}

	// Panel signal information
	//
	m_pSignalInfoPanel = new PanelSignalInfo(theOptions.signalInfo(), this);
	if (m_pSignalInfoPanel != nullptr)
	{
		m_pSignalInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_pSignalInfoPanel);

		if (m_pViewPanelMenu != nullptr)
		{
			m_pViewPanelMenu->addAction(m_pSignalInfoPanel->toggleViewAction());
		}

		m_pSignalInfoPanel->hide();


		m_pSignalInfoPanel->setCalibratorBase(&m_calibratorBase);
		m_pSignalInfoPanel->measureKindChanged(m_measureKind);
		m_pSignalInfoPanel->connectionTypeChanged(m_connectionType);

		connect(this, &MainWindow::measureKindChanged, m_pSignalInfoPanel, &PanelSignalInfo::measureKindChanged, Qt::QueuedConnection);
		connect(this, &MainWindow::connectionTypeChanged, m_pSignalInfoPanel, &PanelSignalInfo::connectionTypeChanged, Qt::QueuedConnection);

		connect(m_pSignalInfoPanel, &PanelSignalInfo::changeActiveDestSignal, this, &MainWindow::changeActiveDestSignal, Qt::QueuedConnection);
		connect(m_pSignalInfoPanel, &PanelSignalInfo::changeActiveDestSignals, this, &MainWindow::changeActiveDestSignals, Qt::QueuedConnection);
	}

	// Panel comparator information
	//
	m_pComparatorInfoPanel = new PanelComparatorInfo(theOptions.comparatorInfo(), this);
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

		m_pComparatorInfoPanel->setCalibratorBase(&m_calibratorBase);
		m_pComparatorInfoPanel->measureKindChanged(m_measureKind);
		m_pComparatorInfoPanel->connectionTypeChanged(m_connectionType);

		connect(this, &MainWindow::measureKindChanged,
				m_pComparatorInfoPanel, &PanelComparatorInfo::measureKindChanged, Qt::QueuedConnection);

		connect(this, &MainWindow::connectionTypeChanged,
				m_pComparatorInfoPanel, &PanelComparatorInfo::connectionTypeChanged, Qt::QueuedConnection);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMeasureViews()
{
	m_pMainTab = new QTabWidget();
	m_pMainTab->setTabPosition(QTabWidget::South);

	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		Measure::View* pView = new Measure::View(static_cast<Measure::Type>(measureType), this);
		if (pView == nullptr)
		{
			continue;
		}

		pView->loadMeasurements(m_measureBase);

		m_pMainTab->addTab(pView, qApp->translate("MeasureBase", Measure::TypeCaption(measureType).toUtf8()));

		pView->setFrameStyle(QFrame::NoFrame);

		appendMeasureView(measureType, pView);

		//
		//

		connect(pView, &Measure::View::removeFromBase, &theDatabase, &Database::removeFromBase, Qt::QueuedConnection);
		connect(pView, &Measure::View::removeFromBase, &m_measureBase, &Measure::Base::removeFromBase, Qt::QueuedConnection);

		connect(this, &MainWindow::appendMeasure, pView, &Measure::View::appendMeasure, Qt::QueuedConnection);
	}

	setCentralWidget(m_pMainTab);

	connect(m_pMainTab, &QTabWidget::currentChanged, this, &MainWindow::setMeasureType);
}

// -------------------------------------------------------------------------------------------------------------------

Measure::View* MainWindow::measureView(Measure::Type measureType)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
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

void MainWindow::appendMeasureView(int measureType, Measure::View* pView)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
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
	m_statusLoadSignals = new QProgressBar(pStatusBar);
	m_statusMeasureThreadInfo = new QLabel(pStatusBar);
	m_statusMeasureTimeout = new QProgressBar(pStatusBar);
	m_statusMeasureThreadState = new QLabel(pStatusBar);
	m_statusCalibratorCount = new QLabel(pStatusBar);
	m_statusConnectToConfigServer = new QLabel(pStatusBar);
	m_statusConnectToAppDataServer = new QLabel(pStatusBar);
	m_statusConnectToTuningServer = new QLabel(pStatusBar);

	m_statusLoadSignals->setTextVisible(false);
	m_statusLoadSignals->setRange(0, 100);
	m_statusLoadSignals->setFixedWidth(100);
	m_statusLoadSignals->setFixedHeight(10);
	m_statusLoadSignals->setLayoutDirection(Qt::LeftToRight);

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
	pStatusBar->addWidget(m_statusLoadSignals);
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
	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		Measure::View* pView = measureView(static_cast<Measure::Type>(measureType));
		if (pView == nullptr)
		{
			continue;
		}

		pView->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(pView, &QTableView::customContextMenuRequested, this, &MainWindow::onContextMenu);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadOnToolBar_MeasureKind()
{
	if (m_pMeasureKindList == nullptr)
	{
		return;
	}

	int selectedItem = -1;
	int curerentMeasureKind = theOptions.toolBar().measureKind();

	m_measureKind = static_cast<Measure::Kind>(curerentMeasureKind);

	m_pMeasureKindList->blockSignals(true);

		m_pMeasureKindList->clear();

		for(int measureKind = 0; measureKind < Measure::KindCount; measureKind++)
		{
			if (measureKind == Measure::Kind::MultiRack)
			{
				if (theSignalBase.racks().groups().count() == 0)
				{
					continue;
				}
			}

			if (measureKind == curerentMeasureKind)
			{
				selectedItem = measureKind;
			}

			m_pMeasureKindList->addItem(qApp->translate("MeasureBase", Measure::KindCaption(measureKind).toUtf8()), measureKind);
		}

	m_pMeasureKindList->blockSignals(false);

	if (selectedItem == -1)
	{
		selectedItem = 0;
		m_measureKind = Measure::Kind::OneRack;
	}

	m_pMeasureKindList->setCurrentIndex(selectedItem);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadOnToolBar_Connection()
{
	if (m_pConnectionTypeList == nullptr)
	{
		return;
	}

	// get current connection type from otions
	//
	int currentConnectionType = theOptions.toolBar().connectionType();
	if (ERR_METROLOGY_CONNECTION_TYPE(currentConnectionType) == true)
	{
		currentConnectionType = Metrology::ConnectionType::Unused;
	}
	m_connectionType = static_cast<Metrology::ConnectionType>(currentConnectionType);


	// found all exist connections
	//
	QSet<int> metrologyConnectionSet;

	int connectionCount = theSignalBase.connections().count();
	for(int i = 0; i < connectionCount; i++)
	{
		int type = theSignalBase.connections().connection(i).type();
		if (ERR_METROLOGY_CONNECTION_TYPE(type) == true)
		{
			continue;
		}

		metrologyConnectionSet.insert(type);
	}


	// fill list of metrology connections
	//
	m_pConnectionTypeList->blockSignals(true);

		m_pConnectionTypeList->clear();

		// append Metrology::ConnectionType::Unused
		//
		m_pConnectionTypeList->addItem(qApp->translate(	"MetrologyConnection",
														Metrology::ConnectionTypeCaption(Metrology::ConnectionType::Unused).toUtf8()),
														Metrology::ConnectionType::Unused);
		// get selectedItem
		//
		int selectedItem = -1;

		QList<int> metrologyConnectionList = metrologyConnectionSet.values();
		std::sort(metrologyConnectionList.begin(), metrologyConnectionList.end());

		connectionCount = metrologyConnectionList.count();
		for(int index = 0; index < connectionCount; index++)
		{
			int connectionType = metrologyConnectionList.at(index);
			if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
			{
				continue;
			}

			if (connectionType == currentConnectionType)
			{
				selectedItem = index + 1;	// Metrology::ConnectionType::Unused item has already been added early, hence +1
			}

			m_pConnectionTypeList->addItem(qApp->translate("MetrologyConnection", Metrology::ConnectionTypeCaption(static_cast<Metrology::ConnectionType>(connectionType)).toUtf8()), connectionType);
		}

	m_pConnectionTypeList->blockSignals(false);

	//
	//
	if (selectedItem == -1)
	{
		selectedItem = 0;	// first element - this is Metrology::ConnectionType::Unused
		m_connectionType = Metrology::ConnectionType::Unused;
	}

	//
	//
	m_pConnectionTypeList->setCurrentIndex(selectedItem);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadOnToolBar_Racks()
{
	if (m_pRackCombo == nullptr || m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	m_pRackCombo->clear();
	m_pRackCombo->setEnabled(false);

	m_pSelectSignalWidget->clear();
	updatePrevNextSignalActions(0);

	if (ERR_MEASURE_KIND(m_measureKind) == true)
	{
		return;
	}

	if (ERR_METROLOGY_CONNECTION_TYPE(m_connectionType) == true)
	{
		return;
	}

	int rackCount = theSignalBase.createRackListForMeasure(m_measureKind, m_connectionType);
	if (rackCount == 0)
	{
		updateStartStopActions();
		return;
	}

	int currentRackIndex = 0;

	// fill racks or rackGroups on Tool bar
	//
	m_pRackCombo->blockSignals(true);

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
		m_pRackCombo->addItem(caption, rack.index());
	}

	m_pRackCombo->setCurrentIndex(-1);

	m_pRackCombo->blockSignals(false);

	m_pRackCombo->setCurrentIndex(currentRackIndex);
	m_pRackCombo->setEnabled(true);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::loadOnToolBar_Signals()
{
	if (m_pRackCombo == nullptr || m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	m_pSelectSignalWidget->clear();
	updatePrevNextSignalActions(0);

	if (ERR_MEASURE_KIND(m_measureKind) == true)
	{
		return;
	}

	if (ERR_METROLOGY_CONNECTION_TYPE(m_connectionType) == true)
	{
		return;
	}

	// get rackIndex or rackGroupIndex, it depend from measureKind
	//
	int rackIndex = m_pRackCombo->currentData().toInt();
	if (rackIndex == -1)
	{
		return;
	}

	// fill signal on Tool bar
	//
	std::vector<SelectSignalItem> signalList;

	int signalCount = theSignalBase.createSignalListForMeasure(m_measureKind, m_connectionType, rackIndex);
	if (signalCount == 0)
	{
		updateStartStopActions();
		return;
	}

	for(int index = 0; index < signalCount; index++)
	{
		const MeasureSignal& measureSignal = theSignalBase.signalForMeasure(index);
		if (measureSignal.isEmpty() == true)
		{
			continue;
		}

		SelectSignalItem signal(index, m_connectionType, measureSignal);
		if (signal.isValid() == false)
		{
			continue;
		}

		signalList.push_back(signal);
	}

	int currentSignalIndex = m_pSelectSignalWidget->setSignalList(signalList, theOptions.toolBar().defaultSignalId());
	setAcitiveMeasureSignal(currentSignalIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureType(int measureType)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	if (m_measureThread.isRunning() == true)
	{
		return;
	}

	m_measureType = static_cast<Measure::Type>(measureType);

	//
	//
	Measure::View* pView = measureView(m_measureType);
	if (pView == nullptr)
	{
		return;
	}

	switch(measureType)
	{
		case Measure::Type::Linearity:
		case Measure::Type::Comparators:

			m_pMeasureKindToolBar->show();
			m_pConnectionToolBar->show();
			m_pAnalogSignalToolBar->show();

			m_pSignalInfoPanel->show();
			m_pComparatorInfoPanel->show();

			break;

		default:
			assert(0);
			break;
	}

	//
	//
	emit measureViewChanged(pView);
	emit measureTypeChanged(m_measureType);
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
	if (m_calibratorBase.connectedCalibratorsCount() == 0)
	{
		if (showMsg == true)
		{
			QMessageBox::critical(this,
								  windowTitle(),
								  tr("Proccess of measure can not start, because no connected calibrators!\n"
									 "Please, make initialization calibrators!"));
		}

		return false;
	}

	if (m_connectionType == Metrology::ConnectionType::Tuning_Output)
	{
		if (tuningSocketIsConnected() == false)
		{
			if (showMsg == true)
			{
				QMessageBox::critical(this, windowTitle(), tr("No connect to Tuning Service!"));
			}

			return false;
		}
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::changeInputSignalOnInternal(const MeasureSignal& activeSignal)
{
	if (m_pConnectionTypeList == nullptr)
	{
		return false;
	}

	if (m_measureKind != Measure::Kind::OneRack)
	{
		return false;
	}

	if (activeSignal.isEmpty() == true)
	{
		return false;
	}

	Metrology::Signal* pInSignal = activeSignal.metrologySignal(Metrology::ConnectionIoType::Source, Metrology::Channel_0);
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		return false;
	}

	if (pInSignal->param().isInput() == false)
	{
		return false;
	}

	if (activeSignal.connectionType() != Metrology::ConnectionType::Unused)
	{
		return false;
	}

	int connectionIndex = theSignalBase.connections().findConnectionIndex(Metrology::ConnectionIoType::Source, pInSignal);
	if (connectionIndex == -1)
	{
		return false;
	}

	const Metrology::Connection& connection = theSignalBase.connections().connection(connectionIndex);
	if (connection.isValid() == false)
	{
		return false;
	}

	Metrology::Signal* pOutSignal = connection.metrologySignal(Metrology::ConnectionIoType::Destination);
	if (pOutSignal == nullptr || pOutSignal->param().isValid() == false)
	{
		return false;
	}

	if (pOutSignal->param().isInternal() == false)
	{
		return false;
	}

	m_pConnectionTypeList->setCurrentIndex(connection.type());

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::signalIsMeasured(const MeasureSignal& activeSignal, QString& signalID)
{
	MultiChannelSignal ioSignal;

	switch (m_connectionType)
	{
		case Metrology::ConnectionType::Unused:	ioSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source);	break;
		default:								ioSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination);	break;
	}

	if (ioSignal.isEmpty() == true)
	{
		return false;
	}

	bool isMeasured = false;

	int channelCount = activeSignal.channelCount();
	for(int ch = 0; ch < channelCount; ch++)
	{
		Metrology::Signal* pMetrologySignal = ioSignal.metrologySignal(ch);
		if (pMetrologySignal == nullptr || pMetrologySignal->param().isValid() == false)
		{
			continue;
		}

		StatisticsItem si;

		switch (m_measureType)
		{
			case Measure::Type::Linearity:
				{
					si.setSignal(pMetrologySignal);
				}
				break;

			case Measure::Type::Comparators:
				{
					si.setSignal(pMetrologySignal);

					int startComparatorIndex = theOptions.comparator().startComparatorIndex();
					if (startComparatorIndex < 0 || startComparatorIndex >= pMetrologySignal->param().comparatorCount())
					{
						continue;
					}

					si.setComparator(pMetrologySignal->param().comparator(startComparatorIndex));
				}

				break;

			default:
				assert(0);
				break;
		}

		m_measureBase.updateStatisticsItem(m_measureType, si);
		if (si.isMeasured() == true)
		{
			signalID.append(pMetrologySignal->param().appSignalID() + "\n");

			isMeasured = true;
		}
	}

	return isMeasured;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::inputsOfmoduleIsSame(const MeasureSignal& activeSignal)
{
	MultiChannelSignal ioSignal;

	switch (m_connectionType)
	{
		case Metrology::ConnectionType::Unused:
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			ioSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source);
			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
		case Metrology::ConnectionType::Tuning_Output:
			ioSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination);
			break;

		default:
			assert(0);
	}

	if (ioSignal.isEmpty() == true)
	{
		return false;
	}

	bool				eltalonIsFound = false;

	double				electricLowLimit = 0;
	double				electricHighLimit = 0;
	E::ElectricUnit		electricUnitID = E::ElectricUnit::NoUnit;
	E::SensorType		electricSensorType = E::SensorType::NoSensor;


	int channelCount = activeSignal.channelCount();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		Metrology::Signal* pSignal = ioSignal.metrologySignal(ch);
		if (pSignal == nullptr)
		{
			continue;
		}

		const Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (eltalonIsFound == false)
		{
			eltalonIsFound = true;

			electricLowLimit = param.electricLowLimit();
			electricHighLimit = param.electricHighLimit();
			electricUnitID = param.electricUnitID();
			electricSensorType = param.electricSensorType();
		}
		else
		{
			if (compareDouble(electricLowLimit, param.electricLowLimit()) == false ||
				compareDouble(electricHighLimit, param.electricHighLimit()) == false ||
				electricUnitID != param.electricUnitID() ||
				electricSensorType != param.electricSensorType())
			{
				return false;
			}
		}
	}

	if (eltalonIsFound == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int MainWindow::getMaxComparatorCount(const MeasureSignal& activeSignal)
{
	int maxComparatorCount = 0;

	int channelCount =  activeSignal.channelCount();
	for(int ch = 0; ch < channelCount; ch++)
	{
		Metrology::Signal* pSignal = nullptr;

		switch (activeSignal.connectionType())
		{
			case Metrology::ConnectionType::Unused:
				pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).metrologySignal(ch);
				break;
			default:
				pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination).metrologySignal(ch);
				break;
		}

		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		if (maxComparatorCount < pSignal->param().comparatorCount())
		{
			maxComparatorCount = pSignal->param().comparatorCount();
		}
	}

	return maxComparatorCount;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startMeasure()
{
	if (ERR_MEASURE_TYPE(m_measureType) == true)
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

	// source of signal
	//
	if (signalSourceIsValid(true) == false)
	{
		return;
	}

	const MeasureSignal& activeSignal = theSignalBase.activeSignal();
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	if (theOptions.module().measureInterInsteadIn() == true)
	{
		if (m_measureKind == Measure::Kind::OneRack)
		{
			if (changeInputSignalOnInternal(activeSignal) == true)
			{
				emit startMeasure();
				return;
			}
		}
	}

	if (m_measureType == Measure::Type::Comparators)
	{
		int comparatorCount = getMaxComparatorCount(activeSignal);
		if (comparatorCount == 0)
		{
			m_measureThread.stopMeasure(MeasureThreadInfo::ExitCode::Program);
			emit measureThreadStoped();
			return;
		}
	}

	// if we check in single module mode
	// all module inputs must be the same
	//
	if (m_measureKind == Measure::Kind::OneModule)
	{
		if (inputsOfmoduleIsSame(activeSignal) == false)
		{
			QMessageBox::critical(this,
								  windowTitle(),
								  tr("Unable to start the measurement process!\n"
									 "All electrical ranges of the inputs of the module must be the same."));
			return;
		}
	}

	// has this signal been measured before?
	//
	if (theOptions.module().warningIfMeasured() == true)
	{
		QString measuredSignals;

		if (signalIsMeasured(activeSignal, measuredSignals) == true)
		{

			int result = QMessageBox::question(this,
											  windowTitle(),
											  tr("Following signals were measured:\n\n%1\n"
												 "Do you want to measure them again?").
											  arg(measuredSignals));

			if (result == QMessageBox::No)
			{
				m_measureThread.stopMeasure(MeasureThreadInfo::ExitCode::Program);
				emit measureThreadStoped();
				return;
			}
		}
	}

	m_measureThread.setActiveSignalParam(theSignalBase.activeSignal(), m_calibratorBase);
	m_measureThread.setLinearityOption(theOptions.linearity());
	m_measureThread.setComparatorOption(theOptions.comparator());

	m_measureThread.start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopMeasure()
{
	if (m_measureThread.isFinished() == true)
	{
		return;
	}

	m_measureThread.stopMeasure(MeasureThreadInfo::ExitCode::Manual);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::exportMeasure()
{
	Measure::View* pMeasureView = activeMeasureView();
	if (pMeasureView == nullptr)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	QString fileName;

	switch (m_measureType)
	{
		case Measure::Type::Linearity:		fileName = "Linearity";		break;
		case Measure::Type::Comparators:	fileName = "Comparators";	break;
		default:							assert(0);
	}

	if (fileName.isEmpty() == true)
	{
		return;
	}

	ExportData* dialog = new ExportData(pMeasureView, false, fileName);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::copyMeasure()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->copy();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::removeMeasure()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->removeMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllMeasure()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showRackList()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	DialogRackList dialog(this);

	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &dialog, &DialogRackList::updateList, Qt::QueuedConnection);

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	theSignalBase.racks() = dialog.racks();
	if (theSignalBase.racks().groups().save() == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Attempt to save rack groups was unsuccessfully!"));
		return;
	}

	loadOnToolBar_MeasureKind();

	theSignalBase.initRackParam();

	if (m_measureKind == Measure::Kind::MultiRack)
	{
		loadOnToolBar_Racks();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showSignalList()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	DialogSignalList dialog(false, this);

	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &dialog, &DialogSignalList::updateList, Qt::QueuedConnection);

	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showComparatorsList()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	DialogComparatorList dialog(this);

	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &dialog, &DialogComparatorList::updateList, Qt::QueuedConnection);

	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showTuningSignalList()
{
	if (m_pConfigSocket == nullptr || m_pTuningSocket == nullptr)
	{
		return;
	}

	DialogTuningSignalList dialog(this);

	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &dialog, &DialogTuningSignalList::updateSignalList, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::sourcesLoaded, &dialog, &DialogTuningSignalList::updateSourceList, Qt::QueuedConnection);
	connect(m_pTuningSocket, &TuningSocket::socketDisconnected, &dialog, &DialogTuningSignalList::updateSourceList, Qt::QueuedConnection);

	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showConnectionList()
{
	if (m_pConfigSocket == nullptr)
	{
		return;
	}

	DialogMetrologyConnection dialog(this);

	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &dialog, &DialogMetrologyConnection::signalBaseLoaded, Qt::QueuedConnection);

	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	theSignalBase.connections() = dialog.metrologyConnections();

	// loadOnToolBar_Connection call loadOnToolBar_Racks() after setCurrentIndex
	//
	loadOnToolBar_Connection();

	theSignalBase.statistics().createSignalList();
	theSignalBase.statistics().createComparatorList();

	if (m_pStatisticsPanel == nullptr)
	{
		assert(m_pStatisticsPanel);
		return;
	}

	m_pStatisticsPanel->updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showGraphLinEl()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	emit pView->showGraph(MVG_TYPE_LIN_EL);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showGraphLinEn()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	emit pView->showGraph(MVG_TYPE_LIN_EN);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showGraph20El()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	emit pView->showGraph(MVG_TYPE_20VAL_EL);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showGraph20En()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	emit pView->showGraph(MVG_TYPE_20VAL_EN);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showStatistics()
{
	if (m_pStatisticsPanel == nullptr)
	{
		return;
	}

	m_pStatisticsPanel->show();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showCalibrators()
{
	m_calibratorBase.showInitDialog();
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

	DialogOptions* pDialog = new DialogOptions(theOptions, this);
	if (pDialog->exec() != QDialog::Accepted)
	{
		return;
	}

	theOptions = pDialog->options();
	theOptions.save();

	theDatabase.setDatabaseOption(theOptions.database());

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

	m_measureThread.setLinearityOption(theOptions.linearity());
	m_measureThread.setComparatorOption(theOptions.comparator());

	// update columns in the measure views
	//
	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		if (theOptions.measureView().updateColumnView(static_cast<Measure::Type>(measureType)) == false)
		{
			continue;
		}

		Measure::View* pView = measureView(static_cast<Measure::Type>(measureType));
		if (pView == nullptr)
		{
			continue;
		}

		pView->updateColumn();
	}

	m_pFindMeasurePanel->setViewFont(theOptions.measureView().font());
	m_pStatisticsPanel->setViewFont(theOptions.measureView().font());

	// update panels
	//
	if (m_pSignalInfoPanel != nullptr)
	{
		m_pSignalInfoPanel->setSignalInfo(theOptions.signalInfo());
	}

	if (m_pComparatorInfoPanel != nullptr)
	{
		m_pComparatorInfoPanel->setComparatorInfo(theOptions.comparatorInfo());
	}


	// if changed error type or limitType
	//
	if (	options.linearity().errorType() != theOptions.linearity().errorType() ||
			options.linearity().limitType() != theOptions.linearity().limitType() ||
			options.comparator().errorType() != theOptions.comparator().errorType() ||
			options.comparator().limitType() != theOptions.comparator().limitType())
	{
		m_pStatisticsPanel->updateList();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutConnection()
{
	DialogProjectProperty dialog(theOptions.projectInfo(), this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::aboutApp()
{
	DialogAbout::show(this, QString(), ":/Images/logo.png");
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureTimeout(QString value)
{
	m_measureTimeout = static_cast<int>(value.toDouble() * 1000);

	//
	//

	theOptions.toolBar().setMeasureTimeout(m_measureTimeout);
	theOptions.toolBar().save();

	emit measureTimeoutChanged(m_measureTimeout);

	//
	//

	if (m_statusMeasureTimeout == nullptr)
	{
		return;
	}

	if (m_measureTimeout == 0)
	{
		m_statusMeasureTimeout->hide();
	}
	else
	{
		if (m_measureThread.isRunning() == true)
		{
			m_statusMeasureTimeout->show();
			m_statusMeasureTimeout->setRange(0, m_measureTimeout);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureKind(int index)
{
	if (m_pMeasureKindList == nullptr)
	{
		return;
	}

	int measureKind = m_pMeasureKindList->itemData(index).toInt();
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	if (measureKind == Measure::Kind::MultiRack)
	{
		if (theSignalBase.racks().groups().count() == 0)
		{
			m_pMeasureKindList->blockSignals(true);
			m_pMeasureKindList->setCurrentIndex(0);
			m_pMeasureKindList->blockSignals(false);

			QMessageBox::information(this,
									 windowTitle(),
									 tr( "For measurements in several racks simultaneously, "
										 "you need to combine several racks into groups."
										 "Currently, no groups have been found.\n"
										 "To create a group of racks, click menu \"View\" - \"Racks ...\" ."));
			return;
		}
	}

	//
	//
	m_measureKind = static_cast<Measure::Kind>(measureKind);

	//
	//
	theOptions.toolBar().setMeasureKind(measureKind);
	theOptions.toolBar().save();

	emit measureKindChanged(measureKind);

	//
	//
	loadOnToolBar_Racks();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setConnectionType(int index)
{
	if (m_pConnectionTypeList == nullptr)
	{
		assert(m_pConnectionTypeList);
		return;
	}

	if (index == -1)
	{
		return;
	}

	int connectionType = m_pConnectionTypeList->itemData(index).toInt();
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	//
	//
	theOptions.toolBar().setConnectionType(connectionType);
	theOptions.toolBar().save();

	//
	//
	m_connectionType = static_cast<Metrology::ConnectionType>(connectionType);

	emit connectionTypeChanged(connectionType);

	//
	//
	loadOnToolBar_Racks();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setConnectionTypeFromStatistic(int connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}
	if (m_pConnectionTypeList == nullptr)
	{
		assert(m_pConnectionTypeList);
		return;
	}

	int connectionIndex = -1;

	int count = m_pConnectionTypeList->count();
	for(int i = 0; i < count; i++)
	{
		if (m_pConnectionTypeList->itemData(i).toInt() == connectionType)
		{
			connectionIndex = i;
			break;
		}
	}

	if (connectionIndex == -1)
	{
		connectionIndex = 0;
	}

	m_pConnectionTypeList->setCurrentIndex(connectionIndex);
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
	loadOnToolBar_Signals();

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
	const MultiChannelSignal& signal = measureSignal.multiChannelSignal(Metrology::ConnectionIoType::Source);
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
	if (m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	int currentSignalIndex = m_pSelectSignalWidget->currentSignalIndex();
	if (currentSignalIndex < 0 || currentSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentSignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return;
	}

	int previousSignalIndex = currentSignalIndex - 1;
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
	if (m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	int currentSignalIndex = m_pSelectSignalWidget->currentSignalIndex();
	if (currentSignalIndex < 0 || currentSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentSignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return;
	}

	int nextSignalIndex = currentSignalIndex + 1;
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

bool MainWindow::setNextMeasureSignalFromModule()
{
	if (m_pSelectSignalWidget == nullptr)
	{
		return false;
	}

	int currentSignalIndex = m_pSelectSignalWidget->currentSignalIndex();
	if (currentSignalIndex < 0 || currentSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return false;
	}

	MeasureSignal currentActiveSignal = theSignalBase.signalForMeasure(currentSignalIndex);
	if (currentActiveSignal.isEmpty() == true)
	{
		return false;
	}

	int nextSignalIndex = currentSignalIndex + 1;
	if (nextSignalIndex < 0 || nextSignalIndex >= theSignalBase.signalForMeasureCount())
	{
		return false;
	}

	const MeasureSignal& nextActiveSignal = theSignalBase.signalForMeasure(nextSignalIndex);
	if (nextActiveSignal.isEmpty() == true)
	{
		return false;
	}

	// if module numbers not equal then disabling selection of next input
	//
	if (	currentActiveSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).location().chassis() != nextActiveSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).location().chassis() ||
			currentActiveSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).location().module() != nextActiveSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).location().module())
	{
		return false;
	}

	setAcitiveMeasureSignal(nextSignalIndex);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::changeActiveDestSignal(int channel, Metrology::Signal* pDestSignal)
{
	if (m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	int index = m_pSelectSignalWidget->currentSignalIndex();
	if(index < 0 || index > theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal measureSignal = theSignalBase.signalForMeasure(index);
	if (measureSignal.isEmpty() == true)
	{
		return;
	}

	MultiChannelSignal multiChannelSignal = measureSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination);
	if (multiChannelSignal.isEmpty() == true)
	{
		return;
	}

	if (ERR_MEASURE_KIND(m_measureKind) == true)
	{
		return;
	}

	if (channel < 0 || channel >= measureSignal.channelCount())
	{
		return;
	}

	if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
	{
		return;
	}

	// change
	//
	multiChannelSignal.setMetrologySignal(m_measureKind, channel, pDestSignal);

	measureSignal.setMultiSignal(Metrology::ConnectionIoType::Destination, multiChannelSignal);

	// set
	//
	bool result = theSignalBase.setSignalForMeasure(index, measureSignal);
	if (result == false)
	{
		return;
	}

	m_pSelectSignalWidget->updateActiveOutputSignal(measureSignal);

	setAcitiveMeasureSignal(index);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::changeActiveDestSignals(int channelPrev, int channelNext)
{
	if (m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	int index = m_pSelectSignalWidget->currentSignalIndex();
	if(index < 0 || index > theSignalBase.signalForMeasureCount())
	{
		return;
	}

	MeasureSignal measureSignal = theSignalBase.signalForMeasure(index);
	if (measureSignal.isEmpty() == true)
	{
		return;
	}

	MultiChannelSignal multiChannelSignal = measureSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination);
	if (multiChannelSignal.isEmpty() == true)
	{
		return;
	}

	if (ERR_MEASURE_KIND(m_measureKind) == true)
	{
		return;
	}

	// get output signals
	//
	if (channelPrev < 0 || channelPrev >= measureSignal.channelCount())
	{
		return;
	}

	Metrology::Signal* pOutputSignalPrev = multiChannelSignal.metrologySignal(channelPrev);
	if (pOutputSignalPrev == nullptr || pOutputSignalPrev->param().isValid() == false)
	{
		return;
	}

	if (channelNext < 0 || channelNext >= measureSignal.channelCount())
	{
		return;
	}

	Metrology::Signal* pOutputSignalNext = multiChannelSignal.metrologySignal(channelNext);
	if (pOutputSignalNext == nullptr || pOutputSignalNext->param().isValid() == false)
	{
		return;
	}

	// change
	//
	multiChannelSignal.setMetrologySignal(m_measureKind, channelPrev, pOutputSignalNext);
	multiChannelSignal.setMetrologySignal(m_measureKind, channelNext, pOutputSignalPrev);

	measureSignal.setMultiSignal(Metrology::ConnectionIoType::Destination, multiChannelSignal);

	// set
	//
	bool result = theSignalBase.setSignalForMeasure(index, measureSignal);
	if (result == false)
	{
		return;
	}

	m_pSelectSignalWidget->updateActiveOutputSignal(measureSignal);

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
	if (m_statusCalibratorCount == nullptr)
	{
		return;
	}

	m_statusCalibratorCount->setText(tr(" Connected calibrators: %1 ").arg(count));

	if (count == 0)
	{
		m_statusCalibratorCount->setStyleSheet("background-color: rgb(255, 160, 160);");
		m_statusCalibratorCount->setToolTip(tr("Please, connect Calibrators\nclick menu \"Tool\" - \"Calibrators...\""));
	}
	else
	{
		m_statusCalibratorCount->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");

		QString calibratorInfo;

		int calibratorCount = m_calibratorBase.calibratorCount();
		for(int i  = 0; i < calibratorCount; i++)
		{
			if (m_calibratorBase.calibratorManager(i) == nullptr)
			{
				continue;
			}

			Calibrator* pCalibrator = m_calibratorBase.calibratorManager(i)->calibrator();
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
		                        .arg(File::METROLOGY_ITEMS_XML)
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
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");
	m_statusConnectToConfigServer->setToolTip(configSocketConnectedStateStr());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketDisconnected()
{
	if (m_statusConnectToConfigServer == nullptr || m_statusLoadSignals == nullptr)
	{
		return;
	}

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: off "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(255, 160, 160);");
	m_statusConnectToConfigServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));

	m_statusLoadSignals->hide();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketUnknownClient()
{
	QMessageBox::critical(this,
						  windowTitle(),
						  tr("Configuration Service does not recognize EquipmentID \"%1\" for software \"Metrology\"")
						  .arg(theOptions.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY)));
	return;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketConfigurationLoaded()
{
	runSignalSocket();
	runTuningSocket();

	//
	//
	if (m_statusConnectToConfigServer == nullptr || m_statusLoadSignals == nullptr)
	{
		return;
	}

	m_statusConnectToConfigServer->setText(tr(" ConfigurationService: on "));
	m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");
	m_statusConnectToConfigServer->setToolTip(configSocketConnectedStateStr());


	m_statusLoadSignals->show();
	m_statusLoadSignals->setValue(0);

	if (m_pConfigSocket == nullptr)
	{
		assert(m_pConfigSocket);
		return;
	}

	if (m_pConfigSocket->loadedFiles().count() == 0)
	{
		QMessageBox::critical(this, windowTitle(), tr("No loaded files from Configuration Service!"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketSignalBaseLoading(int persentage)
{
	if (m_statusLoadSignals == nullptr)
	{
		return;
	}

	m_statusLoadSignals->setValue(persentage);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::configSocketSignalBaseLoaded()
{
	// loadOnToolBar_Connection call loadOnToolBar_Racks() after setCurrentIndex
	// if m_connectionType != Metrology::ConnectionType::Unused
	//
	loadOnToolBar_Connection();

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		loadOnToolBar_Racks();
	}

	//
	//
	if (m_statusConnectToConfigServer == nullptr || m_statusLoadSignals == nullptr)
	{
		return;
	}

	if (theSignalBase.signalCount() == 0)
	{
		m_statusConnectToConfigServer->setStyleSheet("background-color: rgb(255, 255, 160);");
	}

	m_statusLoadSignals->hide();
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
	m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");
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
		m_measureThread.stopMeasure(MeasureThreadInfo::ExitCode::Program);
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
	m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");
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
		if (m_connectionType == Metrology::ConnectionType::Tuning_Output)
		{
			m_measureThread.stopMeasure(MeasureThreadInfo::ExitCode::Program);
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
		m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");

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
	m_pMeasureKindToolBar->setDisabled(true);
	m_pConnectionToolBar->setDisabled(true);
	m_pAnalogSignalToolBar->setDisabled(true);

	m_statusMeasureThreadInfo->setText(QString());
	m_statusMeasureThreadInfo->show();

	if (m_measureTimeout != 0)
	{
		m_statusMeasureTimeout->show();
		m_statusMeasureTimeout->setRange(0, m_measureTimeout);
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
	m_pMeasureKindToolBar->setEnabled(true);
	m_pConnectionToolBar->setEnabled(true);
	m_pAnalogSignalToolBar->setEnabled(true);

	m_statusMeasureThreadInfo->setText(QString());
	m_statusMeasureThreadInfo->hide();

	m_statusMeasureTimeout->hide();
	m_statusMeasureTimeout->setRange(0, m_measureTimeout);
	m_statusMeasureTimeout->setValue(0);

	m_statusMeasureThreadState->setText(tr(" Measure process is stopped "));
	m_statusMeasureThreadState->setStyleSheet("background-color: rgb(0x0, 0x0, 0x0);");

	//
	//
	const MeasureSignal& activeSignal = theSignalBase.activeSignal();
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	if (m_measureThread.info().exitCode() == MeasureThreadInfo::ExitCode::Manual)
	{
		return;
	}

	//
	//
	if (theOptions.module().measureLinAndCmp() == true)
	{
		switch (m_measureType)
		{
			case Measure::Type::Linearity:
				{
					int comparatorCount = getMaxComparatorCount(activeSignal);
					if (comparatorCount == 0)
					{
						break;
					}

					m_pMainTab->setCurrentIndex(Measure::Type::Comparators);

					emit startMeasure();
					return;
				}
			case Measure::Type::Comparators:
				{
					m_pMainTab->setCurrentIndex(Measure::Type::Linearity);
				}
				break;
			default:
				assert(0);
				break;
		}
	}

	//
	//
	if (theOptions.module().measureEntireModule() == true)
	{
		bool signalIsSelected = setNextMeasureSignalFromModule();
		if (signalIsSelected == true)
		{
			emit startMeasure();
			return;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadInfo(const MeasureThreadInfo& info)
{
	switch (info.type())
	{
		case MeasureThreadInfo::msgType::String:

			if (m_statusMeasureThreadInfo == nullptr)
			{
				break;
			}

			m_statusMeasureThreadInfo->setText(info.message());
			m_statusMeasureThreadInfo->setStyleSheet("color: rgb(0x0, 0x0, 0x0);");

			break;

		case MeasureThreadInfo::msgType::StringError:

			if (m_statusMeasureThreadInfo == nullptr)
			{
				break;
			}

			m_statusMeasureThreadInfo->setText(info.message());
			m_statusMeasureThreadInfo->setStyleSheet("color: rgb(255, 0, 0);");

			break;

		case MeasureThreadInfo::msgType::Timeout:

			if (m_statusMeasureTimeout == nullptr)
			{
				break;
			}

			m_statusMeasureTimeout->setValue(info.timeout());
	}
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

		case QMessageBox::Critical:

			QMessageBox::critical(this, windowTitle(), text);

			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureComplite(Measure::Item* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(pMeasurement->measureType()) == true)
	{
		return;
	}

	emit appendMeasure(pMeasurement);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateMeasureView()
{
	Measure::View* pView = activeMeasureView();
	if (pView == nullptr)
	{
		return;
	}

	pView->updateColumn();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateStartStopActions()
{
	if (m_pStartMeasureAction == nullptr || m_pStopMeasureAction == nullptr)
	{
		return;
	}

	m_pStartMeasureAction->setEnabled(false);
	m_pStopMeasureAction->setEnabled(false);

	if (ERR_MEASURE_TYPE(m_measureType) == true)
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

	if (theSignalBase.rackForMeasureCount() == 0 || theSignalBase.signalForMeasureCount() == 0)
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
	if(m_pSelectSignalWidget == nullptr)
	{
		return;
	}

	if (m_pPreviousSignalAction == nullptr || m_pNextSignalAction == nullptr)
	{
		return;
	}

	if (m_pSelectSignalWidget->count() == 0)
	{
		m_pSelectSignalWidget->setEnabled(false);
		m_pPreviousSignalAction->setEnabled(false);
		m_pNextSignalAction->setEnabled(false);

		return;
	}

	m_pSelectSignalWidget->setEnabled(true);
	m_pPreviousSignalAction->setEnabled(true);
	m_pNextSignalAction->setEnabled(true);

	if (signalIndex == 0)
	{
		m_pPreviousSignalAction->setEnabled(false);
	}

	if (signalIndex == m_pSelectSignalWidget->count() - 1)
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

	connect(m_pConfigSocket, &ConfigSocket::unknownClient, this, &MainWindow::configSocketUnknownClient, Qt::QueuedConnection);

	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &MainWindow::configSocketConfigurationLoaded, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoading, this, &MainWindow::configSocketSignalBaseLoading, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, this, &MainWindow::configSocketSignalBaseLoaded, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &theSignalBase.statistics(), &StatisticsBase::signalBaseLoaded, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, m_pStatisticsPanel, &PanelStatistics::updateList, Qt::QueuedConnection);
	connect(m_pConfigSocket, &ConfigSocket::signalBaseLoaded, &m_measureBase, &Measure::Base::signalBaseLoaded, Qt::QueuedConnection);

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
	connect(&m_measureThread, &MeasureThread::sendMeasureInfo, this, &MainWindow::measureThreadInfo, Qt::QueuedConnection);
	connect(&m_measureThread, &MeasureThread::msgBox, this, &MainWindow::measureThreadMsgBox, Qt::BlockingQueuedConnection);
	connect(&m_measureThread, &MeasureThread::measureComplite, this, &MainWindow::measureComplite, Qt::QueuedConnection);

	m_measureThread.measureTypeChanged(m_measureType);
	m_measureThread.measureTimeoutChanged(m_measureTimeout);
	m_measureThread.measureKindChanged(m_measureKind);
	m_measureThread.connectionTypeChanged(m_connectionType);

	connect(this, &MainWindow::measureTypeChanged, &m_measureThread, &MeasureThread::measureTypeChanged, Qt::QueuedConnection);
	connect(this, &MainWindow::measureTimeoutChanged, &m_measureThread, &MeasureThread::measureTimeoutChanged, Qt::QueuedConnection);
	connect(this, &MainWindow::measureKindChanged, &m_measureThread, &MeasureThread::measureKindChanged, Qt::QueuedConnection);
	connect(this, &MainWindow::connectionTypeChanged, &m_measureThread, &MeasureThread::connectionTypeChanged, Qt::QueuedConnection);

	connect(&theSignalBase, &SignalBase::signalParamChanged, &m_measureThread, &MeasureThread::signalParamChanged, Qt::QueuedConnection);

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

void MainWindow::showFindMeasurePanel(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		return;
	}

	if (m_pFindMeasurePanel == nullptr)
	{
		return;
	}

	m_pFindMeasurePanel->show();
	m_pFindMeasurePanel->setFindText(appSignalID);
	emit m_pFindMeasurePanel->find();
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

	theSignalBase.clear();
	theOptions.linearity().points().save();
	m_measureBase.clear();
	m_calibratorBase.clear();

	theDatabase.close();

	saveSettings();

	QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
