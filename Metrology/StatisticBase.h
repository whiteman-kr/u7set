#ifndef STATISTICSIGNALBASE_H
#define STATISTICSIGNALBASE_H

#include <QObject>
#include <QTableView>

#include "../lib/MetrologySignal.h"

// ==============================================================================================

class StatisticBase : public QObject
{
	Q_OBJECT

public:

	explicit StatisticBase(QObject *parent = nullptr);
	virtual ~StatisticBase();

private:

	mutable QMutex				m_signalMutex;
	QVector<Metrology::Signal*>	m_signalList;
	int							m_signalCount = 0;

	int							m_measuredCount = 0;
	int							m_invalidMeasureCount = 0;

public:

	int							measuredCount() const { return m_measuredCount; }
	void						setMeasuredCount(int count) { m_measuredCount = count; }

	int							invalidMeasureCount() const { return m_invalidMeasureCount; }
	void						setInvalidMeasureCount(int count) { m_invalidMeasureCount = count; }

	int							signalCount() const { return m_signalCount; }
	void						clear();

	void						createSignalList();

	Metrology::Signal*			signal(int index) const;

	void						updateState(QTableView* pView);

public slots:

	void						signalLoaded() {}
	void						measurementAppend() {}
	void						measurementRemoved() {}
};

// ==============================================================================================


#endif // STATISTICSIGNALBASE_H
