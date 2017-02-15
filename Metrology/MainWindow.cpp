#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QCloseEvent>

#include "OptionsDialog.h"
#include "CalibratorBase.h"
#include "Database.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"
#include "ReportView.h"
#include "ExportData.h"
#include "SignalList.h"
#include "TuningSignalList.h"
#include "OutputSignal.h"
#include "Statistic.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // init calibration base
    //
    theCalibratorBase.init(this);
    connect(&theCalibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::calibratorConnectedChanged, Qt::QueuedConnection);

    connect(&theSignalBase, &SignalBase::updatedSignalParam, &theTuningSignalBase, &TuningSignalBase::updateSignalParam, Qt::QueuedConnection);

    // load output signals base
    //
    theOutputSignalBase.load();

    // init interface
    //
    createInterface();

    // init measure thread
    //
    connect(&m_measureThread, &MeasureThread::started, this, &MainWindow::measureThreadStarted, Qt::QueuedConnection);
    connect(&m_measureThread, &MeasureThread::finished, this, &MainWindow::measureThreadStoped, Qt::QueuedConnection);
    connect(&m_measureThread, static_cast<void (MeasureThread::*)(QString)>(&MeasureThread::measureInfo), this, static_cast<void (MainWindow::*)(QString)>(&MainWindow::setMeasureThreadInfo), Qt::QueuedConnection);
    connect(&m_measureThread, static_cast<void (MeasureThread::*)(int)>(&MeasureThread::measureInfo), this, static_cast<void (MainWindow::*)(int)>(&MainWindow::setMeasureThreadInfo), Qt::QueuedConnection);
    connect(&m_measureThread, &MeasureThread::measureComplite, this, &MainWindow::measureComplite, Qt::QueuedConnection);

    m_measureThread.init(this);

    // init signal socket thread
    //
    HostAddressPort signalSocketAddress(theOptions.signalSocket().serverIP(), theOptions.signalSocket().serverPort());
    m_pSignalSocket = new SignalSocket(signalSocketAddress);
    m_pSignalSocketThread = new SimpleThread(m_pSignalSocket);
    m_pSignalSocketThread->start();

    connect(m_pSignalSocket, &SignalSocket::signalsLoaded, this, &MainWindow::signalSocketSignalsLoaded, Qt::QueuedConnection);
    connect(m_pSignalSocket, &SignalSocket::socketConnected, this, &MainWindow::signalSocketConnected, Qt::QueuedConnection);
    connect(m_pSignalSocket, &SignalSocket::socketDisconnected, this, &MainWindow::signalSocketDisconnected, Qt::QueuedConnection);
    connect(m_pSignalSocket, &SignalSocket::socketDisconnected, &m_measureThread, &MeasureThread::signalSocketDisconnected, Qt::QueuedConnection);

    // init tuning socket thread
    //
    HostAddressPort tuningSocketAddress(theOptions.tuningSocket().serverIP(), theOptions.tuningSocket().serverPort());
    m_pTuningSocket = new TuningSocket(tuningSocketAddress);
    m_pTuningSocketThread = new SimpleThread(m_pTuningSocket);
    m_pTuningSocketThread->start();

    connect(m_pTuningSocket, &TuningSocket::socketConnected, this, &MainWindow::tuningSocketConnected, Qt::QueuedConnection);
    connect(m_pTuningSocket, &TuningSocket::socketDisconnected, this, &MainWindow::tuningSocketDisconnected, Qt::QueuedConnection);
    connect(m_pTuningSocket, &TuningSocket::socketDisconnected, &m_measureThread, &MeasureThread::tuningSocketDisconnected, Qt::QueuedConnection);

    measureThreadStoped();

    m_pCalculator = new Calculator(this);
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
    setWindowIcon(QIcon(":/icons/Metrology.ico"));
    setWindowTitle(tr("Metrology"));
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    createActions();
    createMenu();
    createToolBars();
    createMeasurePages();
    createPanels();
    createStatusBar();
    createContextMenu();

    loadSettings();

    setMeasureType(MEASURE_TYPE_LINEARITY);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void  MainWindow::createActions()
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

    m_pPrintMeasureAction = new QAction(tr("&Print ..."), this);
    m_pPrintMeasureAction->setShortcut(Qt::CTRL + Qt::Key_P);
    m_pPrintMeasureAction->setIcon(QIcon(":/icons/Print.png"));
    m_pPrintMeasureAction->setToolTip(tr("Printing of the measurements"));
    connect(m_pPrintMeasureAction, &QAction::triggered, this, &MainWindow::printMeasure);

    m_pExportMeasureAction = new QAction(tr("&Export ..."), this);
    m_pExportMeasureAction->setShortcut(Qt::CTRL + Qt::Key_E);
    m_pExportMeasureAction->setIcon(QIcon(":/icons/Export.png"));
    m_pExportMeasureAction->setToolTip(tr("Export measurements"));
    connect(m_pExportMeasureAction, &QAction::triggered, this, &MainWindow::exportMeasure);

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
    m_pShowReportsAction = new QAction(tr("&Reports ..."), this);
    m_pShowReportsAction->setShortcut(Qt::CTRL + Qt::Key_R);
    m_pShowReportsAction->setIcon(QIcon(":/icons/Reports.png"));
    m_pShowReportsAction->setToolTip(tr("Preview the report on the measurements"));
    connect(m_pShowReportsAction, &QAction::triggered, this, &MainWindow::showReports);

    m_pShowCalculatorAction = new QAction(tr("Metrological &calculator ..."), this);
    m_pShowCalculatorAction->setShortcut(Qt::ALT + Qt::Key_C);
    m_pShowCalculatorAction->setIcon(QIcon(":/icons/Calculator.png"));
    m_pShowCalculatorAction->setToolTip(tr("Calculator for converting metrological quantities"));
    connect(m_pShowCalculatorAction, &QAction::triggered, this, &MainWindow::showCalculator);

    // Tools
    //
    m_pCalibratorsAction = new QAction(tr("&Calibrations ..."), this);
    m_pCalibratorsAction->setIcon(QIcon(":/icons/Calibrators.png"));
    m_pCalibratorsAction->setToolTip(tr("Connecting and configuring calibrators"));
    connect(m_pCalibratorsAction, &QAction::triggered, this, &MainWindow::calibrators);

    m_pShowSignalListAction = new QAction(tr("&Signals ..."), this);
    m_pShowSignalListAction->setIcon(QIcon(":/icons/Signals.png"));
    m_pShowSignalListAction->setToolTip("");
    connect(m_pShowSignalListAction, &QAction::triggered, this, &MainWindow::showSignalList);

    m_pShowComparatorsListAction = new QAction(tr("&Comparators ..."), this);
    m_pShowComparatorsListAction->setIcon(QIcon(":/icons/Comparator.png"));
    m_pShowComparatorsListAction->setToolTip("");
    connect(m_pShowComparatorsListAction, &QAction::triggered, this, &MainWindow::showComparatorsList);

    m_pShowOutputSignalListAction = new QAction(tr("Output signals ..."), this);
    m_pShowOutputSignalListAction->setIcon(QIcon(":/icons/InOut.png"));
    m_pShowOutputSignalListAction->setToolTip("");
    connect(m_pShowOutputSignalListAction, &QAction::triggered, this, &MainWindow::showOutputSignalList);

    m_pShowTuningSignalListAction = new QAction(tr("Tuning signals ..."), this);
    m_pShowTuningSignalListAction->setIcon(QIcon(":/icons/InOut.png"));
    m_pShowTuningSignalListAction->setToolTip("");
    connect(m_pShowTuningSignalListAction, &QAction::triggered, this, &MainWindow::showTuningSignalList);

    m_pOptionsAction = new QAction(tr("&Options ..."), this);
    m_pOptionsAction->setShortcut(Qt::CTRL + Qt::Key_O);
    m_pOptionsAction->setIcon(QIcon(":/icons/Options.png"));
    m_pOptionsAction->setToolTip(tr("Editing application settings"));
    connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::options);

    // ?
    //
    m_pShowStatisticAction = new QAction(tr("Sta&tistics ..."), this);
    m_pShowStatisticAction->setIcon(QIcon(":/icons/Statistics.png"));
    m_pShowStatisticAction->setToolTip("");
    connect(m_pShowStatisticAction, &QAction::triggered, this, &MainWindow::showStatistic);

    m_pAboutConnectionAction = new QAction(tr("About connect to server ..."), this);
    m_pAboutConnectionAction->setIcon(QIcon(":/icons/About connection.png"));
    m_pAboutConnectionAction->setToolTip("");
    connect(m_pAboutConnectionAction, &QAction::triggered, this, &MainWindow::aboutConnection);

    m_pAboutAppAction = new QAction(tr("About Metrology ..."), this);
    m_pAboutAppAction->setIcon(QIcon(":/icons/About Application.png"));
    m_pAboutAppAction->setToolTip("");
    connect(m_pAboutAppAction, &QAction::triggered, this, &MainWindow::aboutApp);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateStartStopActions()
{
    m_pStartMeasureAction->setEnabled(true);
    m_pStopMeasureAction->setEnabled(true);

    return;

    bool startMeasure = true;
    bool stopMeasure = true;

    if (m_measureThread.isRunning() == true)
    {
        startMeasure = false;
    }
    else
    {
        stopMeasure = false;
    }

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            if (theCalibratorBase.connectedCalibratorsCount() == 0)
            {
               startMeasure = false;
            }

            break;

        default:

            startMeasure = false;

            break;
    }

    m_pStartMeasureAction->setEnabled(startMeasure);
    m_pStopMeasureAction->setEnabled(stopMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
    QMenuBar* pMenuBar = menuBar();
    if (pMenuBar == nullptr)
    {
        return;
    }

    m_pMeasureMenu = pMenuBar->addMenu(tr("&Measure"));

    m_pMeasureMenu->addAction(m_pStartMeasureAction);
    m_pMeasureMenu->addAction(m_pStopMeasureAction);
    m_pMeasureMenu->addSeparator();
    m_pMeasureMenu->addAction(m_pPrintMeasureAction);
    m_pMeasureMenu->addAction(m_pExportMeasureAction);

    m_pEditMenu = pMenuBar->addMenu(tr("&Edit"));

    m_pEditMenu->addAction(m_pCopyMeasureAction);
    m_pEditMenu->addAction(m_pRemoveMeasureAction);
    m_pEditMenu->addSeparator();
    m_pEditMenu->addAction(m_pSelectAllMeasureAction);
    m_pEditMenu->addSeparator();

    m_pViewMenu = pMenuBar->addMenu(tr("&View"));

    m_pViewPanelMenu = new QMenu("&Panels", m_pViewMenu);
    m_pViewMenu->addMenu(m_pViewPanelMenu);
    m_pViewMenu->addAction(m_pShowReportsAction);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addAction(m_pShowCalculatorAction);


    m_pSettingMenu = pMenuBar->addMenu(tr("&Tools"));

    m_pSettingMenu->addAction(m_pCalibratorsAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pShowSignalListAction);
    m_pSettingMenu->addAction(m_pShowComparatorsListAction);
    m_pSettingMenu->addAction(m_pShowOutputSignalListAction);
    m_pSettingMenu->addAction(m_pShowTuningSignalListAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pOptionsAction);


    m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

    m_pInfoMenu->addAction(m_pShowStatisticAction);
    m_pInfoMenu->addSeparator();
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
        m_pMeasureControlToolBar->addAction(m_pPrintMeasureAction);
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
        QRegExp rx( "^[0-9]*[.]{1}[0-9]*$" );
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
            measureTimeoutList->addItem(QString::number(MeasureTimeout[t], 10, 1));
        }

        measureTimeoutList->setCurrentText(QString::number(double(theOptions.toolBar().measureTimeout()) / 1000, 10, 1));

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
        QComboBox* measureKindList = new QComboBox(m_pMeasureKind);

        m_pMeasureKind->addWidget(measureKindLabel);
        m_pMeasureKind->addWidget(measureKindList);

        measureKindLabel->setText(tr(" Measure kind "));
        measureKindLabel->setEnabled(false);

        for(int k = 0; k < MEASURE_KIND_COUNT; k++)
        {
            measureKindList->addItem(MeasureKind[k]);
        }

        measureKindList->setCurrentIndex(theOptions.toolBar().measureKind());

        connect(measureKindList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setMeasureKind);
    }


    // Control panel output signals
    //
    m_pOutputSignalToolBar = new QToolBar(this);
    if (m_pOutputSignalToolBar != nullptr)
    {
        m_pOutputSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pOutputSignalToolBar->setWindowTitle(tr("Control panel output signals"));
        m_pOutputSignalToolBar->setObjectName(m_pOutputSignalToolBar->windowTitle());
        addToolBarBreak(Qt::RightToolBarArea);
        addToolBar(m_pOutputSignalToolBar);

        QLabel* outputSignalLabel = new QLabel(m_pOutputSignalToolBar);
        m_outputSignalTypeList = new QComboBox(m_pOutputSignalToolBar);

        m_pOutputSignalToolBar->addWidget(outputSignalLabel);
        outputSignalLabel->setText(tr(" Output signals "));
        outputSignalLabel->setEnabled(false);

        m_pOutputSignalToolBar->addWidget(m_outputSignalTypeList);

        for(int s = 0; s < OUTPUT_SIGNAL_TYPE_COUNT; s++)
        {
            m_outputSignalTypeList->addItem(OutputSignalType[s]);
        }

        m_outputSignalTypeList->setCurrentIndex(theOptions.toolBar().outputSignalType());

        connect(m_outputSignalTypeList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setOutputSignalType);
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

        QLabel* asCaseLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asCaseLabel);
        asCaseLabel->setText(tr(" Case "));
        asCaseLabel->setEnabled(false);

        m_asCaseTypeCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asCaseTypeCombo);
        m_asCaseTypeCombo->setEnabled(false);
        m_asCaseTypeCombo->setFixedWidth(100);
        connect(m_asCaseTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setCaseType);

        QLabel* asSignalLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSignalLabel);
        asSignalLabel->setText(tr(" Signal "));
        asSignalLabel->setEnabled(false);

        m_asSignalCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asSignalCombo);
        m_asSignalCombo->setEnabled(false);
        m_asSignalCombo->setFixedWidth(250);
        connect(m_asSignalCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setMeasureSignal);

        m_pAnalogSignalToolBar->addSeparator();

        QLabel* asChannelLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asChannelLabel);
        asChannelLabel->setText(tr(" Case No "));
        asChannelLabel->setEnabled(false);

        m_asCaseNoCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asCaseNoCombo);
        m_asCaseNoCombo->setEnabled(false);
        m_asCaseNoCombo->setFixedWidth(60);
        connect(m_asCaseNoCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setCaseNo);

        QLabel* asSubblockLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSubblockLabel);
        asSubblockLabel->setText(tr(" Subblock "));
        asSubblockLabel->setEnabled(false);

        m_asSubblockCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asSubblockCombo);
        m_asSubblockCombo->setEnabled(false);
        m_asSubblockCombo->setFixedWidth(60);
        connect(m_asSubblockCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setSubblock);

        QLabel* asBlockLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asBlockLabel);
        asBlockLabel->setText(tr(" Block "));
        asBlockLabel->setEnabled(false);

        m_asBlockCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asBlockCombo);
        m_asBlockCombo->setEnabled(false);
        m_asBlockCombo->setFixedWidth(60);
        connect(m_asBlockCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setBlock);

        QLabel* asEntryLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asEntryLabel);
        asEntryLabel->setText(tr(" Entry "));
        asEntryLabel->setEnabled(false);

        m_asEntryCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(m_asEntryCombo);
        m_asEntryCombo->setEnabled(false);
        m_asEntryCombo->setFixedWidth(60);
        connect(m_asEntryCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setEntry);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMeasurePages()
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

        m_pMainTab->addTab(pView, tr(MeasureType[measureType]));

        pView->setFrameStyle(QFrame::NoFrame);

        appendMeasureView(measureType, pView);

        connect(this, &MainWindow::appendMeasure, pView, &MeasureView::appendMeasure, Qt::QueuedConnection);
        connect(pView, &MeasureView::measureCountChanged, this, &MainWindow::measureCountChanged, Qt::QueuedConnection);
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

