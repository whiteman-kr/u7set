#ifndef MEASUREVIEW_H
#define MEASUREVIEW_H

#include <QTableView>
#include <QMenu>
#include <QList>

#include "Measure.h"
#include "MeasureViewHeader.h"
#include "MeasureBase.h"

// ==============================================================================================

class MeasureTable : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            MeasureTable(QObject* parent = 0);
                        ~MeasureTable();

    int                 measureType() { return m_measureType; }
    void                setMeasureType(int type);

    MeasureViewHeader&  header() { return m_header; }

    int                 count() { return m_measureBase.measurementCount(); }

    bool                columnIsVisible(int column);

    bool                append(Measurement* pMeasurement);
    bool                remove(const QList<int> removeIndexList);

    QColor              backgroundColor(int row, int column) const;
    QString             text(int row, int column) const;

    MeasurementBase     m_measureBase;

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    MeasureViewHeader   m_header;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;

    QString             textLinearity(int row, int column) const;
    QString             textComparator(int row, int column) const;
    QString             textComplexComparator(int row, int column) const;

};

// ==============================================================================================

class MeasureView : public QTableView
{
    Q_OBJECT

public:
    explicit            MeasureView(int type, QWidget *parent = 0);
                        ~MeasureView();

    int                 measureType()   { return m_measureType; }
    MeasureTable&       table()         { return m_table; }

    void                updateColumn();

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;
    MeasureTable        m_table;

    QMenu*              m_headerContextMenu = nullptr;
    QList<QAction*>     m_actionList;

    void                createContextMenu();

signals:

    void                measureCountChanged(int);

public slots:

    void                onHeaderContextMenu(QPoint);
    void                onHeaderContextAction(QAction* action);

    void                onColumnResized(int index, int, int width);

    void                appendMeasure(Measurement* pMeasurement);
    void                removeMeasure();

    void                copy();
};

// ==============================================================================================

#endif // MEASUREVIEW_H
