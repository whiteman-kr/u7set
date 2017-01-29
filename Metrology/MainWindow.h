#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QToolBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>

#include "MeasureView.h"
#include "SignalSocket.h"
#include "MeasureThread.h"
#include "FindMeasurePanel.h"
#include "SignalInfoPanel.h"
#include "Calculator.h"

#include "../lib/SimpleThread.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit            MainWindow(QWidget *parent = 0);
                        ~MainWindow();

    int                 measureType() const { return m_measureType; }

    // Elements of interface - Menu
    //
    QMenu*              m_pMeasureMenu = nullptr;
    QMenu*              m_pEditMenu = nullptr;
    QMenu*              m_pViewMenu = nullptr;
    QMenu*              m_pViewPanelMenu = nullptr;
    QMenu*              m_pSettingMenu = nullptr;
    QMenu*              m_pInfoMenu = nullptr;

    // Elements of interface - ToolBar
    //
    QToolBar*           m_pMeasureControlToolBar = nullptr;
    QToolBar*           m_pMeasureTimeout = nullptr;
    QToolBar*           m_pMeasureKind = nullptr;
    QToolBar*           m_pOutputSignalToolBar = nullptr;
    QToolBar*           m_pAnalogSignalToolBar = nullptr;
    QToolBar*           m_pComplexComporatorToolBar = nullptr;

    // Elements of interface - Items of ToolBars
    //
    QComboBox*          m_outputSignalTypeList;

    QComboBox*          m_asCaseTypeCombo;
    QComboBox*          m_asMeasureSignalCombo;
    QComboBox*          m_asCaseNoCombo;
    QComboBox*          m_asSubblockCombo;
    QComboBox*          m_asBlockCombo;
    QComboBox*          m_asEntryCombo;

    // Elements of interface - Pages of Tab
    //
    QTabWidget*         m_pMainTab = nullptr;

    // Elements of interface - Panels
    //
    FindMeasurePanel*   m_pFindMeasurePanel = nullptr;
    SignalInfoPanel*    m_pSignalInfoPanel = nullptr;
    QDockWidget*        m_pComparatorInfoPanel = nullptr;
    QTableView*         m_pComparatorInfoView = nullptr;
    QDockWidget*        m_pComplexComparatorInfoPanel = nullptr;
    QTableView*         m_pComplexComparatorInfoView = nullptr;


    // Elements of interface - StatusBar
    //
    QLabel*             m_statusEmpty = nullptr;
    QLabel*             m_statusMeasureThreadInfo = nullptr;
    QProgressBar*       m_statusMeasureTimeout = nullptr;
    QLabel*             m_statusMeasureThreadState = nullptr;
    QLabel*             m_statusCalibratorCount = nullptr;
    QLabel*             m_statusConnectToServer = nullptr;

public:

    SignalSocket*       m_pSignalSocket = nullptr;
    SimpleThread*       m_pSignalSocketThread = nullptr;

    MeasureThread       m_measureThread;

    Calculator*         m_pCalculator = nullptr;

    void                loadSettings();
    void                saveSettings();

protected:

    void                closeEvent(QCloseEvent* e);

public:

    bool                createInterface();

    void                createActions();
    void                updateStartStopActions();
    void                createMenu();
    bool                createToolBars();
    void                createMeasurePages();
    void                createPanels();
    void                createStatusBar();
    void                createContextMenu();

    void                updateAnalogSignalToolBar();

    MeasureView*        activeMeasureView() { return measureView(m_measureType); }
    MeasureView*        measureView(const int measureType);
    void                appendMeasureView(const int measureType, MeasureView* pView);

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    QMap<int, MeasureView*> m_measureViewMap;

    // Actions of main menu
    //

    // menu - Measure
    //
    QAction*            m_pStartMeasureAction = nullptr;
    QAction*            m_pStopMeasureAction = nullptr;
    QAction*            m_pPrintMeasureAction = nullptr;
    QAction*            m_pExportMeasureAction = nullptr;

    // menu - Edit
    //
    QAction*            m_pCopyMeasureAction = nullptr;
    QAction*            m_pRemoveMeasureAction = nullptr;
    QAction*            m_pSelectAllMeasureAction = nullptr;

    // menu - View
    //
    QAction*            m_pShowReportsAction = nullptr;
    QAction*            m_pShowCalculatorAction = nullptr;

    // menu - Tools
    //
    QAction*            m_pCalibratorsAction = nullptr;
    QAction*            m_pShowSignalListAction = nullptr;
    QAction*            m_pShowComparatorsListAction = nullptr;
    QAction*            m_pShowOutputSignalListAction = nullptr;
    QAction*            m_pShowOutputRangeListAction = nullptr;
    QAction*            m_pShowComlexComparatorListAction = nullptr;
    QAction*            m_pOptionsAction;

    // menu - ?
    //
    QAction*            m_pShowStatisticAction = nullptr;
    QAction*            m_pAboutConnectionAction = nullptr;
    QAction*            m_pAboutAppAction = nullptr;

    QMenu*              m_pContextMenu = nullptr;

signals:

    void                appendMeasure(Measurement*);

    void                setActiveSignal();

private slots:

    // Slots of main menu
    //

    // menu - Measure
    //
    void                startMeasure();
    void                stopMeasure();
    void                printMeasure();
    void                exportMeasure();

    // menu - Edit
    //
    void                copyMeasure();
    void                removeMeasure();
    void                selectAllMeasure();

    // menu - View
    //
    void                showReports() {};
    void                showCalculator();

    // menu - Tools
    //
    void                calibrators();
    void                showSignalList();
    void                showComparatorsList() {};
    void                showOutputSignalList();
    void                showOutputRangeList() {};
    void                showComlexComparatorList() {};
    void                options();

    // menu - ?
    //
    void                showStatistic();
    void                aboutConnection() {};
    void                aboutApp() {};

    // Slots of tab -- page measure type
    //
    void                setMeasureType(const int measureType);
    void                measureCountChanged(int);

    // Slots of control panels
    //
    void                setMeasureKind(int index);
    void                setMeasureTimeout(QString value);
    void                setOutputSignalType(int index);

    // Slots of analog signal toolbar
    //
    void                setCaseType(int index);
    void                setMeasureSignal(int index);
    void                setCaseNo(int index);
    void                setSubblock(int index);
    void                setBlock(int index);
    void                setEntry(int index);
    void                setMeasureSignalByPosition(int index);

    // Slots of contex menu
    //
    void                onContextMenu(QPoint);

    // Slots of calibrator base
    //
    void                calibratorConnectedChanged(int);

    // Slots of socket for signals
    //
    void                signalSocketConnected();
    void                signalSocketDisconnected();
    void                signalSocketSignalsLoaded();

    // Slots of measure thread
    //
    void                measureThreadStarted();
    void                measureThreadStoped();
    void                setMeasureThreadInfo(QString msg);
    void                setMeasureThreadInfo(int timeout);
    void                measureComplite(Measurement* pMeasurement);
};

// ==============================================================================================

#endif // MAINWINDOW_H