void  MainWindow::appendMeasureView(int measureType, MeasureView* pView)
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

            if(m_pEditMenu != nullptr)
            {
                m_pEditMenu->addAction(findAction);
            }

            if(m_pMeasureControlToolBar != nullptr)
            {
                m_pMeasureControlToolBar->addAction(findAction);
            }
        }
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
    }

    // Panel comparator information
    //
    m_pComparatorInfoPanel = new QDockWidget(tr("Panel comparator information"), this);
    m_pComparatorInfoPanel->setObjectName("Panel comparator information");
    if (m_pComparatorInfoPanel != nullptr)
    {
        m_pComparatorInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

        m_pComparatorInfoView = new QTableView;
        if (m_pComparatorInfoView != nullptr)
        {
            m_pComparatorInfoPanel->setWidget(m_pComparatorInfoView);
        }
        addDockWidget(Qt::BottomDockWidgetArea, m_pComparatorInfoPanel);

        if (m_pViewPanelMenu != nullptr)
        {
            m_pViewPanelMenu->addAction(m_pComparatorInfoPanel->toggleViewAction());
        }

        m_pComparatorInfoPanel->hide();
    }
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
    m_statusConnectToAppDataServer = new QLabel(pStatusBar);
    m_statusConnectToTuningServer = new QLabel(pStatusBar);

    m_statusMeasureTimeout->setTextVisible(false);
    m_statusMeasureTimeout->setRange(0, 100);
    m_statusMeasureTimeout->setFixedWidth(100);
    m_statusMeasureTimeout->setFixedHeight(10);
    m_statusMeasureTimeout->setLayoutDirection(Qt::LeftToRight);

    pStatusBar->addWidget(m_statusConnectToTuningServer);
    pStatusBar->addWidget(m_statusConnectToAppDataServer);
    pStatusBar->addWidget(m_statusCalibratorCount);
    pStatusBar->addWidget(m_statusMeasureThreadState);
    pStatusBar->addWidget(m_statusMeasureTimeout);
    pStatusBar->addWidget(m_statusMeasureThreadInfo);
    pStatusBar->addWidget(m_statusEmpty);

    pStatusBar->setLayoutDirection(Qt::RightToLeft);

    m_statusEmpty->setText(QString());

    m_statusConnectToAppDataServer->setText(tr(" AppDataService: offline "));
    m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(255, 160, 160);");
    m_statusConnectToAppDataServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));

    m_statusConnectToTuningServer->setText(tr(" TuningService: offline "));
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

