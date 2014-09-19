#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Metrology)
{
    ui->setupUi(this);

    createInterface();

    emit m_calibratorBase.openAllCalibrator();
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    delete ui;
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
    setWindowTitle(tr("Metrology"));

    createActions();
    createMenu();
    createToolBars();
    createTabPages();
    createPanels();
    createStatusBar();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void  MainWindow::createActions()
{
    // Measure
    //
    m_pStartMeasureAction = new QAction(tr("&Start"), this);
    m_pStartMeasureAction->setShortcut(Qt::Key_F5);
    m_pStartMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pStartMeasureAction, &QAction::triggered, this, &MainWindow::startMeasure);

    m_pStopMeasureAction = new QAction(tr("&Stop"), this);
    m_pStopMeasureAction->setShortcut(Qt::SHIFT + Qt::Key_F5);
    m_pStopMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pStopMeasureAction, &QAction::triggered, this, &MainWindow::stopMeasure);

    m_pExportMeasureAction = new QAction(tr("&Export ..."), this);
    connect(m_pExportMeasureAction, &QAction::triggered, this, &MainWindow::exportMeasure);


    // Edit
    //
    m_pCutMeasureAction = new QAction(tr("Cu&t"), this);
    m_pCutMeasureAction->setShortcut(Qt::CTRL + Qt::Key_X);
    m_pCutMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pCutMeasureAction, &QAction::triggered, this, &MainWindow::cutMeasure);

    m_pCopyMeasureAction = new QAction(tr("&Copy"), this);
    m_pCopyMeasureAction->setShortcut(Qt::CTRL + Qt::Key_C);
    m_pCopyMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pCopyMeasureAction, &QAction::triggered, this, &MainWindow::copyMeasure);

    m_pRemoveMeasureAction = new QAction(tr("&Delete"), this);
    m_pRemoveMeasureAction->setShortcut(Qt::CTRL + Qt::Key_Delete);
    m_pRemoveMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pRemoveMeasureAction, &QAction::triggered, this, &MainWindow::removeMeasure);

    m_pSelectAllMeasureAction = new QAction(tr("&Select All"), this);
    m_pSelectAllMeasureAction->setShortcut(Qt::CTRL + Qt::Key_A);
    connect(m_pSelectAllMeasureAction, &QAction::triggered, this, &MainWindow::selectAllMeasure);

    // View
    //
    m_pShowReportsAction = new QAction(tr("&Reports ..."), this);
    m_pShowReportsAction->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(m_pShowReportsAction, &QAction::triggered, this, &MainWindow::showReports);

    m_pShowCalculateAction = new QAction(tr("Metrological &calculator"), this);
    m_pShowCalculateAction->setShortcut(Qt::ALT + Qt::Key_C);
    connect(m_pShowCalculateAction, &QAction::triggered, this, &MainWindow::showCalculate);

    // Tools
    //
    m_pConnectToServerAction = new QAction(tr("Connect to &server ... "), this);
    connect(m_pConnectToServerAction, &QAction::triggered, this, &MainWindow::connectToServer);

    m_pCalibratorsAction = new QAction(tr("&Calibrations ..."), this);
    connect(m_pCalibratorsAction, &QAction::triggered, this, &MainWindow::calibrators);

    m_pShowOutputSignalListAction = new QAction(tr("&Signals input/output ..."), this);
    connect(m_pShowOutputSignalListAction, &QAction::triggered, this, &MainWindow::showOutputSignalList);

