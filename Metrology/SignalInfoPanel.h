#ifndef SIGNALINFOPANEL_H
#define SIGNALINFOPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "Measure.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const           SignalInfoColumn[] =
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

const int                   SIGNAL_INFO_COLUMN_COUNT    = sizeof(SignalInfoColumn)/sizeof(SignalInfoColumn[0]);

const int                   SIGNAL_INFO_COLUMN_CASE         = 0,
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

    explicit                SignalInfoTable(QObject* parent = 0);
                            ~SignalInfoTable();

    int                     signalCount() const { return MAX_CHANNEL_COUNT; }
    SignalParam             signalParam(int index) const;
    void                    set(const MetrologyMultiSignal& activeSignal);
    void                    clear();

    QString                 text(int row, int column, const SignalParam& param, const AppSignalState& state) const;
    QString                 signalStateStr(const SignalParam& param, const AppSignalState& state) const;

    void                    updateColumn(int column);

private:

    mutable QMutex          m_signalMutex;
    SignalParam             m_activeSignalParam[MAX_CHANNEL_COUNT];

    int                     columnCount(const QModelIndex &parent) const;
    int                     rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant                headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant                data(const QModelIndex &index, int role) const;

private slots:

    void                    updateSignalParam(const Hash& signalHash);
};

// ==============================================================================================

#define                     SIGNAL_INFO_OPTIONS_KEY        "Options/SignalInfo/"

// ==============================================================================================

class SignalInfoPanel : public QDockWidget
{
    Q_OBJECT

public:

    explicit                SignalInfoPanel(QWidget* parent = 0);
                            ~SignalInfoPanel();

    void                    clear()  { m_signalParamTable.clear(); }

private:

    static int              m_columnWidth[SIGNAL_INFO_COLUMN_COUNT];

    // elements of interface
    //
    QMainWindow*            m_pSignalInfoWindow = nullptr;
    QTableView*             m_pView = nullptr;
    SignalInfoTable         m_signalParamTable;

    QMenu*                  m_pShowMenu = nullptr;
    QMenu*                  m_pContextMenu = nullptr;
    QAction*                m_pCopyAction = nullptr;
    QAction*                m_pShowCustomIDAction = nullptr;
    QAction*                m_pShowElectricValueAction = nullptr;
    QAction*                m_pShowAdcValueAction = nullptr;
    QAction*                m_pShowAdcHexValueAction = nullptr;
    QAction*                m_pSignalPropertyAction = nullptr;

    QAction*                m_pColumnAction[SIGNAL_INFO_COLUMN_COUNT];
    QMenu*                  m_headerContextMenu = nullptr;

    void                    createInterface();
    void                    createHeaderContexMenu();
    void                    createContextMenu();

    void                    hideColumn(int column, bool hide);

    QTimer*                 m_updateSignalStateTimer = nullptr;
    void                    startSignalStateTimer();
    void                    stopSignalStateTimer();

protected:

    bool                    eventFilter(QObject *object, QEvent *event);

public slots:

    // slot informs that signal for measure was selected
    //
    void                    onSetActiveSignal();

    // slot informs that signal for measure has updated his state
    //
    void                    updateSignalState();

private slots:

    // slots of menu
    //
    void                    copy();
    void                    showCustomID();
    void                    showElectricValue();
    void                    showAdcValue();
    void                    showAdcHexValue();
    void                    signalProperty();

    void                    onContextMenu(QPoint);

    // slots for list header, to hide or show columns
    //
    void                    onHeaderContextMenu(QPoint);
    void                    onColumnAction(QAction* action);
    // slots for list
    //
    void                    onListDoubleClicked(const QModelIndex&) { signalProperty(); }
};

// ==============================================================================================

#endif // SIGNALINFOPANELPANEL_H
