#include "MainWindow.h"

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

#include "CalibratorBase.h"
#include "ExportMeasure.h"
#include "OptionsDialog.h"

#include <QWindow>

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // init calibration base
    //
    theCalibratorBase.init(this);

    connect(&theCalibratorBase, &CalibratorBase::calibratorConnectedChanged, this, &MainWindow::calibratorConnectedChanged, Qt::QueuedConnection);

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

    measureThreadStoped();
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

    createActions();
    createMenu();
    createToolBars();
    createMeasurePages();
    createPanels();
    createStatusBar();

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
    m_pCutMeasureAction = new QAction(tr("Cu&t"), this);
    m_pCutMeasureAction->setShortcut(Qt::CTRL + Qt::Key_X);
    m_pCutMeasureAction->setIcon(QIcon(":/icons/Cut.png"));
    m_pCutMeasureAction->setToolTip(tr("Cut of the measurements"));
    connect(m_pCutMeasureAction, &QAction::triggered, this, &MainWindow::cutMeasure);

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

    m_pShowCalculateAction = new QAction(tr("Metrological &calculator ..."), this);
    m_pShowCalculateAction->setShortcut(Qt::ALT + Qt::Key_C);
    m_pShowCalculateAction->setIcon(QIcon(":/icons/Calculator.png"));
    m_pShowCalculateAction->setToolTip(tr("Calculator for converting metrological quantities"));
    connect(m_pShowCalculateAction, &QAction::triggered, this, &MainWindow::showCalculate);

    // Tools
    //
    m_pCalibratorsAction = new QAction(tr("&Calibrations ..."), this);
    m_pCalibratorsAction->setIcon(QIcon(":/icons/Calibrators.png"));
    m_pCalibratorsAction->setToolTip(tr("Connecting and configuring calibrators"));
    connect(m_pCalibratorsAction, &QAction::triggered, this, &MainWindow::calibrators);

    m_pShowOutputSignalListAction = new QAction(tr("Signals input/output ..."), this);
    m_pShowOutputSignalListAction->setIcon(QIcon(":/icons/InOut.png"));
    m_pShowOutputSignalListAction->setToolTip("");
    connect(m_pShowOutputSignalListAction, &QAction::triggered, this, &MainWindow::showOutputSignalList);

    m_pShowOutputRangeListAction = new QAction(tr("Signals with output ranges ..."), this);
    m_pShowOutputRangeListAction->setIcon(QIcon(":/icons/OutRange.png"));
    m_pShowOutputRangeListAction->setToolTip("");
    connect(m_pShowOutputRangeListAction, &QAction::triggered, this, &MainWindow::showOutputRangeList);

    m_pShowComlexComparatorListAction = new QAction(tr("&Signals of complex comparator ..."), this);
    m_pShowComlexComparatorListAction->setIcon(QIcon(":/icons/Complex.png"));
    m_pShowComlexComparatorListAction->setToolTip("");
    connect(m_pShowComlexComparatorListAction, &QAction::triggered, this, &MainWindow::showComlexComparatorList);

    m_pOptionsAction = new QAction(tr("&Options ..."), this);
    m_pOptionsAction->setShortcut(Qt::CTRL + Qt::Key_O);
    m_pOptionsAction->setIcon(QIcon(":/icons/Options.png"));
    m_pOptionsAction->setToolTip(tr("Editing application settings"));
    connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::options);


    // ?
    //
    m_pShowSignalListAction = new QAction(tr("&Signals ..."), this);
    m_pShowSignalListAction->setIcon(QIcon(":/icons/Signals.png"));
    m_pShowSignalListAction->setToolTip("");
    connect(m_pShowSignalListAction, &QAction::triggered, this, &MainWindow::showSignalList);

    m_pShowComparatorsListAction = new QAction(tr("&Comparators ..."), this);
    m_pShowComparatorsListAction->setIcon(QIcon(":/icons/Signals.png"));
    m_pShowComparatorsListAction->setToolTip("");
    connect(m_pShowComparatorsListAction, &QAction::triggered, this, &MainWindow::showComparatorsList);

    m_pShowCorrecrtionsListAction = new QAction(tr("Co&rrections ..."), this);
    m_pShowCorrecrtionsListAction->setIcon(QIcon(":/icons/Signals.png"));
    m_pShowCorrecrtionsListAction->setToolTip("");
    connect(m_pShowCorrecrtionsListAction, &QAction::triggered, this, &MainWindow::showCorrecrtionsList);

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

