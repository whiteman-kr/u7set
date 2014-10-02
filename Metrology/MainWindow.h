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

#include "MeasureThread.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

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
    QToolBar*           m_pOutputSignalToolBar = nullptr;
    QToolBar*           m_pAnalogSignalToolBar = nullptr;
    QToolBar*           m_pComplexComporatorSignalToolBar = nullptr;

    // Elements of interface - Pages of Tab
    //
    QTabWidget*         m_pMainTab = nullptr;
    QList<QTableView*>  m_measureView;

    // Elements of interface - Panels
    //
    QDockWidget*        m_pFindMeasurePanel = nullptr;
    QTableView*         m_pFindMeasureView = nullptr;
    QDockWidget*        m_pSignalInfoPanel = nullptr;
    QTableView*         m_pSignalInfoView = nullptr;
    QDockWidget*        m_pComparatorInfoPanel = nullptr;
    QTableView*         m_pComparatorInfoView = nullptr;
    QDockWidget*        m_pComplexComparatorInfoPanel = nullptr;
    QTableView*         m_pComplexComparatorInfoView = nullptr;


    // Elements of interface - StatusBar
    //
    QLabel*             m_statusEmpty = nullptr;
    QLabel*             m_statusMeasureThreadInfo = nullptr;
    QProgressBar*       m_statusMeasureDemphrer = nullptr;
    QLabel*             m_statusMeasureThreadState = nullptr;
    QLabel*             m_statusMeasureCount = nullptr;
    QLabel*             m_statusConnectToServer = nullptr;

public:

    MeasureThread   m_measureThread;        // Measure thread

protected:

    void            closeEvent(QCloseEvent* e);

public:

    // Create interface
    //
    bool createInterface();

    void createActions();
    void createMenu();
    bool createToolBars();
    void createTabPages();
    void createPanels();
    void createStatusBar();

    // Actions of main menu
    //
private:

    // menu - Measure
    //
    QAction* m_pStartMeasureAction = nullptr;
    QAction* m_pStopMeasureAction = nullptr;
    QAction* m_pExportMeasureAction = nullptr;

    // menu - Edit
    //
    QAction* m_pCutMeasureAction = nullptr;
    QAction* m_pCopyMeasureAction = nullptr;
    QAction* m_pRemoveMeasureAction = nullptr;
    QAction* m_pSelectAllMeasureAction = nullptr;

    // menu - View
    //
    QAction* m_pShowReportsAction = nullptr;
    QAction* m_pShowCalculateAction = nullptr;

    // menu - Tools
    //
    QAction* m_pConnectToServerAction = nullptr;
    QAction* m_pCalibratorsAction = nullptr;
    QAction* m_pShowOutputSignalListAction = nullptr;
    //QAction* m_pShowComlexComparatorListAction = nullptr;
    QAction* m_pShowOutputRangeListAction = nullptr;
    QAction* m_pOptionsAction;

    // menu - ?
    //
    QAction* m_pShowSignalListAction = nullptr;
    QAction* m_pShowComparatorsListAction = nullptr;
    QAction* m_pShowCorrecrtionsListAction = nullptr;
    QAction* m_pShowStatisticAction = nullptr;
    QAction* m_pAboutConnectionAction = nullptr;
    QAction* m_pAboutAppAction = nullptr;


    // Slots of main menu
    //
public slots:

    // menu - Measure
    //
    void startMeasure();
    void stopMeasure();
    void exportMeasure() {};

    // menu - Edit
    //
    void cutMeasure() {};
    void copyMeasure() {};
    void removeMeasure() {};
    void selectAllMeasure() {};

    // menu - View
    //
    void showReports() {};
    void showCalculate() {};

    // menu - Tools
    //
    void connectToServer() {};
    void calibrators();
    void showOutputSignalList() {};
    //void showComlexComparatorList() {};
    void showOutputRangeList() {};
    void options();

    // menu - ?
    //
    void showSignalList() {};
    void showComparatorsList() {};
    void showCorrecrtionsList() {};
    void showStatistic() {};
    void aboutConnection() {};
    void aboutApp() {};

};

// ==============================================================================================

#endif // MAINWINDOW_H
