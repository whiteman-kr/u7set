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

                        FindItem();
                        FindItem(const int row, const int column, const QString& text, const int beginPos, const int endPos);
                        ~FindItem();

private:

    int                 m_row = -1;
    int                 m_column = -1;

    QString             m_text;

    int                 m_beginPos = -1;
    int                 m_endPos = -1;

public:

    int                 row() const { return m_row; }
    void                setRow(const int row) { m_row = row; }

    int                 column() const { return m_column; }
    void                setColumn(const int column) { m_column = column; }

    void                setCoordinates(const int row, const int column) { m_row = row; m_column = column; }

    QString             text() const { return m_text; }
    void                setText(const QString& text) { m_text = text; }

    int                 beginPos() const { return m_beginPos; }
    void                setBeginPos(const int pos) { m_beginPos = pos; }

    int                 endPos() const { return m_endPos; }
    void                setEndPos(const int pos) { m_endPos = pos; }

    void                setPos(int beginPos, int endPos) { m_beginPos = beginPos; m_endPos = endPos; }

    FindItem&           operator=(const FindItem& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(FindItem)   // for type QVariant

// ==============================================================================================

const char* const       FindMeasureColumn[] =
{
                        QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Row"),
                        QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Text"),
};

const int               FIND_MEASURE_COLUMN_COUNT       = sizeof(FindMeasureColumn)/sizeof(FindMeasureColumn[0]);

const int               FIND_MEASURE_COLUMN_ROW         = 0,
                        FIND_MEASURE_COLUMN_TEXT        = 1;

const int               FIND_MEASURE_COLUMN_ROW_WIDTH   = 50;

// ==============================================================================================

class FindMeasureTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            FindMeasureTable(QObject* parent = 0);
                        ~FindMeasureTable();

    int                 count() const { return m_findItemList.count(); }
    FindItem            at(const int index) const;
    void                set(const QList<FindItem> list_add);
    void                clear();

    QString             text(const int row, const int column) const;

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
                        ~FindMeasurePanel();

    void                clear() { m_table.clear(); m_statusLabel->setText(QString()); }

private:

    QMainWindow*        m_pMainWindow;

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

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

    void                selectItemInMeasureView();

    void                onContextMenu(QPoint);

    void                copy();
    void                selectAll() { m_pView->selectAll(); }
};

// ==============================================================================================

#endif // FINDMEASUREPANEL_H