void MainWindow::updateCasesOnToolBar()
{
    m_asCaseTypeCombo->clear();
    m_asCaseTypeCombo->setEnabled(false);

    int outputSignalType = theOptions.toolBar().outputSignalType();
    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    theSignalBase.createCaseTypeList( theOptions.toolBar().outputSignalType() );

    int caseTypeCount = theSignalBase.caseTypeCount();
    if (caseTypeCount == 0)
    {
        return;
    }

    m_asCaseTypeCombo->blockSignals(true);

    for(int c = 0; c < caseTypeCount; c++)
    {
        m_asCaseTypeCombo->addItem( theSignalBase.caseTypeCaption(c) );
    }

    m_asCaseTypeCombo->setCurrentIndex(0);
    m_asCaseTypeCombo->setEnabled(true);
    m_asCaseTypeCombo->blockSignals(false);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateSignalsOnToolBar()
{
    m_asSignalCombo->clear();
    m_asSignalCombo->setEnabled(false);

    m_asCaseNoCombo->clear();
    m_asCaseNoCombo->setEnabled(false);

    m_asSubblockCombo->clear();
    m_asSubblockCombo->setEnabled(false);

    m_asBlockCombo->clear();
    m_asBlockCombo->setEnabled(false);

    m_asEntryCombo->clear();
    m_asEntryCombo->setEnabled(false);

    int caseType = m_asCaseTypeCombo->currentIndex();
    if (caseType == -1)
    {
        return;
    }

    int measureKind = theOptions.toolBar().measureKind();
    if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
    {
        return;
    }

    int outputSignalType = theOptions.toolBar().outputSignalType();
    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    int signalCount = theSignalBase.createMeasureSignalList(caseType, measureKind, outputSignalType);
    if (signalCount == 0)
    {
        return;
    }

    QMap<int, int> subblockMap;
    QMap<int, int> blockMap;
    QMap<int, int> entryMap;


    // fill signal on Tool bar
    //

    m_asSignalCombo->blockSignals(true);

    for(int s = 0; s < signalCount; s++)
    {
        MeasureSignal measureSignal = theSignalBase.measureSignal(s);
        if(measureSignal.isEmpty() == true)
        {
            continue;
        }

        MetrologyMultiSignal signal = measureSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);
        if (signal.isEmpty() == true)
        {
            continue;
        }

        if (signal.strID().isEmpty() == true)
        {
            continue;
        }

        m_asSignalCombo->addItem( signal.strID() );


        if (subblockMap.contains(signal.position().subblock()) == false)
        {
            subblockMap.insert(signal.position().subblock(), s);

            m_asSubblockCombo->addItem( QString::number( signal.position().subblock() + 1 ), signal.position().subblock() );
        }

        if (blockMap.contains(signal.position().block()) == false)
        {
            blockMap.insert(signal.position().block(), s);

            m_asBlockCombo->addItem( QString::number( signal.position().block() + 1 ), signal.position().block() );
        }

        if (entryMap.contains(signal.position().entry()) == false)
        {
            entryMap.insert(signal.position().entry(), s);

            m_asEntryCombo->addItem( QString::number( signal.position().entry() + 1 ), signal.position().entry() );
        }
    }

    m_asSignalCombo->blockSignals(false);

    if (m_asSignalCombo->count() == 0)
    {
        return;
    }

    updateSignalPositionOnToolBar();

    m_asSubblockCombo->setEnabled(true);
    m_asBlockCombo->setEnabled(true);
    m_asEntryCombo->setEnabled(true);

    m_asSignalCombo->setEnabled(true);
    m_asSignalCombo->setCurrentIndex(0);

    setMeasureSignal(0);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::updateSignalPositionOnToolBar()
{
    m_asCaseNoCombo->clear();
    m_asCaseNoCombo->setEnabled(false);

    int measureKind = theOptions.toolBar().measureKind();
    if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
    {
        return;
    }

    // fill caseNo on Tool bar
    //

    if (measureKind == MEASURE_KIND_ONE)
    {
        int CaseNoCount = theSignalBase.caseNoCount();
        if (CaseNoCount != 0)
        {
            for(int i = 0; i < CaseNoCount; i++)
            {
                int caseNo = theSignalBase.caseNoByCaseIndex( i );
                m_asCaseNoCombo->addItem( QString::number( caseNo + 1 ), caseNo );
            }

            m_asCaseNoCombo->setCurrentIndex(0);
            m_asCaseNoCombo->setEnabled(true);
        }
    }
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

    measureCountChanged(pView->table().count());

    switch(measureType)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            m_pMeasureKind->show();
            m_pOutputSignalToolBar->show();
            m_pAnalogSignalToolBar->show();

            m_pSignalInfoPanel->show();
            m_pComparatorInfoPanel->show();

            break;

        default:
            assert(0);
            break;
    }

    m_measureType = measureType;

    updateStartStopActions();

    m_pFindMeasurePanel->clear();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureCountChanged(int)
{
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startMeasure()
{
    // test
    //
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    // test
    //

    if (m_pSignalSocket == nullptr || m_pSignalSocketThread == nullptr)
    {
        return;
    }

    if (m_pSignalSocket->isConnected() == false)
    {
        QMessageBox::critical(this, windowTitle(), tr("No connect to Application Data Service!"));
        return;
    }

    // test
    //

    switch ( theOptions.toolBar().outputSignalType() )
    {
        case OUTPUT_SIGNAL_TYPE_UNUSED:
        case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

            if (theCalibratorBase.connectedCalibratorsCount() == 0)
            {
                QMessageBox::critical(this, windowTitle(), tr("Proccess of measure can not start, because no connected calibrators!\nPlease, make initialization calibrators"));
                return;
            }


            break;

        case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

            if (m_pTuningSocket == nullptr || m_pTuningSocketThread == nullptr)
            {
                return;
            }

            if (m_pTuningSocket->isConnected() == false)
            {
                QMessageBox::critical(this, windowTitle(), tr("No connect to Tuning Service!"));
                return;
            }

            break;

        default:
            assert(0);
    }


    // test
    //

    MeasureSignal measureSignal = theSignalBase.activeSignal();
    if(measureSignal.isEmpty() == true)
    {
        assert(false);
        return;
    }

    MetrologyMultiSignal signal = measureSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);
    if (signal.isEmpty() == true)
    {
        QMessageBox::critical(this, windowTitle(), tr("Signal for measure is not selected!"));
        return;
    }

    // temporary solution
    // param.setStatistic( theMeasureBase.statisticItem( param.hash() ) );
    //
    //
    MeasureView* pMeasureView = activeMeasureView();
    if (pMeasureView == nullptr)
    {
        return;
    }

    QString measuredSignalID;

    for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
    {
        Hash hash = signal.hash( c );
        if (hash == 0)
        {
            continue;
        }

        SignalParam param = theSignalBase.signalParam(hash);
        param.setStatistic( pMeasureView->table().m_measureBase.statistic( hash ) );

        if ( param.statistic().measureCount() != 0 )
        {
            measuredSignalID.append( param.customAppSignalID() + "\n");
        }
    }
    //
    //
    // temporary solution

    if (measuredSignalID.isEmpty() == false)
    {
        if (QMessageBox::question(this, windowTitle(), tr("Following signals were measured:\n\n%1\nDo you want to measure them again?").arg(measuredSignalID)) == QMessageBox::No)
        {
            return;
        }
    }

    // test
    //
    if (m_measureType == MEASURE_TYPE_LINEARITY)
    {
        if (theOptions.linearity().points().count() == 0)
        {
            QMessageBox::critical(this, windowTitle(), tr("No points for measure"));
            return;
        }
    }

    if (m_measureThread.isRunning() == true)
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

void MainWindow::printMeasure()
{
    MeasureView* pMeasureView = activeMeasureView();
    if (pMeasureView == nullptr)
    {
        return;
    }

    ReportView report;
    report.preview(pMeasureView);
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

    ExportData* dialog = new ExportData(pMeasureView, MeasureFileName[m_measureType]);
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

void MainWindow::showCalculator()
{
    if (m_pCalculator == nullptr)
    {
        return;
    }

    m_pCalculator->show();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::calibrators()
{
    theCalibratorBase.showInitDialog();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showSignalList()
{
    SignalListDialog dialog(false, this);
    dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showOutputSignalList()
{
    OutputSignalDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    if (theOutputSignalBase.save() == false)
    {
        QMessageBox::information(this, windowTitle(), tr("Attempt to save output signals was unsuccessfully!"));
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showTuningSignalList()
{
    TuningSignalListDialog dialog(false, this);
    dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::options()
{
    OptionsDialog dialog;
    dialog.exec();

    for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
    {
        if (theOptions.m_updateColumnView[type] == false)
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
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::showStatistic()
{
    StatisticDialog dialog(this);
    dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureKind(int index)
{
    int kind = index;
    if (kind < 0 || kind >= MEASURE_KIND_COUNT)
    {
        return;
    }

    theOptions.toolBar().setMeasureKind(kind);
    theOptions.toolBar().save();

    updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureTimeout(QString value)
{
    theOptions.toolBar().setMeasureTimeout(value.toDouble() * 1000);
    theOptions.toolBar().save();

    m_statusMeasureTimeout->setRange(0, theOptions.toolBar().measureTimeout());
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setOutputSignalType(int index)
{
    if (index == -1)
    {
        return;
    }

    int type = index;
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    theOptions.toolBar().setOutputSignalType(type);
    theOptions.toolBar().save();

    updateCasesOnToolBar();
    updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setCaseType(int index)
{
    int type = index;
    if (type < 0 || type >= theSignalBase.caseTypeCount())
    {
        return;
    }

    updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureSignal(int index)
{
    if (index < 0 || index >= theSignalBase.measureSignalCount())
    {
        return;
    }

    MeasureSignal measureSignal = theSignalBase.measureSignal(index);
    if(measureSignal.isEmpty() == true)
    {
        assert(false);
        return;
    }

    theSignalBase.setActiveSignal( measureSignal );

    MetrologyMultiSignal signal = measureSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);
    if (signal.isEmpty() == true)
    {
        return;
    }

    if (theOptions.toolBar().measureKind() == MEASURE_KIND_ONE)
    {
        m_asCaseNoCombo->setCurrentText( QString::number( signal.position().caseNo() + 1) );
    }

    m_asSubblockCombo->setCurrentText( QString::number( signal.position().subblock() + 1) );
    m_asBlockCombo->setCurrentText( QString::number( signal.position().block() + 1) );
    m_asEntryCombo->setCurrentText( QString::number( signal.position().entry() + 1) );
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setCaseNo(int index)
{
    if (index = -1)
    {
        return;
    }

//    int caseNo = index;

//    m_asSubblockCombo->clear();
//    m_asBlockCombo->clear();
//    m_asEntryCombo->clear();

//    QMap<int, int> subblockMap;
//    QMap<int, int> blockMap;
//    QMap<int, int> entryMap;

//    int signalCount = theSignalBase.signalForMeasureCount();
//    for(int s = 0; s < signalCount; s++)
//    {
//        MeasureSignal ms;

//        if (theSignalBase.signalForMeasure(s, ms) == false)
//        {
//            continue;
//        }

//        if (ms.caseNo() != caseNo)
//        {
//            continue;
//        }

//        if (subblockMap.contains(ms.subblock()) == false)
//        {
//            subblockMap.insert(ms.subblock(), s);

//            m_asSubblockCombo->addItem( QString::number( ms.subblock() + 1 ), ms.subblock() );
//        }

//        if (blockMap.contains(ms.block()) == false)
//        {
//            blockMap.insert(ms.block(), s);

//            m_asBlockCombo->addItem( QString::number( ms.block() + 1 ), ms.block() );
//        }

//        if (entryMap.contains(ms.entry()) == false)
//        {
//            entryMap.insert(ms.entry(), s);

//            m_asEntryCombo->addItem( QString::number( ms.entry() + 1 ), ms.entry() );
//        }
//    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setSubblock(int index)
{
    if (index = -1)
    {
        return;
    }

//    int caseNo = m_asCaseNoCombo->currentData().toInt();
//    int subblock = index;

//    m_asBlockCombo->clear();
//    m_asEntryCombo->clear();

//    QMap<int, int> blockMap;
//    QMap<int, int> entryMap;

//    int signalCount = theSignalBase.signalForMeasureCount();
//    for(int s = 0; s < signalCount; s++)
//    {
//        MeasureSignal ms;

//        if (theSignalBase.signalForMeasure(s, ms) == false)
//        {
//            continue;
//        }

//        if (ms.caseNo() != caseNo || ms.subblock() != subblock)
//        {
//            continue;
//        }

//        if (blockMap.contains(ms.block()) == false)
//        {
//            blockMap.insert(ms.block(), s);

//            m_asBlockCombo->addItem( QString::number( ms.block() + 1 ), ms.block() );
//        }

//        if (entryMap.contains(ms.entry()) == false)
//        {
//            entryMap.insert(ms.entry(), s);

//            m_asEntryCombo->addItem( QString::number( ms.entry() + 1 ), ms.entry() );
//        }
//    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setBlock(int index)
{
    if (index = -1)
    {
        return;
    }

//    int caseNo = m_asCaseNoCombo->currentData().toInt();
//    int subblock = m_asSubblockCombo->currentData().toInt();
//    int block = index;

//    m_asEntryCombo->clear();

//    QMap<int, int> entryMap;

//    int signalCount = theSignalBase.signalForMeasureCount();
//    for(int s = 0; s < signalCount; s++)
//    {
//        MeasureSignal ms;

//        if (theSignalBase.signalForMeasure(s, ms) == false)
//        {
//            continue;
//        }

//        if (ms.caseNo() != caseNo || ms.subblock() != subblock || ms.block() != block)
//        {
//            continue;
//        }

//        if (entryMap.contains(ms.entry()) == false)
//        {
//            entryMap.insert(ms.entry(), s);

//            m_asEntryCombo->addItem( QString::number( ms.entry() + 1 ), ms.entry() );
//        }
//    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setEntry(int index)
{
    if (index = -1)
    {
        return;
    }

//    int caseNo = m_asCaseNoCombo->currentData().toInt();
//    int subblock = m_asSubblockCombo->currentData().toInt();
//    int block = m_asBlockCombo->currentData().toInt();
//    int entry = m_asEntryCombo->currentData().toInt();

//    qDebug() << "setMetrologySignalByPosition: C" << caseNo << " - S" << subblock << " - B" << block << " - E" << entry;

//    int signalCount = theSignalBase.signalForMeasureCount();

//    for(int s = 0; s < signalCount; s++)
//    {
//        MeasureSignal ms;

//        if (theSignalBase.signalForMeasure(s, ms) == false)
//        {
//            continue;
//        }

//        if (ms.caseNo() == caseNo && ms.subblock() == subblock && ms.block() == block && ms.entry() == entry)
//        {
//            theSignalBase.setActiveSignal( ms );

//            m_asMetrologySignalCombo->setCurrentIndex(s);

//            break;
//        }
//    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::MainWindow::setMetrologySignalByPosition(int index)
{
    if (index = -1)
    {
        return;
    }
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
    updateStartStopActions();

    m_statusCalibratorCount->setText( tr(" Connected calibrators: %1 ").arg(count) );

    if (count == 0)
    {
        m_statusCalibratorCount->setStyleSheet("background-color: rgb(255, 160, 160);");
        m_statusCalibratorCount->setToolTip(tr("Please, connect Calibrators\nclick menu \"Tool\" - \"Calibrators...\""));
    }
    else
    {
        m_statusCalibratorCount->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
        m_statusCalibratorCount->setToolTip(QString());
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalSocketConnected()
{
    updateStartStopActions();

    m_statusConnectToAppDataServer->setText( tr(" AppDataService: on  ") );
    m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
    m_statusConnectToAppDataServer->setToolTip(tr("Connected: %1 : %2\nLoaded signals: 0").arg(theOptions.signalSocket().serverIP()).arg(theOptions.signalSocket().serverPort()) );
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalSocketDisconnected()
{
    if (m_measureThread.isRunning() == true)
    {
        m_measureThread.stop();
    }

    updateStartStopActions();

    m_asCaseTypeCombo->clear();
    m_asCaseTypeCombo->setEnabled(false);

    m_asSignalCombo->clear();
    m_asSignalCombo->setEnabled(false);

    m_asCaseNoCombo->clear();
    m_asCaseNoCombo->setEnabled(false);

    m_asSubblockCombo->clear();
    m_asSubblockCombo->setEnabled(false);

    m_asBlockCombo->clear();
    m_asBlockCombo->setEnabled(false);

    m_asEntryCombo->clear();
    m_asEntryCombo->setEnabled(false);

    theTuningSignalBase.clearSignalLlst();

    theSignalBase.clear();

    m_statusConnectToAppDataServer->setText( tr(" AppDataService: off ") );
    m_statusConnectToAppDataServer->setStyleSheet("background-color: rgb(255, 160, 160);");
    m_statusConnectToAppDataServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::signalSocketSignalsLoaded()
{
    m_statusConnectToAppDataServer->setToolTip(tr("Connected: %1 : %2\nLoaded signals: %3").arg(theOptions.signalSocket().serverIP()).arg(theOptions.signalSocket().serverPort()).arg(theSignalBase.signalCount()) );

    theSignalBase.sortByPosition();

    theTuningSignalBase.createSignalList();

    updateCasesOnToolBar();
    updateSignalsOnToolBar();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::tuningSocketConnected()
{
    m_statusConnectToTuningServer->setText( tr(" TuningService: on  ") );
    m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
    m_statusConnectToTuningServer->setToolTip(tr("Connected: %1 : %2\nTuning signals: %3").arg(theOptions.tuningSocket().serverIP()).arg(theOptions.tuningSocket().serverPort()).arg(theTuningSignalBase.signalCount()) );
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::tuningSocketDisconnected()
{
    if (m_measureThread.isRunning() == true)
    {
        if (theOptions.toolBar().outputSignalType() == OUTPUT_SIGNAL_TYPE_FROM_TUNING)
        {
            m_measureThread.stop();
        }
    }

    theTuningSignalBase.clearSourceList();
    theTuningSignalBase.singalsSetNovalid();

    m_statusConnectToTuningServer->setText( tr(" TuningService: off ") );
    m_statusConnectToTuningServer->setStyleSheet("background-color: rgb(255, 160, 160);");
    m_statusConnectToTuningServer->setToolTip(tr("Please, connect to server\nclick menu \"Tool\" - \"Options...\" - \"Connect to server\""));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadStarted()
{
    updateStartStopActions();

    m_pMeasureKind->setDisabled(true);
    m_pOutputSignalToolBar->setDisabled(true);
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
    updateStartStopActions();

    m_pMeasureKind->setEnabled(true);
    m_pOutputSignalToolBar->setEnabled(true);
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

void MainWindow::loadSettings()
{
    QSettings s;

    QByteArray geometry = s.value(QString("%1MainWindow/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
    QByteArray state = s.value(QString("%1MainWindow/State").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();

    restoreGeometry( geometry );
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
    if (m_pCalculator != nullptr)
    {
        delete m_pCalculator;
    }

    if (m_measureThread.isRunning() == true)
    {
        QMessageBox::critical(this, windowTitle(), m_statusMeasureThreadState->text());
        e->ignore();
        return;
    }

    if (m_pTuningSocketThread != nullptr)
    {
        m_pTuningSocketThread->quitAndWait(10000);
        delete m_pTuningSocketThread;
    }

    if (m_pSignalSocketThread != nullptr)
    {
        m_pSignalSocketThread->quitAndWait(10000);
        delete m_pSignalSocketThread;
    }

    theTuningSignalBase.clear();

    theSignalBase.clear();

    theMeasurementBase.clear();

    theCalibratorBase.clear();

    saveSettings();

    QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
