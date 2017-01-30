#ifndef SIGNALLISTDIALOG_H
#define SIGNALLISTDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>

#include "SignalBase.h"

#include "../lib/Signal.h"

// ==============================================================================================

const char* const       SignalListColumn[] =
{
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "ID"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "EquipmentID"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Caption"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Case"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Subblock"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Block"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Entry"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "ADC range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Input Ph.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Input El.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Output Ph.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Output El.range"),
};

const int               SIGNAL_LIST_COLUMN_COUNT        = sizeof(SignalListColumn)/sizeof(SignalListColumn[0]);

const int               SIGNAL_LIST_COLUMN_ID           = 0,
                        SIGNAL_LIST_COLUMN_EQUIPMENT_ID = 1,
                        SIGNAL_LIST_COLUMN_CAPTION      = 2,
                        SIGNAL_LIST_COLUMN_CASE         = 3,
                        SIGNAL_LIST_COLUMN_SUBBLOCK     = 4,
                        SIGNAL_LIST_COLUMN_BLOCK        = 5,
                        SIGNAL_LIST_COLUMN_ENTRY        = 6,
                        SIGNAL_LIST_COLUMN_ADC          = 7,
                        SIGNAL_LIST_COLUMN_IN_PH_RANGE  = 8,
                        SIGNAL_LIST_COLUMN_IN_EL_RANGE  = 9,
                        SIGNAL_LIST_COLUMN_OUT_PH_RANGE = 10,
                        SIGNAL_LIST_COLUMN_OUT_EL_RANGE = 11;

// ==============================================================================================

class SignalListTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            SignalListTable(QObject* parent = 0);
                        ~SignalListTable();

    int                 signalCount() const;
    MeasureSignalParam  signalParam(const int index) const;
    void                set(const QList<MeasureSignalParam> list_add);
    void                clear();

    QString             text(const int row, const int column, const MeasureSignalParam& param) const;

    bool                showCustomID() const { return m_showCustomID; }
    void                setShowCustomID(const bool show) { m_showCustomID = show; }

    bool                showADCInHex() const { return m_showADCInHex; }
    void                setShowADCInHex(const bool show) { m_showADCInHex = show; }

private:

    mutable QMutex     m_signalMutex;
    QList<MeasureSignalParam> m_signalParamList;

    static bool         m_showCustomID;
    static bool         m_showADCInHex;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;

private slots:

    void                updateSignalParam(const Hash& signalHash);

};

// ==============================================================================================

class SignalListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit            SignalListDialog(const bool hasButtons, QWidget *parent = 0);
                        ~SignalListDialog();

    Hash                selectedSignalHash() const { return m_selectedSignalHash; }

private:

    static int          m_columnWidth[SIGNAL_LIST_COLUMN_COUNT];

    QMenuBar*           m_pMenuBar = nullptr;
    QMenu*              m_pSignalMenu = nullptr;
    QMenu*              m_pEditMenu = nullptr;
    QMenu*              m_pViewMenu = nullptr;
    QMenu*              m_pViewTypeADMenu = nullptr;
    QMenu*              m_pViewTypeIOMenu = nullptr;
    QMenu*              m_pViewShowMenu = nullptr;
    QMenu*              m_pContextMenu = nullptr;

    QAction*            m_pPrintAction = nullptr;
    QAction*            m_pExportAction = nullptr;

    QAction*            m_pFindAction = nullptr;
    QAction*            m_pCopyAction = nullptr;
    QAction*            m_pSelectAllAction = nullptr;
    QAction*            m_pSignalPropertyAction = nullptr;

    QAction*            m_pTypeAnalogAction = nullptr;
    QAction*            m_pTypeDiscreteAction = nullptr;
    QAction*            m_pTypeInputAction = nullptr;
    QAction*            m_pTypeInternalAction = nullptr;
    QAction*            m_pTypeOutputAction = nullptr;
    QAction*            m_pShowCustomIDAction = nullptr;
    QAction*            m_pShowADCInHexAction = nullptr;

    QTableView*         m_pView = nullptr;
    SignalListTable     m_signalParamTable;

    QDialogButtonBox*   m_buttonBox = nullptr;

    QAction*            m_pColumnAction[SIGNAL_LIST_COLUMN_COUNT];
    QMenu*              m_headerContextMenu = nullptr;

    static E::SignalType       m_typeAD;
    static E::SignalInOutType  m_typeIO;
    static int          m_currenIndex;

    Hash                m_selectedSignalHash = 0;

    void                createInterface(const bool hasButtons);
    void                createHeaderContexMenu();
    void                createContextMenu();

    void                updateVisibleColunm();
    void                hideColumn(const int column, const bool hide);

protected:

    bool                eventFilter(QObject *object, QEvent *event);

signals:

private slots:

    // slots for updating
    //
    void                updateList();

    // slots of menu
    //
                        // Signal
                        //
    void                printSignal();
    void                exportSignal();

                        // Edit
                        //
    void                find();
    void                copy();
    void                selectAll() { m_pView->selectAll(); }
    void                signalProperty();


                        // View
                        //
    void                showTypeAnalog();
    void                showTypeDiscrete();

    void                showTypeInput();
    void                showTypeInternal();
    void                showTypeOutput();

    void                showCustomID();
    void                showADCInHex();

    void                onContextMenu(QPoint);

    // slots for list header, to hide or show columns
    //
    void                onHeaderContextMenu(QPoint);
    void                onColumnAction(QAction* action);

    // slots for list
    //
    void                onListDoubleClicked(const QModelIndex&);

    // slots of buttons
    //
    void                onOk();
};

// ==============================================================================================

#endif // SIGNALLISTDIALOG_H
