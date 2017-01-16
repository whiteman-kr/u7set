#ifndef FINDMEASUREPANEL_H
#define FINDMEASUREPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>

#include "Measure.h"

// ==============================================================================================

class FindItem
{
public:

    explicit            FindItem();
    explicit            FindItem(int row, int column, QString columnTitle, int beginPos, int endPos, QString text);

private:

    int                 m_row = -1;
    int                 m_column = -1;
    QString             m_columnTitle;

    int                 m_beginPos = -1;
    int                 m_endPos = -1;

    QString             m_text;

public:

    int                 row() const { return m_row; }
    void                setRow(int row) { m_row = row; }

    int                 column() const { return m_column; }
    void                setColumn(int column) { m_column = column; }

    QString             columnTitle() const { return m_columnTitle; }
    void                setColumnTitle(QString title) { m_columnTitle = title; }

    void                setCoordinates(int row, int column) { m_row = row; m_column = column; }

    int                 beginPos() const { return m_beginPos; }
    void                setBeginPos(int pos) { m_beginPos = pos; }

    int                 endPos() const { return m_endPos; }
    void                setEndPos(int pos) { m_endPos = pos; }

    void                setPos(int beginPos, int endPos) { m_beginPos = beginPos; m_endPos = endPos; }

    QString             text() const { return m_text; }
    void                setText(QString text) { m_text = text; }

    FindItem&           operator=(const FindItem& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(FindItem)   // for type QVariant

// ==============================================================================================

const char* const       FindMeasureColumn[] =
{
                        QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Row"),
                        QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Text"),
                        QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Column"),
};

const int               FIND_MEASURE_COLUMN_COUNT       = sizeof(FindMeasureColumn)/sizeof(char*);

const int               FIND_MEASURE_COLUMN_ROW         = 0,
                        FIND_MEASURE_COLUMN_TEXT        = 1,
                        FIND_MEASURE_COLUMN_COLUMN      = 2;

// ==============================================================================================

class FindMeasureTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            FindMeasureTable(QObject* parent = 0);
                        ~FindMeasureTable();

    int                 count() { return m_findItemList.count(); }
    FindItem            at(int index);
    void                set(const QList<FindItem> list_add);
    void                clear();

    QString             text(int row, int column) const;

private:

    QList<FindItem>     m_findItemList;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

#define                 FIND_MEASURE_OPTIONS_KEY        "Options/FindMeasure/"

// ==============================================================================================

class FindMeasurePanel : public QDockWidget
{
    Q_OBJECT

public:

    explicit            FindMeasurePanel(QWidget* parent = 0);

    FindMeasureTable&   table() { return m_table; }

private:

    QMainWindow*        m_pMainWindow;

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    static int          m_columnWidth[FIND_MEASURE_COLUMN_COUNT];

    QString             m_findText;

    QMainWindow*        m_pFindWindow = nullptr;
    QLineEdit*          m_findTextEdit  = nullptr;
    QTableView*         m_pView = nullptr;
    QLabel*             m_statusLabel = nullptr;
    FindMeasureTable    m_table;

    QMenu*              m_pContextMenu = nullptr;
    QAction*            m_pCopyAction = nullptr;
    QAction*            m_pSelectAllAction = nullptr;

    void                createInterface();
    void                createContextMenu();

    void                loadSettings();
    void                saveSettings();

protected:

    bool                event(QEvent* e);
    bool                eventFilter(QObject* object, QEvent* e);

private slots:

    void                find();

    void                selectMeasureCell(QModelIndex);

    void                onContextMenu(QPoint);
    void                onColumnResized(int index, int, int width);

    void                copy();
    void                selectAll();
};

// ==============================================================================================

#endif // FINDMEASUREPANEL_H
