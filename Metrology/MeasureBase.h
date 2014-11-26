#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include <QMutex>
#include <QVector>

#include "Measure.h"

// ==============================================================================================

class MeasureBase : public QObject
{
    Q_OBJECT

public:

    explicit                MeasureBase(QObject *parent = 0);

    int                     load(int measureType);

    int                     count() const;

    int                     append(MeasureItem* pMeasure);
    MeasureItem*            at(int index) const;
    bool                    remove(int index);

    void                    clear();

private:

    mutable QMutex          m_mutex;

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;

    QVector<MeasureItem*>   m_measureList;

signals:

public slots:

};

// ==============================================================================================

#endif // MEASUREBASE_H
