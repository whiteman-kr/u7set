#ifndef STATISTICSIGNALBASE_H
#define STATISTICSIGNALBASE_H

#include <QObject>
#include <QTableView>

#include "SignalConnectionBase.h"

#include "../lib/MetrologySignal.h"

// ==============================================================================================

class StatisticItem
{
public:

	StatisticItem();
	explicit StatisticItem(Metrology::Signal* pSignal);
	StatisticItem(Metrology::Signal* pSignal, std::shared_ptr<Metrology::ComparatorEx> pComparator);
	virtual ~StatisticItem();

	enum State
	{
		Failed,
		Success,
	};

private:

	Metrology::Signal* m_pSignal = nullptr;
	int m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;
	std::shared_ptr<Metrology::ComparatorEx> m_pComparator = nullptr;

	int m_measureCount = 0;
	State m_state = State::Success;

public:

	void clear();

	Metrology::Signal* signal() const { return m_pSignal; }
	void setSignal(Metrology::Signal* pSignal);

	int signalConnectionType() const { return m_signalConnectionType; }
	QString signalConnectionTypeStr() const;
	void setSignalConnectionType(int type) { m_signalConnectionType = type; }
	void setSignalConnectionType(Metrology::Signal* pSignal);

	std::shared_ptr<Metrology::ComparatorEx> comparator() const { return m_pComparator; }
	void setComparator(std::shared_ptr<Metrology::ComparatorEx> pComparator) { m_pComparator = pComparator; }

	int measureCount() const { return m_measureCount; }
	void setMeasureCount(int count) { m_measureCount = count; }
	QString measureCountStr() const;

	State state() const { return m_state; }
	void setState(State state) { m_state = state; }
	QString stateStr() const;

	bool isMeasured() const { return m_measureCount != 0; }
};

// ==============================================================================================

typedef QVector<StatisticItem> StatisticList;

// ==============================================================================================

class StatisticBase : public QObject
{
	Q_OBJECT

public:

	explicit StatisticBase(QObject *parent = nullptr);
	virtual ~StatisticBase();

private:

	int							m_measureType = 0;			// measure type

	mutable QMutex				m_signalMutex;
	QVector<Metrology::Signal*>	m_signalList;
	int							m_signalCount = 0;

	QVector<Metrology::Signal*>	m_comparatorList;
	int							m_comparatorCount = 0;

	QVector<StatisticList>		m_statisticList;
	int							m_statisticCount = 0;

	int							m_measuredCount = 0;
	int							m_invalidMeasureCount = 0;

public:

	void						clear();
	int							count() const;

	int							measureType() const { return m_measureType; }
	void						setMeasureType(int type) { m_measureType = type; }

	int							measuredCount() const { return m_measuredCount; }
	void						setMeasuredCount(int count) { m_measuredCount = count; }

	int							invalidMeasureCount() const { return m_invalidMeasureCount; }
	void						setInvalidMeasureCount(int count) { m_invalidMeasureCount = count; }

	void						createStatisticSignalList();
	void						createStatisticComparatorList();
	void						updateConnections();

	StatisticItem				item(int index) const;

	void						updateStatistics(QTableView* pView);
	void						updateStatistics(QTableView* pView, Hash signalHash);

public slots:

	void						signalLoaded() {}
	void						measurementAppend() {}
	void						measurementRemoved() {}
};

// ==============================================================================================


#endif // STATISTICSIGNALBASE_H
