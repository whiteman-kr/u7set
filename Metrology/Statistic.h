#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>

#include "SignalBase.h"

#include "../lib/Signal.h"

// ==============================================================================================

const bool              STATISTIC_STATE_INVALID   = false,
                        STATISTIC_STATE_SUCCESS     = true;

// ==============================================================================================

class StatisticItem
{
public:

    explicit            StatisticItem() {}
    explicit            StatisticItem(Hash signalHash) { m_signalHash = signalHash; }

private:


    Hash                m_signalHash = 0;

    int                 m_measureCount = 0;
    bool                m_state = STATISTIC_STATE_SUCCESS;

public:

    Hash                signalHash() const { return m_signalHash; }
    void                setSignalHash(Hash hash) { m_signalHash = hash; }

    int                 incrementMeasureCount() { m_measureCount++; return m_measureCount; }
    int                 measureCount() { return m_measureCount; }

    bool                state() const { return m_state; }
    void                setState(bool state) { m_state = state; }

    QString             stateString();

};

// ==============================================================================================

const char* const       StatisticColumn[] =
{
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "ID"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "EquipmentID"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Caption"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Case"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Subblock"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Block"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Entry"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "ADC range"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Input Ph.range"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Input El.range"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Output Ph.range"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Output El.range"),
                        QT_TRANSLATE_NOOP("StatisticDialog.h", "Measure count"),
};

const int               STATISTIC_COLUMN_COUNT          = sizeof(StatisticColumn)/sizeof(char*);

const int               STATISTIC_COLUMN_ID             = 0,
                        STATISTIC_COLUMN_EQUIPMENT_ID   = 1,
                        STATISTIC_COLUMN_CAPTION        = 2,
                        STATISTIC_COLUMN_CASE           = 3,
                        STATISTIC_COLUMN_SUBBLOCK       = 4,
                        STATISTIC_COLUMN_BLOCK          = 5,
                        STATISTIC_COLUMN_ENTRY          = 6,
                        STATISTIC_COLUMN_ADC            = 7,
                        STATISTIC_COLUMN_IN_PH_RANGE    = 8,
                        STATISTIC_COLUMN_IN_EL_RANGE    = 9,
                        STATISTIC_COLUMN_OUT_PH_RANGE   = 10,
                        STATISTIC_COLUMN_OUT_EL_RANGE   = 11,
                        STATISTIC_COLUMN_MEASURE_COUNT  = 12;

// ==============================================================================================

class StatisticTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            StatisticTable(QObject* parent = 0);
                        ~StatisticTable();

    int                 count() { return m_signalHashList.count(); }
    Hash                at(int index);
    void                set(const QList<Hash> list_add);
    void                clear();

    QString             text(int row, int column) const;

    void                setMeasureType(const int& type) { m_measureType = type; }

    bool                showCustomID() const { return m_showCustomID; }
    void                setShowCustomID(bool show) { m_showCustomID = show; }

    bool                showADCInHex() const { return m_showADCInHex; }
    void                setShowADCInHex(bool show) { m_showADCInHex = show; }

    QMainWindow*        m_pMainWindow;

private:

    int                 m_measureType;

    QList<Hash>         m_signalHashList;

    static bool         m_showCustomID;
    static bool         m_showADCInHex;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

class StatisticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit            StatisticDialog(QWidget *parent = 0);
                        ~StatisticDialog();

private:

    static int          m_columnWidth[STATISTIC_COLUMN_COUNT];

    QMenuBar*           m_pMenuBar = nullptr;
    QMenu*              m_pEditMenu = nullptr;
    QMenu*              m_pViewMenu = nullptr;
    QMenu*              m_pViewMeasureTypeMenu = nullptr;
    QMenu*              m_pViewShowMenu = nullptr;

    QAction*            m_pCopyAction = nullptr;
    QAction*            m_pFindAction = nullptr;
    QAction*            m_pSignalPropertyAction = nullptr;

    QAction*            m_pTypeLinearityAction = nullptr;
    QAction*            m_pTypeComparatorsAction = nullptr;
    QAction*            m_pShowCustomIDAction = nullptr;
    QAction*            m_pShowADCInHexAction = nullptr;

    QTableView*         m_pView = nullptr;
    StatisticTable      m_table;

    QAction*            m_pColumnAction[STATISTIC_COLUMN_COUNT];
    QMenu*              m_headerContextMenu = nullptr;

    static int          m_measureType;

    void                createInterface();
    void                createHeaderContexMenu();

    void                updateVisibleColunm();

    void                hideColumn(int column, bool hide);

protected:

    bool                eventFilter(QObject *object, QEvent *event);

signals:

private slots:

    // slots for updating
    //
    void                updateList();

    // slots of menu
    //
                        // Edit
                        //
    void                copy();
    void                find();
    void                signalProperty();


                        // View
                        //
    void                showTypeLinearity();
    void                showTypeComparators();
    void                showCustomID();
    void                showADCInHex();

    void                onContextMenu(QPoint);

    // slots for list header, to hide or show columns
    //
    void                onHeaderContextMenu(QPoint);
    void                onColumnAction(QAction* action);

    // slots for list
    //
    void                onListDoubleClicked(const QModelIndex&) { signalProperty(); }
};

// ==============================================================================================

#endif // STATISTICDIALOG_H
