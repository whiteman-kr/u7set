#ifndef MEASUREVIEW_H
#define MEASUREVIEW_H

#include <QTableView>
#include <QMenu>
#include <QList>

#include "Measure.h"
#include "MeasureViewHeader.h"
#include "MeasureBase.h"

// ==============================================================================================

class MeasureModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit            MeasureModel(QObject* parent = 0);
    explicit            MeasureModel(int type, QObject* parent = 0);
                        ~MeasureModel();

    int                 measureType() { return m_measureType; }
    void                setMeasureType(int type);

    MeasureViewHeader&  header() { return m_header; }

    int                 count() { return m_measureBase.count(); }

    bool                columnIsVisible(int column);

    int                 append(MeasureItem* pMeasure);

    QString             text(int row, int column) const;

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    MeasureViewHeader   m_header;
    MeasureBase         m_measureBase;

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

    void                updateColumn();

    MeasureModel&       table() { return m_Table; }

    int                 measureType() { return  m_measureType; }

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    MeasureModel        m_Table;

    QMenu*              m_headerContextMenu = nullptr;
    QList<QAction*>     m_actionList;

signals:

    void                measureCountChanged(int);

public slots:

    void                onHeaderContextMenu(QPoint);
    void                onHeaderContextAction(QAction* action);

    void                onColumnResized(int index, int, int width);

    void                appendMeasure(MeasureItem* pMeasure);
};

// ==============================================================================================

#endif // MEASUREVIEW_H
