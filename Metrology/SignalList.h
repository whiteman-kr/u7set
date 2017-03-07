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

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const           SignalListColumn[] =
{
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Case"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "ID"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "EquipmentID"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Caption"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Subblock"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Block"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Entry"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "ADC range"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Input Ph.range"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Input El.range"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Input El.sensor"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Output Ph.range"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Output El.range"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Output El.sensor"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Tuning"),
                            QT_TRANSLATE_NOOP("SignalListDialog.h", "Default value"),
};

const int                   SIGNAL_LIST_COLUMN_COUNT            = sizeof(SignalListColumn)/sizeof(SignalListColumn[0]);

const int                   SIGNAL_LIST_COLUMN_RACK             = 0,
                            SIGNAL_LIST_COLUMN_ID               = 1,
                            SIGNAL_LIST_COLUMN_EQUIPMENT_ID     = 2,
                            SIGNAL_LIST_COLUMN_CAPTION          = 3,
                            SIGNAL_LIST_COLUMN_SUBBLOCK         = 4,
                            SIGNAL_LIST_COLUMN_BLOCK            = 5,
                            SIGNAL_LIST_COLUMN_ENTRY            = 6,
                            SIGNAL_LIST_COLUMN_ADC              = 7,
                            SIGNAL_LIST_COLUMN_IN_PH_RANGE      = 8,
                            SIGNAL_LIST_COLUMN_IN_EL_RANGE      = 9,
                            SIGNAL_LIST_COLUMN_IN_EL_SENSOR     = 10,
                            SIGNAL_LIST_COLUMN_OUT_PH_RANGE     = 11,
                            SIGNAL_LIST_COLUMN_OUT_EL_RANGE     = 12,
                            SIGNAL_LIST_COLUMN_OUT_EL_SENSOR    = 13,
                            SIGNAL_LIST_COLUMN_TUN_SIGNAL       = 14,
                            SIGNAL_LIST_COLUMN_TUN_DEFAULT_VAL  = 15;

// ==============================================================================================

class SignalListTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit                SignalListTable(QObject* parent = 0);
                            ~SignalListTable();

    int                     signalCount() const;
	Metrology::SignalParam	signalParam(int index) const;
	void                    set(const QList<Metrology::SignalParam> list_add);
    void                    clear();

	QString                 text(int row, int column, const Metrology::SignalParam& param) const;

    bool                    showCustomID() const { return m_showCustomID; }
    void                    setShowCustomID(bool show) { m_showCustomID = show; }

    bool                    showADCInHex() const { return m_showADCInHex; }
    void                    setShowADCInHex(bool show) { m_showADCInHex = show; }

private:

    mutable QMutex          m_signalMutex;
	QList<Metrology::SignalParam> m_signalParamList;

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

class SignalListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                SignalListDialog(bool hasButtons, QWidget *parent = 0);
                            ~SignalListDialog();

    Hash                    selectedSignalHash() const { return m_selectedSignalHash; }

private:

    static int              m_columnWidth[SIGNAL_LIST_COLUMN_COUNT];

    QMenuBar*               m_pMenuBar = nullptr;
    QMenu*                  m_pSignalMenu = nullptr;
    QMenu*                  m_pEditMenu = nullptr;
    QMenu*                  m_pViewMenu = nullptr;
    QMenu*                  m_pViewTypeADMenu = nullptr;
    QMenu*                  m_pViewTypeIOMenu = nullptr;
    QMenu*                  m_pViewShowMenu = nullptr;
    QMenu*                  m_pContextMenu = nullptr;

    QAction*                m_pPrintAction = nullptr;
    QAction*                m_pExportAction = nullptr;

    QAction*                m_pFindAction = nullptr;
    QAction*                m_pCopyAction = nullptr;
    QAction*                m_pSelectAllAction = nullptr;
    QAction*                m_pSignalPropertyAction = nullptr;

    QAction*                m_pTypeAnalogAction = nullptr;
    QAction*                m_pTypeDiscreteAction = nullptr;
    QAction*                m_pTypeInputAction = nullptr;
    QAction*                m_pTypeInternalAction = nullptr;
    QAction*                m_pTypeOutputAction = nullptr;
    QAction*                m_pShowCustomIDAction = nullptr;
    QAction*                m_pShowADCInHexAction = nullptr;

    QTableView*             m_pView = nullptr;
    SignalListTable         m_signalParamTable;

    QDialogButtonBox*       m_buttonBox = nullptr;

    QAction*                m_pColumnAction[SIGNAL_LIST_COLUMN_COUNT];
    QMenu*                  m_headerContextMenu = nullptr;

    static E::SignalType       m_typeAD;
    static E::SignalInOutType  m_typeIO;
    static int              m_currenIndex;

    Hash                    m_selectedSignalHash = 0;

    void                    createInterface(bool hasButtons);
    void                    createHeaderContexMenu();
    void                    createContextMenu();

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

                            // Edit
                            //
    void                    find();
    void                    copy();
    void                    selectAll() { m_pView->selectAll(); }
    void                    signalProperty();


                            // View
                            //
    void                    showTypeAnalog();
    void                    showTypeDiscrete();

    void                    showTypeInput();
    void                    showTypeInternal();
    void                    showTypeOutput();

    void                    showCustomID();
    void                    showADCInHex();

    void                    onContextMenu(QPoint);

    // slots for list header, to hide or show columns
    //
    void                    onHeaderContextMenu(QPoint);
    void                    onColumnAction(QAction* action);

    // slots for list
    //
    void                    onListDoubleClicked(const QModelIndex&);

    // slots of buttons
    //
    void                    onOk();
};

// ==============================================================================================

#endif // SIGNALLISTDIALOG_H
