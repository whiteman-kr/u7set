#ifndef MEASUREBASE_H
#define MEASUREBASE_H

#include "Measure.h"
#include "Statistic.h"

// ==============================================================================================

class MeasurementBase : public QObject
{
	Q_OBJECT

public:

	explicit MeasurementBase(QObject *parent = 0);
	~MeasurementBase();

private:

	int						m_measureType = MEASURE_TYPE_UNKNOWN;

	mutable QMutex			m_measurmentListMutex;
	QVector<Measurement*>	m_measurementList;

	int						m_measurementCount[MEASURE_TYPE_COUNT];

public:

	int						measurementCount() const;
	int						measurementCount(int measureType) const;

	void					clear(bool removeData = true);

	int						load(int measureType);

	int						append(Measurement* pMeasurement);
	bool					remove(int index, bool removeData = true);
	Measurement*			measurement(int index) const;

	StatisticItem			statistic(const Hash& signalHash);
};

// ==============================================================================================

extern MeasurementBase theMeasurementBase;

// ==============================================================================================


#endif // MEASUREBASE_H
