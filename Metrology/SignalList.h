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

Q_DECLARE_METATYPE(MeasureSignal)   // for type QVariant

// ==============================================================================================

const char* const       SignalListColumn[] =
{
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "AppSignalID"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "CustomAppSignalID"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Caption"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "EquipmentID"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Case"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Chassis"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Module"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Input"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "ADC"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Input Ph.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Input El.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Output Ph.range"),
                        QT_TRANSLATE_NOOP("SignalListDialog.h", "Output El.range"),
};

const int               SIGNAL_LIST_COLUMN_COUNT        = sizeof(SignalListColumn)/sizeof(char*);

const int               SIGNAL_LIST_COLUMN_ID           = 0,
                        SIGNAL_LIST_COLUMN_CUSTOM_ID    = 1,
                        SIGNAL_LIST_COLUMN_CAPTION      = 2,
                        SIGNAL_LIST_COLUMN_EQUIPMENT_ID = 3,
                        SIGNAL_LIST_COLUMN_CASE         = 4,
                        SIGNAL_LIST_COLUMN_CHASSIS      = 5,
                        SIGNAL_LIST_COLUMN_MODULE       = 6,
                        SIGNAL_LIST_COLUMN_INPUT        = 7,
                        SIGNAL_LIST_COLUMN_ADC          = 8,
                        SIGNAL_LIST_COLUMN_IN_PH_RANGE  = 9,
                        SIGNAL_LIST_COLUMN_IN_EL_RANGE  = 10,
                        SIGNAL_LIST_COLUMN_OUT_PH_RANGE = 11,
                        SIGNAL_LIST_COLUMN_OUT_EL_RANGE = 12;

// ==============================================================================================

class SignalListTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit SignalListTable(QObject* parent = 0);
             ~SignalListTable();

    int count() { return m_signalList.count(); }
    MeasureSignal at(int index);
    void set(const QList<MeasureSignal> list_add);
    void clear();

    QString text(int row, int column) const;

private:

    QList<MeasureSignal> m_signalList;

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

class SignalListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignalListDialog(QWidget *parent = 0);
             ~SignalListDialog();

private:

    QMenuBar* m_pMenuBar = nullptr;
    QMenu* m_pViewMenu = nullptr;
    QMenu* m_pViewTypeADMenu = nullptr;
    QMenu* m_pViewTypeIOMenu = nullptr;

    QAction* m_pTypeAnalogAction = nullptr;
    QAction* m_pTypeDiscreteAction = nullptr;
    QAction* m_pTypeInputAction = nullptr;
    QAction* m_pTypeOutputAction = nullptr;
    QAction* m_pTypeInternalAction = nullptr;

    QTableView* m_pView = nullptr;
    SignalListTable m_table;

    QAction* m_pAction[SIGNAL_LIST_COLUMN_COUNT];
    QMenu* m_headerContextMenu = nullptr;

    E::SignalType m_typeAD = E::SignalType::Analog;
    E::SignalInOutType m_typeIO = E::SignalInOutType::Input;

    void createInterface();
    void createHeaderContexMenu();

    void updateList();
    void updateVisibleColunm();

    void hideColumn(int column, bool hide);

signals:

private slots:

    // slots of menu
    //
    void onTypeAnalog();
    void onTypeDiscrete();
    void onTypeInput();
    void onTypeOutput();
    void onTypeInternal();

    // slots for list header, to hide or show columns
    //
    void                onHeaderContextMenu(QPoint);
    void                onAction(QAction* action);
};

// ==============================================================================================

#endif // SIGNALLISTDIALOG_H
