#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QMap>

#include "Measure.h"

// ==============================================================================================

class MeasureBase : public QObject
{
    Q_OBJECT

public:

    explicit                MeasureBase(QObject *parent = 0);

    void                    setMeasureType(int type)    { m_measureType = type; }

    int                     count() const;

    int                     append(MeasureItem* pMeasure);
    MeasureItem*            at(int index) const;
    bool                    removeAt(int index);

    void                    clear();

private:

    mutable QMutex          m_mutex;

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;

    QList<MeasureItem*>     m_measureList;

    int                     formatLinearityMeasure(MeasureItem* pMeasure);
    int                     formatComparatorMeasure(MeasureItem* pMeasure);
    int                     formatComplexComparatorMeasure(MeasureItem* pMeasure);

signals:

public slots:

};

// ==============================================================================================

#endif // MEASUREBASE_H
