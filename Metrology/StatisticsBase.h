#ifndef STATISTICSIGNALBASE_H
#define STATISTICSIGNALBASE_H

#include <QObject>

#include "../lib/MetrologySignal.h"

#include "SignalConnectionBase.h"

// ==============================================================================================

class StatisticsItem
{
public:

	StatisticsItem();
	explicit StatisticsItem(Metrology::Signal* pSignal);
	StatisticsItem(Metrology::Signal* pSignal, std::shared_ptr<Metrology::ComparatorEx> pComparator);
	virtual ~StatisticsItem();

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

typedef QVector<StatisticsItem> StatisticList;

// ==============================================================================================

class StatisticsBase : public QObject
{
	Q_OBJECT

public:

	explicit StatisticsBase(QObject *parent = nullptr);
	virtual ~StatisticsBase();

private:

	int m_measureType = 0;			// measure type

	mutable QMutex m_signalMutex;
	QVector<StatisticList> m_statisticList;

	int m_measuredCount = 0;
	int m_invalidMeasureCount = 0;

public:

	void clear();
	int count() const;
	int count(int measureType) const;

	int measureType() const { return m_measureType; }
	void setMeasureType(int type) { m_measureType = type; }

	int measuredCount() const { return m_measuredCount; }
	void setMeasuredCount(int count) { m_measuredCount = count; }

	int invalidMeasureCount() const { return m_invalidMeasureCount; }
	void setInvalidMeasureCount(int count) { m_invalidMeasureCount = count; }

	void createSignalList();
	void createComparatorList();
	void updateConnections();

	StatisticsItem item(int index) const;
	StatisticsItem item(int measureType, int index) const;
	StatisticsItem* itemPtr(int measureType, int index);
	void setItem(int measureType, int index, const StatisticsItem& item);

public slots:

	void signalBaseLoaded() {}
	void measurementAppend() {}
	void measurementRemoved() {}
};

// ==============================================================================================


#endif // STATISTICSIGNALBASE_H