void MainWindow::updateActions()
{
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

            if (theCalibratorBase.getConnectedCalibratorsCount() == 0)
            {
               startMeasure = false;
            }

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            if (theCalibratorBase.getConnectedCalibratorsCount() < CALIBRATOR_COUNT_FOR_CC)
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

    m_pEditMenu->addAction(m_pCutMeasureAction);
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
    m_pViewMenu->addAction(m_pShowCalculateAction);


    m_pSettingMenu = pMenuBar->addMenu(tr("&Tools"));

    m_pSettingMenu->addAction(m_pCalibratorsAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pShowOutputSignalListAction);
    m_pSettingMenu->addAction(m_pShowOutputRangeListAction);
    m_pSettingMenu->addAction(m_pShowComlexComparatorListAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pOptionsAction);


    m_pInfoMenu = pMenuBar->addMenu(tr("&?"));

    m_pInfoMenu->addAction(m_pShowSignalListAction);
    m_pInfoMenu->addAction(m_pShowComparatorsListAction);
    m_pInfoMenu->addAction(m_pShowCorrecrtionsListAction);
    m_pInfoMenu->addSeparator();
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

        measureTimeoutList->setCurrentText(QString::number(double(theOptions.toolBar().m_measureTimeout) / 1000, 10, 1));

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

        measureKindList->setCurrentIndex(theOptions.toolBar().m_measureKind);

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
        QComboBox* outputSignalList = new QComboBox(m_pOutputSignalToolBar);

        m_pOutputSignalToolBar->addWidget(outputSignalLabel);
        outputSignalLabel->setText(tr(" Output signals "));
        outputSignalLabel->setEnabled(false);

        m_pOutputSignalToolBar->addWidget(outputSignalList);

        for(int s = 0; s < OUTPUT_SIGNAL_TYPE_COUNT; s++)
        {
            outputSignalList->addItem(OutputSignalType[s]);
        }
        outputSignalList->setCurrentIndex(theOptions.toolBar().m_outputSignalType);

        connect(outputSignalList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setOutputSignalType);
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

        QComboBox* asCaseCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asCaseCombo);
        asCaseCombo->addItem("");
        asCaseCombo->setEnabled(false);
        asCaseCombo->setFixedWidth(100);

        QLabel* asSignalLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSignalLabel);
        asSignalLabel->setText(tr(" Signal "));
        asSignalLabel->setEnabled(false);

        QComboBox* asSignalCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSignalCombo);
        asSignalCombo->addItem("");
        asSignalCombo->setEnabled(false);
        asSignalCombo->setFixedWidth(250);

        m_pAnalogSignalToolBar->addSeparator();

        QLabel* asChannelLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asChannelLabel);
        asChannelLabel->setText(tr(" Case No "));
        asChannelLabel->setEnabled(false);

        QComboBox* asChannelCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asChannelCombo);
        asChannelCombo->addItem("");
        asChannelCombo->setEnabled(false);
        asChannelCombo->setFixedWidth(60);

        QLabel* asSubblockLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSubblockLabel);
        asSubblockLabel->setText(tr(" Subblock "));
        asSubblockLabel->setEnabled(false);

        QComboBox* asSubblockCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asSubblockCombo);
        asSubblockCombo->addItem("");
        asSubblockCombo->setEnabled(false);
        asSubblockCombo->setFixedWidth(60);

        QLabel* asBlockLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asBlockLabel);
        asBlockLabel->setText(tr(" Block "));
        asBlockLabel->setEnabled(false);

        QComboBox* asBlockCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asBlockCombo);
        asBlockCombo->addItem("");
        asBlockCombo->setEnabled(false);
        asBlockCombo->setFixedWidth(60);

        QLabel* asEntryLabel = new QLabel(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asEntryLabel);
        asEntryLabel->setText(tr(" Entry "));
        asEntryLabel->setEnabled(false);

        QComboBox* asEntryCombo = new QComboBox(m_pAnalogSignalToolBar);
        m_pAnalogSignalToolBar->addWidget(asEntryCombo);
        asEntryCombo->addItem("");
        asEntryCombo->setEnabled(false);
        asEntryCombo->setFixedWidth(60);
    }


    // Control panel selecting signal of complex comparator
    //
    m_pComplexComporatorToolBar = new QToolBar(this);
    if (m_pComplexComporatorToolBar != nullptr)
    {
        m_pComplexComporatorToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pComplexComporatorToolBar->setWindowTitle(tr("Control panel selecting signal of complex comparator"));
        m_pComplexComporatorToolBar->setObjectName(m_pComplexComporatorToolBar->windowTitle());
        addToolBarBreak(Qt::TopToolBarArea);
        addToolBar(m_pComplexComporatorToolBar);

        QLabel* ccTypeLabel = new QLabel(m_pComplexComporatorToolBar);
        m_pComplexComporatorToolBar->addWidget(ccTypeLabel);
        ccTypeLabel->setText(tr(" Type "));
        ccTypeLabel->setEnabled(false);


        QComboBox* ccTypeCombo = new QComboBox(m_pComplexComporatorToolBar);
        m_pComplexComporatorToolBar->addWidget(ccTypeCombo);
        ccTypeCombo->addItem("");
        ccTypeCombo->setEnabled(false);
        ccTypeCombo->setFixedWidth(60);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMeasurePages()
{
    m_pMainTab = new QTabWidget();
    m_pMainTab->setTabPosition(QTabWidget::South);

    for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
    {
        MeasureView* pView = new MeasureView(type, this);
        if (pView == nullptr)
        {
            continue;
        }

        m_pMainTab->addTab(pView, tr(MeasureType[type]));

        pView->setFrameStyle(QFrame::NoFrame);

        m_measureView[type] = pView;

        connect(this, &MainWindow::appendMeasure, pView, &MeasureView::appendMeasure, Qt::QueuedConnection);
        connect(pView, &MeasureView::measureCountChanged, this, &MainWindow::measureCountChanged, Qt::QueuedConnection);
    }

    setCentralWidget(m_pMainTab);

    connect(m_pMainTab, &QTabWidget::currentChanged, this, &MainWindow::setMeasureType);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
    // Search measurements panel
    //
    m_pFindMeasurePanel = new FindMeasure(this);
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
    m_pSignalInfoPanel = new QDockWidget(tr("Panel signal information"), this);
    m_pSignalInfoPanel->setObjectName("Panel signal information");
    if (m_pSignalInfoPanel != nullptr)
    {
        m_pSignalInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

        m_pSignalInfoView = new QTableView;

        if (m_pSignalInfoPanel != nullptr)
        {
            m_pSignalInfoPanel->setWidget(m_pSignalInfoView);
        }
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


    // Panel complex comparator information
    //
    m_pComplexComparatorInfoPanel = new QDockWidget(tr("Panel complex comparator information"), this);
    m_pComplexComparatorInfoPanel->setObjectName("Panel complex comparator information");
    if (m_pComplexComparatorInfoPanel != nullptr)
    {
        m_pComplexComparatorInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

        m_pComplexComparatorInfoView = new QTableView;
        if (m_pComplexComparatorInfoView != nullptr)
        {
            m_pComplexComparatorInfoPanel->setWidget(m_pComplexComparatorInfoView);
        }
        addDockWidget(Qt::BottomDockWidgetArea, m_pComplexComparatorInfoPanel);

        if (m_pViewPanelMenu != nullptr)
        {
            m_pViewPanelMenu->addAction(m_pComplexComparatorInfoPanel->toggleViewAction());
        }

        m_pComplexComparatorInfoPanel->hide();
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
    m_statusMeasureCount = new QLabel(pStatusBar);
    m_statusCalibratorCount = new QLabel(pStatusBar);
    m_statusConnectToServer = new QLabel(pStatusBar);

    m_statusMeasureTimeout->setTextVisible(false);
    m_statusMeasureTimeout->setRange(0, 100);
    m_statusMeasureTimeout->setFixedWidth(100);
    m_statusMeasureTimeout->setFixedHeight(10);
    m_statusMeasureTimeout->setLayoutDirection(Qt::LeftToRight);

    pStatusBar->addWidget(m_statusConnectToServer);
    pStatusBar->addWidget(m_statusCalibratorCount);
    pStatusBar->addWidget(m_statusMeasureCount);
    pStatusBar->addWidget(m_statusMeasureThreadState);
    pStatusBar->addWidget(m_statusMeasureTimeout);
    pStatusBar->addWidget(m_statusMeasureThreadInfo);
    pStatusBar->addWidget(m_statusEmpty);

    pStatusBar->setLayoutDirection(Qt::RightToLeft);

    m_statusEmpty->setText("");
    m_statusMeasureCount->setText(tr("Measurement 0/0 "));
    m_statusConnectToServer->setText(tr("Connect to server: offline "));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureType(int type)
{
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    MeasureView* pView = m_measureView[type];
    if (pView == nullptr)
    {
        return;
    }

    measureCountChanged(pView->table().count());

    switch(type)
    {
        case MEASURE_TYPE_LINEARITY:
        case MEASURE_TYPE_COMPARATOR:

            m_pMeasureKind->show();
            m_pOutputSignalToolBar->show();
            m_pAnalogSignalToolBar->show();
            m_pComplexComporatorToolBar->hide();

            m_pSignalInfoPanel->show();
            m_pComparatorInfoPanel->show();
            m_pComplexComparatorInfoPanel->hide();

            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:

            m_pMeasureKind->hide();
            m_pOutputSignalToolBar->hide();
            m_pAnalogSignalToolBar->hide();
            m_pComplexComporatorToolBar->show();

            m_pSignalInfoPanel->hide();
            m_pComparatorInfoPanel->hide();
            m_pComplexComparatorInfoPanel->show();

            break;

        default:
            assert(0);
            break;
    }

    m_measureType = type;

    updateActions();

    if (m_pFindMeasurePanel != nullptr)
    {
        m_pFindMeasurePanel->table().clear();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureCountChanged(int count)
{
    m_statusMeasureCount->setText(tr("Measurement 0 / %1 ").arg(count));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::startMeasure()
{
    if (m_measureThread.isRunning() == true)
    {
        return;
    }

    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
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
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    MeasureView* pView = m_measureView[m_measureType];
    if (pView == nullptr)
    {
        return;
    }

    ExportMeasure* dialog = new ExportMeasure(pView);
    dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::selectAllMeasure()
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    MeasureView* pView = m_measureView[m_measureType];
    if (pView == nullptr)
    {
        return;
    }

    pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::calibrators()
{
    theCalibratorBase.show();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::options()
{
    OptionsDialog dialog;
    dialog.exec();

    for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
    {
        if (theOptions.m_updateColumnView[type] = false)
        {
            continue;
        }

        MeasureView* pView = m_measureView[type];
        if (pView == nullptr)
        {
            continue;
        }

        pView->updateColumn();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureKind(int index)
{
    int kind = index;
    if (kind < 0 || kind >= MEASURE_KIND_COUNT)
    {
        return;
    }

    theOptions.toolBar().m_measureKind = kind;
    theOptions.toolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureTimeout(QString value)
{
    theOptions.toolBar().m_measureTimeout = value.toDouble() * 1000;
    theOptions.toolBar().save();

    m_statusMeasureTimeout->setRange(0, theOptions.toolBar().m_measureTimeout);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setOutputSignalType(int index)
{
    int type = index;
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    theOptions.toolBar().m_outputSignalType = type;
    theOptions.toolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::calibratorConnectedChanged(int count)
{
    updateActions();

    m_statusCalibratorCount->setText( tr(" Connected calibrators: %1 ").arg(count) );
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadStarted()
{
    updateActions();

    m_pMeasureKind->setDisabled(true);
    m_pOutputSignalToolBar->setDisabled(true);
    m_pAnalogSignalToolBar->setDisabled(true);
    m_pComplexComporatorToolBar->setDisabled(true);

    m_statusMeasureThreadInfo->setText("");

    if (theOptions.toolBar().m_measureTimeout != 0)
    {
        m_statusMeasureTimeout->show();
        m_statusMeasureTimeout->setRange(0, theOptions.toolBar().m_measureTimeout);
        m_statusMeasureTimeout->setValue(0);
    }
    else
    {
         m_statusMeasureTimeout->hide();
    }

    m_statusMeasureThreadState->setText(tr("The measurement process is started "));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::measureThreadStoped()
{
    updateActions();

    m_pMeasureKind->setEnabled(true);
    m_pOutputSignalToolBar->setEnabled(true);
    m_pAnalogSignalToolBar->setEnabled(true);
    m_pComplexComporatorToolBar->setEnabled(true);

    m_statusMeasureThreadInfo->setText("");

    m_statusMeasureTimeout->hide();
    m_statusMeasureTimeout->setRange(0, theOptions.toolBar().m_measureTimeout);
    m_statusMeasureTimeout->setValue(0);

    m_statusMeasureThreadState->setText(tr("The measurement process is stopped "));
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

void MainWindow::measureComplite(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }

    int type = pMeasure->measureType();
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    emit appendMeasure(pMeasure);
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
    if (m_measureThread.isRunning() == true)
    {
        QMessageBox::information(this, windowTitle(), m_statusMeasureThreadState->text());
        e->ignore();
        return;
    }

    theCalibratorBase.clear();

    saveSettings();

    QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
