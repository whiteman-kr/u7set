#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "Measure.h"
#include "Statistic.h"

// ==============================================================================================

class MeasurementBase : public QObject
{
    Q_OBJECT

public:
    explicit                MeasurementBase(QObject *parent = 0);
                            ~MeasurementBase();

    int                     measurementCount() const;
    int                     measurementCount(const int measureType) const;

    void                    clear(const bool removeData = true);

    int                     load(const int measureType);

    int                     append(Measurement* pMeasurement);
    bool                    remove(const int index, const bool removeData = true);
    Measurement*            measurement(const int index) const;

    StatisticItem           statistic(const Hash& signalHash);

private:

    int                     m_measureType = MEASURE_TYPE_UNKNOWN;

    mutable QMutex          m_measurmentListMutex;
    QVector<Measurement*>   m_measurementList;

    int                     m_measurementCount[MEASURE_TYPE_COUNT];
};

// ==============================================================================================

extern MeasurementBase theMeasurementBase;

// ==============================================================================================


#endif // MEASUREBASE_H
