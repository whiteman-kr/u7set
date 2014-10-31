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

    explicit            MeasureModel(int type, QObject* parent = 0);
                        ~MeasureModel();

    MeasureViewHeader   m_header;

    int                 count() { return m_measureBase.count(); }

    int                 append(MeasureItem* pMeasure);

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    MeasureBase         m_measureBase;

    int                 columnCount(const QModelIndex &parent) const;
    int                 rowCount(const QModelIndex &parent=QModelIndex()) const;

    QVariant            headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant            data(const QModelIndex &index, int role) const;

    QString             measureLinearity(const QModelIndex& index) const;
    QString             measureComparator(const QModelIndex& index) const;
    QString             measureComplexComparator(const QModelIndex& index) const;
};

// ==============================================================================================

class MeasureView : public QTableView
{
    Q_OBJECT

public:
    explicit            MeasureView(int type, QWidget *parent = 0);
                        ~MeasureView();

    void                updateColumn();

    int                 measureCount() { return m_pModel->count(); }

private:

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    MeasureModel*       m_pModel = nullptr;

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
