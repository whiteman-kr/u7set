#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>

#include "Measure.h"

// ==============================================================================================

namespace Ui
{
    class MainWindow;
}

// ==============================================================================================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    // Элементы интерфейса - меню
    //
    QMenu*          m_pMeasureMenu;
    QMenu*          m_pEditMenu;
    QMenu*          m_pViewMenu;
    QMenu*          m_pViewPanelMenu;
    QMenu*          m_pSettingMenu;
    QMenu*          m_pInfoMenu;

    // Элементы интерфейса - тулбар
    //
    QToolBar*       m_pMeasureControlToolBar;
    QToolBar*       m_pMeasureDempher;
    QToolBar*       m_pOutputSignalToolBar;
    QToolBar*       m_pAnalogSignalToolBar;
    QToolBar*       m_pComplexComporatorSignalToolBar;

    // Элементы интерфейса - страницы таба
    //
    QTabWidget*     m_pMainTab;
    QTableView*     m_pMeasureItemView[MEASURE_TYPE_COUNT];

    // Элементы интерфейса - панели
    //
    QDockWidget*    m_pFindMeasurePanel;
    QTableView*     m_pFindMeasureView;
    QDockWidget*    m_pSignalInfoPanel;
    QTableView*     m_pSignalInfoView;
    QDockWidget*    m_pComparatorInfoPanel;
    QTableView*     m_pComparatorInfoView;
    QDockWidget*    m_pComplexComparatorInfoPanel;
    QTableView*     m_pComplexComparatorInfoView;


    // Элементы интерфейса - строка состояния
    //
    QLabel*         m_statusEmpty;
    QLabel*         m_statusMeasureThreadInfo;
    QProgressBar*   m_statusMeasureDemphrer;
    QLabel*         m_statusMeasureThreadState;
    QLabel*         m_statusMeasureCount;
    QLabel*         m_statusConnectToServer;



    // Создание интерфейса
    //
    bool createInterface();

    void createActions();
    void createMenu();
    bool createToolBars();
    void createTabPages();
    void createPanels();
    void createStatusBar();


private:

    Ui::MainWindow *ui;

    // Actions меню
    //
private:

    // Измерения
    //
    QAction* m_pStartMeasureAction;
    QAction* m_pStopMeasureAction;
    QAction* m_pExportMeasureAction;

    // Правка
    //
    QAction* m_pCutMeasureAction;
    QAction* m_pCopyMeasureAction;
    QAction* m_pRemoveMeasureAction;
    QAction* m_pSelectAllMeasureAction;

    // Вид
    //
    QAction* m_pShowReportsAction;
    QAction* m_pShowCalculateAction;

    // Настройки
    //
    QAction* m_pConnectToServerAction;
    QAction* m_pCalibrationAction;
    QAction* m_pShowOutputSignalListAction;
    //QAction* m_pShowComlexComparatorListAction;
    QAction* m_pShowOutputRangeListAction;
    QAction* m_pOptionsAction;

    // ?
    //
    QAction* m_pShowSignalListAction;
    QAction* m_pShowComparatorsListAction;
    QAction* m_pShowCorrecrtionsListAction;
    QAction* m_pShowStatisticAction;
    QAction* m_pAboutConnectionAction;
    QAction* m_pAboutAppAction;



    // Slots меню
    //
public slots:

    // Измерения
    //
    void startMeasure() {};
    void stopMeasure() {};
    void exportMeasure() {};

    // Правка
    //
    void cutMeasure() {};
    void copyMeasure() {};
    void removeMeasure() {};
    void selectAllMeasure() {};

    // Вид
    //
    void showReports() {};
    void showCalculate() {};

    // Настройки
    //
    void connectToServer() {};
    void calibration() {};
    void showOutputSignalList() {};
    //void showComlexComparatorList() {};
    void showOutputRangeList() {};
    void options() {};

    // ?
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
