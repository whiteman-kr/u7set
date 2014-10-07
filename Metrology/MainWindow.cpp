#include "MainWindow.h"

#include <QDebug>
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
#include "OptionsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    theCalibratorBase.init(this);

    createInterface();

    initMeasureThread();
}

// -------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MainWindow::createInterface()
{
    setWindowTitle(tr("Metrology"));
    restoreWindowPosition(this);

    createActions();
    createMenu();
    createToolBars();
    createTabPages();
    createPanels();
    createStatusBar();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::initMeasureThread()
{
    connect(&MeasureThread, &MeasureThread::started, this, &MainWindow::onMeasureThreadStarted, Qt::QueuedConnection );
    connect(&MeasureThread, &MeasureThread::finished, this, &MainWindow::onMeasureThreadStoped, Qt::QueuedConnection );
    connect(&MeasureThread, SIGNAL(measureInfo(QString)), this, SLOT(onMeasureThreadInfo(QString)), Qt::QueuedConnection );
    connect(&MeasureThread, SIGNAL(measureInfo(int)), this, SLOT(onMeasureThreadInfo(int)), Qt::QueuedConnection );

    onMeasureThreadStoped();
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

    m_pPrintMeasureAction = new QAction(tr("&Print ..."), this);
    m_pPrintMeasureAction->setShortcut(Qt::CTRL + Qt::Key_P);
    m_pPrintMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
    connect(m_pPrintMeasureAction, &QAction::triggered, this, &MainWindow::printMeasure);

    m_pExportMeasureAction = new QAction(tr("&Export ..."), this);
    m_pExportMeasureAction->setShortcut(Qt::CTRL + Qt::Key_E);
    m_pExportMeasureAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));
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

    m_pSelectAllMeasureAction = new QAction(tr("Select &All"), this);
    m_pSelectAllMeasureAction->setShortcut(Qt::CTRL + Qt::Key_A);
    connect(m_pSelectAllMeasureAction, &QAction::triggered, this, &MainWindow::selectAllMeasure);

    // View
    //
    m_pShowReportsAction = new QAction(tr("&Reports ..."), this);
    m_pShowReportsAction->setShortcut(Qt::CTRL + Qt::Key_R);
    connect(m_pShowReportsAction, &QAction::triggered, this, &MainWindow::showReports);

    m_pShowCalculateAction = new QAction(tr("Metrological &calculator ..."), this);
    m_pShowCalculateAction->setShortcut(Qt::ALT + Qt::Key_C);
    connect(m_pShowCalculateAction, &QAction::triggered, this, &MainWindow::showCalculate);

    // Tools
    //
    m_pCalibratorsAction = new QAction(tr("&Calibrations ..."), this);
    connect(m_pCalibratorsAction, &QAction::triggered, this, &MainWindow::calibrators);

    m_pShowOutputSignalListAction = new QAction(tr("Signals input/output ..."), this);
    connect(m_pShowOutputSignalListAction, &QAction::triggered, this, &MainWindow::showOutputSignalList);

