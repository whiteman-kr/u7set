#ifndef SIGNALINFOPANEL_H
#define SIGNALINFOPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "Measure.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const       SignalInfoColumn[] =
{
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Case"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "ID"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Sate"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Subblock"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Block"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Entry"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Caption"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Ph. range"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "El. range"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Calibrator"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Out. Ph. range"),
                        QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Out. El. range"),
};

const int               SIGNAL_INFO_COLUMN_COUNT    = sizeof(SignalInfoColumn)/sizeof(char*);

const int               SIGNAL_INFO_COLUMN_CASE         = 0,
                        SIGNAL_INFO_COLUMN_ID           = 1,
                        SIGNAL_INFO_COLUMN_STATE        = 2,
                        SIGNAL_INFO_COLUMN_SUBBLOCK     = 3,
                        SIGNAL_INFO_COLUMN_BLOCK        = 4,
                        SIGNAL_INFO_COLUMN_ENTRY        = 5,
                        SIGNAL_INFO_COLUMN_CAPTION      = 6,
                        SIGNAL_INFO_COLUMN_IN_PH_RANGE  = 7,
                        SIGNAL_INFO_COLUMN_IN_EL_RANGE  = 8,
                        SIGNAL_INFO_COLUMN_CALIBRATOR   = 9,
                        SIGNAL_INFO_COLUMN_OUT_PH_RANGE = 10,
                        SIGNAL_INFO_COLUMN_OUT_EL_RANGE = 11;

// ==============================================================================================

class SignalInfoTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            SignalInfoTable(QObject* parent = 0);
                        ~SignalInfoTable();

    int                 count() { return MAX_CHANNEL_COUNT; }
    Hash                at(int index);
    void                set(const MeasureMultiSignal& multiSignal);
    void                clear();

    QString             text(const int& row, const int& column) const;

    bool                showCustomID() const { return m_showCustomID; }
    void                setShowCustomID(bool show) { m_showCustomID = show; }

    void                updateColumn(const int& column);

private:

    MeasureMultiSignal  m_activeSignal;

    static bool         m_showCustomID;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

#define                 SIGNAL_INFO_OPTIONS_KEY        "Options/SignalInfo/"

// ==============================================================================================

class SignalInfoPanel : public QDockWidget
{
    Q_OBJECT

public:

    explicit            SignalInfoPanel(QWidget* parent = 0);
                        ~SignalInfoPanel();

    void                clear()  { m_table.clear(); }

private:

    QMainWindow*        m_pMainWindow;

    static int          m_columnWidth[SIGNAL_INFO_COLUMN_COUNT];

    // elements of interface
    //
    QMainWindow*        m_pSignalInfoWindow = nullptr;
    QTableView*         m_pView = nullptr;
    SignalInfoTable     m_table;

    QMenu*              m_pContextMenu = nullptr;
    QAction*            m_pCopyAction = nullptr;
    QAction*            m_pShowCustomIDAction = nullptr;
    QAction*            m_pSignalPropertyAction = nullptr;

    QAction*            m_pColumnAction[SIGNAL_INFO_COLUMN_COUNT];
    QMenu*              m_headerContextMenu = nullptr;

    void                createInterface();
    void                createHeaderContexMenu();
    void                createContextMenu();

    void                hideColumn(int column, bool hide);

    QTimer*             m_updateSignalStateTimer = nullptr;
    void                startSignalStateTimer();
    void                stopSignalStateTimer();

protected:

    bool                eventFilter(QObject *object, QEvent *event);

public slots:

    // slot informs that signal for measure was selected
    //
    void                onSetActiveSignal();

    // slot informs that signal for measure has updated his state
    //
    void                updateStateActiveSignal();

private slots:

    // slots of menu
    //
    void                copy();
    void                showCustomID();
    void                signalProperty();

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

#endif // SIGNALINFOPANELPANEL_H
