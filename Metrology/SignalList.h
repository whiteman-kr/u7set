#ifndef SIGNALLISTDIALOG_H
#define SIGNALLISTDIALOG_H

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

const int               SIGNAL_LIST_COLUMN_COUNT        = sizeof(SignalListColumn)/sizeof(char*);

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

    int                 count() { return m_signaHashList.count(); }
    Hash                at(int index);
    void                set(const QList<Hash> list_add);
    void                clear();

    QString             text(int row, int column) const;

    bool                showCustomID() const { return m_showCustomID; }
    void                setShowCustomID(bool show) { m_showCustomID = show; }

    bool                showADCInHex() const { return m_showADCInHex; }
    void                setShowADCInHex(bool show) { m_showADCInHex = show; }

private:

    QList<Hash>         m_signaHashList;

    static bool         m_showCustomID;
    static bool         m_showADCInHex;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

class SignalListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit            SignalListDialog(QWidget *parent = 0);
                        ~SignalListDialog();

private:

    static int          m_columnWidth[SIGNAL_LIST_COLUMN_COUNT];

    QMenuBar*           m_pMenuBar = nullptr;
    QMenu*              m_pEditMenu = nullptr;
    QMenu*              m_pViewMenu = nullptr;
    QMenu*              m_pViewTypeADMenu = nullptr;
    QMenu*              m_pViewTypeIOMenu = nullptr;
    QMenu*              m_pViewShowMenu = nullptr;

    QAction*            m_pCopyAction = nullptr;
    QAction*            m_pFindAction = nullptr;
    QAction*            m_pSignalPropertyAction = nullptr;

    QAction*            m_pTypeAnalogAction = nullptr;
    QAction*            m_pTypeDiscreteAction = nullptr;
    QAction*            m_pTypeInputAction = nullptr;
    QAction*            m_pTypeOutputAction = nullptr;
    QAction*            m_pTypeInternalAction = nullptr;
    QAction*            m_pShowCustomIDAction = nullptr;
    QAction*            m_pShowADCInHexAction = nullptr;

    QTableView*         m_pView = nullptr;
    SignalListTable     m_table;

    QAction*            m_pColumnAction[SIGNAL_LIST_COLUMN_COUNT];
    QMenu*              m_headerContextMenu = nullptr;

    static E::SignalType       m_typeAD;
    static E::SignalInOutType  m_typeIO;

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
    void                showTypeAnalog();
    void                showTypeDiscrete();
    void                showTypeInput();
    void                showTypeOutput();
    void                showTypeInternal();
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

#endif // SIGNALLISTDIALOG_H