//    m_pShowComlexComparatorListAction = new QAction(tr("&Signals of complex comparator..."), this);
//    connect(m_pShowComlexComparatorListAction, &QAction::triggered, this, &MainWindow::showComlexComparatorList);

    m_pShowOutputRangeListAction = new QAction(tr("Signals with output ranges ..."), this);
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
//    m_pSettingMenu->addAction(m_pShowComlexComparatorListAction);
    m_pSettingMenu->addAction(m_pShowOutputRangeListAction);
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

        m_pMeasureControlToolBar->setIconSize(QSize(16,16));
    }

    // Control panel measure timeout
    //
    m_pMeasureTimeout = new QToolBar(this);
    if (m_pMeasureTimeout != nullptr)
    {
        m_pMeasureTimeout->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pMeasureTimeout->setWindowTitle(tr("Control panel measure timeout"));
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

        measureTimeoutList->setCurrentText(QString::number(double(theOptions.getToolBar().m_measureTimeout) / 1000, 10, 1));

        measureTimeoutUnitLabel->setText(tr(" sec."));
        measureTimeoutUnitLabel->setEnabled(false);

        connect(measureTimeoutList, &QComboBox::currentTextChanged, this, &MainWindow::setMeasureTimeout, Qt::QueuedConnection);
    }


    // Control panel measure kind
    //
    m_pMeasureKind = new QToolBar(this);
    if (m_pMeasureKind != nullptr)
    {
        m_pMeasureKind->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pMeasureKind->setWindowTitle(tr("Control panel measure kind"));
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

        measureKindList->setCurrentIndex(theOptions.getToolBar().m_measureKind);

        connect(measureKindList, SIGNAL(currentIndexChanged(int)), this, SLOT(setMeasureKind(int)), Qt::QueuedConnection);
    }


    // Control panel output signals
    //
    m_pOutputSignalToolBar = new QToolBar(this);
    if (m_pOutputSignalToolBar != nullptr)
    {
        m_pOutputSignalToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        m_pOutputSignalToolBar->setWindowTitle(tr("Control panel output signals"));
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
        outputSignalList->setCurrentIndex(theOptions.getToolBar().m_outputSignalType);

        connect(outputSignalList, SIGNAL(currentIndexChanged(int)), this, SLOT(setOutputSignalType(int)), Qt::QueuedConnection);
    }

    // Control panel selecting analog signal
    //
    m_pAnalogSignalToolBar = new QToolBar(this);
    if (m_pAnalogSignalToolBar != nullptr)
    {
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
    }


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
        if (pView != nullptr)
        {
            m_pMainTab->addTab(pView, tr(MeasureType[t]));

            pView->setFrameStyle(QFrame::NoFrame);

            m_measureView.append(pView);
        }
    }

    setCentralWidget(m_pMainTab);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::createPanels()
{
    // Search panel measurements
    //
    m_pFindMeasurePanel = new QDockWidget(tr("Search panel measurements"), this);
    if (m_pFindMeasurePanel != nullptr)
    {
        m_pFindMeasurePanel->setAllowedAreas(Qt::RightDockWidgetArea);

        m_pFindMeasureView = new QTableView;
        if (m_pFindMeasureView != nullptr)
        {
            m_pFindMeasurePanel->setWidget(m_pFindMeasureView);
        }

        addDockWidget(Qt::RightDockWidgetArea, m_pFindMeasurePanel);
        m_pFindMeasurePanel->hide();


        QAction* findAction = m_pFindMeasurePanel->toggleViewAction();
        if (findAction != nullptr)
        {
            findAction->setText(tr("&Find ..."));
            findAction->setShortcut(Qt::CTRL + Qt::Key_F);
            findAction->setIcon(QIcon::fromTheme("empty", QIcon(":/icons/win/empty.png")));

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
    }

    // Panel comparator information
    //
    m_pComparatorInfoPanel = new QDockWidget(tr("Panel comparator information"), this);
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
    }


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
    QStatusBar* pStatusBar = statusBar();
    if (pStatusBar == nullptr)
    {
        return;
    }

    m_statusEmpty = new QLabel(pStatusBar);
    m_statusMeasureThreadInfo = new QLabel(pStatusBar);
    m_statusMeasureTimeout = new QProgressBar(pStatusBar);
    m_statusMeasureThreadState = new QLabel(pStatusBar);
    m_statusMeasureThreadState = new QLabel(pStatusBar);
    m_statusMeasureCount = new QLabel(pStatusBar);
    m_statusConnectToServer = new QLabel(pStatusBar);

    m_statusMeasureTimeout->setTextVisible(false);
    m_statusMeasureTimeout->setRange(0, 100);
    m_statusMeasureTimeout->setFixedWidth(100);
    m_statusMeasureTimeout->setFixedHeight(10);
    m_statusMeasureTimeout->setLayoutDirection(Qt::LeftToRight);

    pStatusBar->addWidget(m_statusConnectToServer);
    pStatusBar->addWidget(m_statusConnectToServer);
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

void MainWindow::startMeasure()
{
    if (MeasureThread.isRunning() == true)
    {
        return;
    }

    int type = m_pMainTab->currentIndex();
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    MeasureThread.setMeasureType(type);

    MeasureThread.start();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::stopMeasure()
{
    if (MeasureThread.isFinished() == true)
    {
        return;
    }

    MeasureThread.stop();
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
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureKind(int index)
{
    int kind = index;
    if (kind < 0 || kind >= MEASURE_KIND_COUNT)
    {
        return;
    }

    theOptions.getToolBar().m_measureKind = kind;
    theOptions.getToolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setMeasureTimeout(QString value)
{
    theOptions.getToolBar().m_measureTimeout = value.toDouble() * 1000;
    theOptions.getToolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::setOutputSignalType(int index)
{
    int type = index;
    if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    theOptions.getToolBar().m_outputSignalType = type;
    theOptions.getToolBar().save();
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onMeasureThreadStarted()
{
    m_pMeasureKind->setDisabled(true);
    m_pOutputSignalToolBar->setDisabled(true);
    m_pAnalogSignalToolBar->setDisabled(true);
    //m_pComplexComporatorSignalToolBar->setDisabled(true);

    m_statusMeasureThreadInfo->setText("");

    if (theOptions.getToolBar().m_measureTimeout != 0)
    {
        m_statusMeasureTimeout->show();
        m_statusMeasureTimeout->setRange(0, theOptions.getToolBar().m_measureTimeout);
        m_statusMeasureTimeout->setValue(0);
    }
    else
    {
         m_statusMeasureTimeout->hide();
    }

    m_statusMeasureThreadState->setText(tr("The measurement process is started "));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onMeasureThreadStoped()
{
    m_pMeasureKind->setEnabled(true);
    m_pOutputSignalToolBar->setEnabled(true);
    m_pAnalogSignalToolBar->setEnabled(true);
    //m_pComplexComporatorSignalToolBar->setEnabled(true);

    m_statusMeasureThreadInfo->setText("");

    m_statusMeasureTimeout->hide();
    m_statusMeasureTimeout->setRange(0, theOptions.getToolBar().m_measureTimeout);
    m_statusMeasureTimeout->setValue(0);

    m_statusMeasureThreadState->setText(tr("The measurement process is stopped "));
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onMeasureThreadInfo(QString msg)
{
    m_statusMeasureThreadInfo->setText(msg);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::onMeasureThreadInfo(int timeout)
{
    m_statusMeasureTimeout->setValue(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (MeasureThread.isRunning() == true)
    {
        QMessageBox msg;
        msg.setText(m_statusMeasureThreadState->text());
        msg.exec();

        e->ignore();

        return;
    }

    theCalibratorBase.clear();

    saveWindowPosition(this);

    QMainWindow::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