//    m_pShowComlexComparatorListAction = new QAction(tr("&Signals of complex comparator..."), this);
//    connect(m_pShowComlexComparatorListAction, &QAction::triggered, this, &MainWindow::showComlexComparatorList);

    m_pShowOutputRangeListAction = new QAction(tr("&Signals with output ranges ..."), this);
    connect(m_pShowOutputRangeListAction, &QAction::triggered, this, &MainWindow::showOutputRangeList);

    m_pOptionsAction = new QAction(tr("&Options ..."), this);
    m_pOptionsAction->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(m_pOptionsAction, &QAction::triggered, this, &MainWindow::options);


    // ?
    //
    m_pShowSignalListAction = new QAction(tr("&Signals ..."), this);
    connect(m_pShowSignalListAction, &QAction::triggered, this, &MainWindow::showSignalList);

    m_pShowComparatorsListAction = new QAction(tr("&Comparators ..."), this);
    connect(m_pShowComparatorsListAction, &QAction::triggered, this, &MainWindow::showComparatorsList);

    m_pShowCorrecrtionsListAction = new QAction(tr("Co&rrections ..."), this);
    connect(m_pShowCorrecrtionsListAction, &QAction::triggered, this, &MainWindow::showCorrecrtionsList);

    m_pShowStatisticAction = new QAction(tr("Sta&tistics ..."), this);
    connect(m_pShowStatisticAction, &QAction::triggered, this, &MainWindow::showStatistic);

    m_pAboutConnectionAction = new QAction(tr("About connect to server ..."), this);
    connect(m_pAboutConnectionAction, &QAction::triggered, this, &MainWindow::aboutConnection);

    m_pAboutAppAction = new QAction(tr("About Metrology ..."), this);
    connect(m_pAboutAppAction, &QAction::triggered, this, &MainWindow::aboutApp);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createMenu()
{
    m_pMeasureMenu = new QMenu(tr("&Measure"));

    m_pMeasureMenu->addAction(m_pStartMeasureAction);
    m_pMeasureMenu->addAction(m_pStopMeasureAction);
    m_pMeasureMenu->addSeparator();
    m_pMeasureMenu->addAction(m_pExportMeasureAction);


    m_pEditMenu = new QMenu(tr("&Edit"));

    m_pEditMenu->addAction(m_pCutMeasureAction);
    m_pEditMenu->addAction(m_pCopyMeasureAction);
    m_pEditMenu->addAction(m_pRemoveMeasureAction);
    m_pEditMenu->addSeparator();
    m_pEditMenu->addAction(m_pSelectAllMeasureAction);
    m_pEditMenu->addSeparator();

    m_pViewMenu = new QMenu(tr("&View"));

    m_pViewPanelMenu = new QMenu("&Panels", m_pViewMenu);
    m_pViewMenu->addMenu(m_pViewPanelMenu);
    m_pViewMenu->addAction(m_pShowReportsAction);
    m_pViewMenu->addSeparator();
    m_pViewMenu->addAction(m_pShowCalculateAction);


    m_pSettingMenu = new QMenu(tr("&Tools"));

    m_pSettingMenu->addAction(m_pConnectToServerAction);
    m_pSettingMenu->addAction(m_pCalibratorsAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pShowOutputSignalListAction);
//    m_pSettingMenu->addAction(m_pShowComlexComparatorListAction);
    m_pSettingMenu->addAction(m_pShowOutputRangeListAction);
    m_pSettingMenu->addSeparator();
    m_pSettingMenu->addAction(m_pOptionsAction);


    m_pInfoMenu = new QMenu(tr("&?"));

    m_pInfoMenu->addAction(m_pShowSignalListAction);
    m_pInfoMenu->addAction(m_pShowComparatorsListAction);
    m_pInfoMenu->addAction(m_pShowCorrecrtionsListAction);
    m_pInfoMenu->addSeparator();
    m_pInfoMenu->addAction(m_pShowStatisticAction);
    m_pInfoMenu->addSeparator();
    m_pInfoMenu->addAction(m_pAboutConnectionAction);
    m_pInfoMenu->addAction(m_pAboutAppAction);

    menuBar()->addMenu(m_pMeasureMenu);
    menuBar()->addMenu(m_pEditMenu);
    menuBar()->addMenu(m_pViewMenu);
    menuBar()->addMenu(m_pSettingMenu);
    menuBar()->addMenu(m_pInfoMenu);
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createToolBars()
{
    // Control panel measure process
    //
    m_pMeasureControlToolBar = new QToolBar(this);
    m_pMeasureControlToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_pMeasureControlToolBar->setWindowTitle(tr("Control panel measure process"));
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(m_pMeasureControlToolBar);

    m_pMeasureControlToolBar->addAction(m_pStartMeasureAction);
    m_pMeasureControlToolBar->addAction(m_pStopMeasureAction);
    m_pMeasureControlToolBar->addSeparator();
    m_pMeasureControlToolBar->addAction(m_pCopyMeasureAction);
    m_pMeasureControlToolBar->addAction(m_pRemoveMeasureAction);
    m_pMeasureControlToolBar->addSeparator();

    m_pMeasureControlToolBar->setIconSize(QSize(16,16));


    // Control panel measure timeout
    //
    m_pMeasureTimeout = new QToolBar(this);
    m_pMeasureTimeout->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_pMeasureTimeout->setWindowTitle(tr("Control panel measure timeout"));
    addToolBarBreak(Qt::RightToolBarArea);
    addToolBar(m_pMeasureTimeout);

    QLabel* measureTimeoutLabel = new QLabel(m_pMeasureTimeout);
    m_pMeasureTimeout->addWidget(measureTimeoutLabel);
    measureTimeoutLabel->setText(tr(" Measure timeout "));
    measureTimeoutLabel->setEnabled(false);

    QComboBox* measureTimeoutCombo = new QComboBox(m_pMeasureTimeout);
    m_pMeasureTimeout->addWidget(measureTimeoutCombo);
    measureTimeoutCombo->setEditable(true);
    measureTimeoutCombo->addItem("Empty");
    //connect(measureTimeoutCombo, SIGNAL(activated(int)), this, SLOT(setMeasureTimeout(int)));


    // Control panel output signals
    //
    m_pOutputSignalToolBar = new QToolBar(this);
    m_pOutputSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_pOutputSignalToolBar->setWindowTitle(tr("Control panel output signals"));
    addToolBarBreak(Qt::RightToolBarArea);
    addToolBar(m_pOutputSignalToolBar);

    QLabel* osLabel = new QLabel(m_pOutputSignalToolBar);
    m_pOutputSignalToolBar->addWidget(osLabel);
    osLabel->setText(tr(" Output signals "));
    osLabel->setEnabled(false);

    QComboBox* osCombo = new QComboBox(m_pOutputSignalToolBar);
    m_pOutputSignalToolBar->addWidget(osCombo);
    osCombo->addItem("Don't use");

    // Control panel selecting analog signal
    //
    m_pAnalogSignalToolBar = new QToolBar(this);
    m_pAnalogSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_pAnalogSignalToolBar->setWindowTitle(tr("Control panel selecting analog signal"));
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


//    // Control panel selecting signal of complex comparator
//    //
//    pComplexComporatorSignalToolBar = new QToolBar(this);
//    pSelectComplexComporatorSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
//    pSelectComplexComporatorSignalToolBar->setWindowTitle(tr("Control panel selecting signal of complex comparator"));
//    addToolBarBreak(Qt::TopToolBarArea);
//    addToolBar(pSelectComplexComporatorSignalToolBar);

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createTabPages()
{
    m_pMainTab = new QTabWidget();
    m_pMainTab->setTabPosition(QTabWidget::South);

    for(int t = 0; t < MEASURE_TYPE_COUNT; t++)
    {
        QTableView* pView = new QTableView;
        m_pMainTab->addTab(pView, tr(MeasureTypeStr[t]));

        pView->setFrameStyle(QFrame::NoFrame);

        m_pMeasureItemView[t] = pView;
    }

    setCentralWidget(m_pMainTab);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
    // Search panel measurements
    //
    m_pFindMeasurePanel = new QDockWidget(tr("Search panel measurements"), this);
    m_pFindMeasurePanel->setAllowedAreas(Qt::RightDockWidgetArea);

    m_pFindMeasureView = new QTableView;

    m_pFindMeasurePanel->setWidget(m_pFindMeasureView);
    addDockWidget(Qt::RightDockWidgetArea, m_pFindMeasurePanel);
    m_pFindMeasurePanel->hide();

    QAction* findAction = m_pFindMeasurePanel->toggleViewAction();
    findAction->setText(tr("Find ..."));
    findAction->setShortcut(Qt::CTRL + Qt::Key_F);
    findAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    m_pEditMenu->addAction(findAction);
    m_pMeasureControlToolBar->addAction(findAction);

    // Separator
    //
    m_pViewPanelMenu->addSeparator();

    // Panel signal information
    //
    m_pSignalInfoPanel = new QDockWidget(tr("Panel signal information"), this);
    m_pSignalInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

    m_pSignalInfoView = new QTableView;

    m_pSignalInfoPanel->setWidget(m_pSignalInfoView);
    addDockWidget(Qt::BottomDockWidgetArea, m_pSignalInfoPanel);

    m_pViewPanelMenu->addAction(m_pSignalInfoPanel->toggleViewAction());


    // Panel comparator information
    //
    m_pComparatorInfoPanel = new QDockWidget(tr("Panel comparator information"), this);
    m_pComparatorInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

    m_pComparatorInfoView = new QTableView;

    m_pComparatorInfoPanel->setWidget(m_pComparatorInfoView);
    addDockWidget(Qt::BottomDockWidgetArea, m_pComparatorInfoPanel);

    m_pViewPanelMenu->addAction(m_pComparatorInfoPanel->toggleViewAction());


//    // Panel complex comparator information
//    //
//    m_pComplexComparatorInfoPanel = new QDockWidget(tr("Panel complex comparator information"), this);
//    m_pComplexComparatorInfoPanel->setAllowedAreas(Qt::BottomDockWidgetArea);

//    m_pComplexComparatorInfoView = new QTableView;

//    m_pComplexComparatorInfoPanel->setWidget(m_pComplexComparatorInfoView);
//    addDockWidget(Qt::BottomDockWidgetArea, m_pComplexComparatorInfoPanel);

//    m_pViewPanelMenu->addAction(m_pComplexComparatorInfoPanel->toggleViewAction());

//    m_pComplexComparatorInfoPanel->hide();

}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createStatusBar()
{
    m_statusEmpty = new QLabel(this);
    m_statusMeasureThreadInfo = new QLabel(this);
    m_statusMeasureDemphrer = new QProgressBar(this);
    m_statusMeasureThreadState = new QLabel(this);
    m_statusMeasureThreadState = new QLabel(this);
    m_statusMeasureCount = new QLabel(this);
    m_statusConnectToServer = new QLabel(this);

    m_statusMeasureDemphrer->setRange(0, 100);
    m_statusMeasureDemphrer->setFixedWidth(100);
    m_statusMeasureDemphrer->setFixedHeight(10);
    m_statusMeasureDemphrer->setLayoutDirection(Qt::LeftToRight);

    statusBar()->addWidget(m_statusConnectToServer);
    statusBar()->addWidget(m_statusConnectToServer);
    statusBar()->addWidget(m_statusMeasureCount);
    statusBar()->addWidget(m_statusMeasureThreadState);
    statusBar()->addWidget(m_statusMeasureDemphrer);
    statusBar()->addWidget(m_statusMeasureThreadInfo);
    statusBar()->addWidget(m_statusEmpty);

    statusBar()->setLayoutDirection(Qt::RightToLeft);

    m_statusEmpty->setText(tr(""));
    m_statusMeasureThreadInfo->setText(tr("Measurement 1 of 10 "));
    m_statusMeasureDemphrer->setValue(50);
    m_statusMeasureThreadState->setText(tr("The measurement process is stopped "));
    m_statusMeasureCount->setText(tr("Measurement 0/0 "));
    m_statusConnectToServer->setText(tr("Connect to server: offline "));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::calibrators()
{
    m_calibratorBase.showInitializationWnd();
}

// -------------------------------------------------------------------------------------------------------------------

