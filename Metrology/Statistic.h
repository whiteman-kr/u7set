#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QStatusBar>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const           StatisticColumn[] =
{
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Case"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "ID"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "EquipmentID"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Caption"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Subblock"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Block"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Entry"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "ADC range"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Input Ph.range"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Input El.range"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Output type"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Output Ph.range"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Output El.range"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "Measure count"),
                            QT_TRANSLATE_NOOP("StatisticDialog.h", "State"),

};

const int                   STATISTIC_COLUMN_COUNT          = sizeof(StatisticColumn)/sizeof(StatisticColumn[0]);

const int                   STATISTIC_COLUMN_CASE           = 0,
                            STATISTIC_COLUMN_ID             = 1,
                            STATISTIC_COLUMN_EQUIPMENT_ID   = 2,
                            STATISTIC_COLUMN_CAPTION        = 3,
                            STATISTIC_COLUMN_SUBBLOCK       = 4,
                            STATISTIC_COLUMN_BLOCK          = 5,
                            STATISTIC_COLUMN_ENTRY          = 6,
                            STATISTIC_COLUMN_ADC            = 7,
                            STATISTIC_COLUMN_IN_PH_RANGE    = 8,
                            STATISTIC_COLUMN_IN_EL_RANGE    = 9,
                            STATISTIC_COLUMN_OUTPUT_TYPE    = 10,
                            STATISTIC_COLUMN_OUT_PH_RANGE   = 11,
                            STATISTIC_COLUMN_OUT_EL_RANGE   = 12,
                            STATISTIC_COLUMN_MEASURE_COUNT  = 13,
                            STATISTIC_COLUMN_STATE          = 14;

// ==============================================================================================

class StatisticTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit                StatisticTable(QObject* parent = 0);
                            ~StatisticTable();

    int                     signalCount() const;
    SignalParam             signalParam(int index) const;
    void                    set(const QList<SignalParam> list_add);
    void                    clear();

    QString                 text(int row, int column, const SignalParam& param) const;

    bool                    showCustomID() const { return m_showCustomID; }
    void                    setShowCustomID(bool show) { m_showCustomID = show; }

    bool                    showADCInHex() const { return m_showADCInHex; }
    void                    setShowADCInHex(bool show) { m_showADCInHex = show; }

private:

    mutable QMutex          m_signalMutex;
    QList<SignalParam>      m_signalParamList;

    static bool             m_showCustomID;
    static bool             m_showADCInHex;

    int                     columnCount(const QModelIndex &parent) const;
    int                     rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant                headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant                data(const QModelIndex &index, int role) const;

private slots:

    void                    updateSignalParam(const Hash& signalHash);
};

// ==============================================================================================

class StatisticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                StatisticDialog(QWidget *parent = 0);
                            ~StatisticDialog();

private:

    QMainWindow*            m_pMainWindow = nullptr;

    static int              m_columnWidth[STATISTIC_COLUMN_COUNT];

    QMenuBar*               m_pMenuBar = nullptr;
    QMenu*                  m_pSignalMenu = nullptr;
    QMenu*                  m_pEditMenu = nullptr;
    QMenu*                  m_pViewMenu = nullptr;
    QMenu*                  m_pViewMeasureTypeMenu = nullptr;
    QMenu*                  m_pViewShowMenu = nullptr;
    QMenu*                  m_pViewGotoMenu = nullptr;
    QMenu*                  m_pContextMenu = nullptr;

    QAction*                m_pPrintAction = nullptr;
    QAction*                m_pExportAction = nullptr;
    QAction*                m_pSelectSignalForMeasure = nullptr;

    QAction*                m_pFindAction = nullptr;
    QAction*                m_pCopyAction = nullptr;
    QAction*                m_pSelectAllAction = nullptr;

    QAction*                m_pTypeLinearityAction = nullptr;
    QAction*                m_pTypeComparatorsAction = nullptr;
    QAction*                m_pShowCustomIDAction = nullptr;
    QAction*                m_pShowADCInHexAction = nullptr;
    QAction*                m_pGotoNextNotMeasuredAction = nullptr;
    QAction*                m_pGotoNextInvalidAction = nullptr;

    QStatusBar*             m_pStatusBar;
    QLabel*                 m_statusEmpty = nullptr;
    QLabel*                 m_statusMeasureInavlid = nullptr;
    QLabel*                 m_statusMeasured = nullptr;


    QTableView*             m_pView = nullptr;
    StatisticTable          m_signalParamTable;

    QAction*                m_pColumnAction[STATISTIC_COLUMN_COUNT];
    QMenu*                  m_headerContextMenu = nullptr;

    static int              m_measureType;

    int                     m_MeasuredCount = 0;
    int                     m_invalidMeasureCount = 0;

    void                    createInterface();
    void                    createHeaderContexMenu();
    void                    createContextMenu();
    void                    createStatusBar();
    void                    updateStatusBar();

    void                    updateVisibleColunm();
    void                    hideColumn(int column, bool hide);

protected:

    bool                    eventFilter(QObject *object, QEvent *event);

signals:

private slots:

    // slots for updating
    //
    void                    updateList();

    // slots of menu
    //
                            // Signal
                            //
    void                    printSignal();
    void                    exportSignal();
    void                    selectSignalForMeasure();

                            // Edit
                            //
    void                    find();
    void                    copy();
    void                    selectAll() { m_pView->selectAll(); }

                            // View
                            //
    void                    showTypeLinearity();
    void                    showTypeComparators();
    void                    showCustomID();
    void                    showADCInHex();
    void                    gotoNextNotMeasured();
    void                    gotoNextInvalid();


    void                    onContextMenu(QPoint);

    // slots for list header, to hide or show columns
    //
    void                    onHeaderContextMenu(QPoint);
    void                    onColumnAction(QAction* action);

    // slots for list
    //
    void                    onListDoubleClicked(const QModelIndex&) { }
};

// ==============================================================================================

#endif // STATISTICDIALOG_H