#include "StatisticsBase.h"

#include "MeasureBase.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsItem::StatisticsItem()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem::StatisticsItem(Metrology::Signal* pSignal)
{
	clear();
	setSignal(pSignal);
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem::StatisticsItem(Metrology::Signal* pSignal, std::shared_ptr<Metrology::ComparatorEx> pComparator)
{
	clear();
	setSignal(pSignal);
	setComparator(pComparator);
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem::~StatisticsItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsItem::setSignal(Metrology::Signal* pSignal)
{
	m_pSignal = pSignal;

	setSignalConnectionType(pSignal);
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsItem::signalConnectionTypeStr() const
{
	if (m_signalConnectionType < 0 || m_signalConnectionType >= Metrology::ConnectionTypeCount)
	{
		return QT_TRANSLATE_NOOP("StatisticBase.cpp", "Input is not set");
	}

	return Metrology::ConnectionTypeCaption(m_signalConnectionType);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsItem::setSignalConnectionType(Metrology::Signal* pSignal)
{
	m_signalConnectionType = Metrology::ConnectionType::Unknown;
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	if (pSignal->param().isInput() == true)
	{
		m_signalConnectionType = Metrology::ConnectionType::Unsed;
		return;
	}

	int connectionIndex = theSignalBase.signalConnections().findConnectionIndex(Metrology::ConnectionIoType::Destination, pSignal);
	if (connectionIndex == -1)
	{
		return;
	}

	const Metrology::Connection& connection = theSignalBase.signalConnections().connection(connectionIndex);
	if (connection.isValid() == false)
	{
		return;
	}

	m_signalConnectionType = connection.type();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsItem::measureCountStr() const
{
	if (m_measureCount == 0)
	{
		return QString();
	}

	return QString::number(m_measureCount);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsItem::clear()
{
	m_pSignal = nullptr;
	m_signalConnectionType = Metrology::ConnectionType::Unknown;
	m_pComparator = nullptr;

	m_measureCount = 0;
	m_state = State::Success;
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsItem::stateStr() const
{
	if (m_measureCount == 0)
	{
		return QT_TRANSLATE_NOOP("StatisticBase.cpp", "Not measured");
	}

	QString state;

	switch (m_state)
	{
		case State::Failed:		state = QT_TRANSLATE_NOOP("StatisticBase.cpp", "Failed");	break;
		case State::Success:	state = QT_TRANSLATE_NOOP("StatisticBase.cpp", "Ok");		break;
		default:				assert(0);													break;
	}

	return state;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsBase::StatisticsBase(QObject *parent) :
	QObject(parent)
{
	QMutexLocker l(&m_signalMutex);

	m_statisticList.resize(MEASURE_TYPE_COUNT);
}

// -------------------------------------------------------------------------------------------------------------------

 StatisticsBase::~StatisticsBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::clear()
{
	QMutexLocker l(&m_signalMutex);

	int listCount = m_statisticList.count();
	for(int i = 0; i < listCount; i++)
	{
		m_statisticList[i].clear();
	}

	m_measuredCount = 0;
	m_invalidMeasureCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsBase::count() const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return m_statisticList[m_measureType].count();
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsBase::count(int measureType) const
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return m_statisticList[measureType].count();
}


// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::createSignalList()
{
	QMutexLocker l(&m_signalMutex);

	if (m_statisticList.count() <= MEASURE_TYPE_LINEARITY)
	{
		return;
	}

	m_statisticList[MEASURE_TYPE_LINEARITY].clear();

	QElapsedTimer responseTime;
	responseTime.start();

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.location().shownOnSchemas() == false)
		{
			continue;
		}

		if (param.isAnalog() == false)
		{
			continue;
		}

		if (param.isInput() == true || param.isOutput() == true)
		{
			if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
			{
				continue;
			}

			if (pSignal->param().electricRangeIsValid() == false)
			{
				continue;
			}
		}

		StatisticsItem si(pSignal);

		if (param.isInternal() == true /*|| param.isOutput() == true */)
		{
			if (si.signalConnectionType() == Metrology::ConnectionType::Unknown)
			{
				continue;
			}
		}

		m_statisticList[MEASURE_TYPE_LINEARITY].append(si);
	}

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::createComparatorList()
{
	QMutexLocker l(&m_signalMutex);

	if (m_statisticList.count() <= MEASURE_TYPE_COMPARATOR)
	{
		return;
	}

	m_statisticList[MEASURE_TYPE_COMPARATOR].clear();

	QElapsedTimer responseTime;
	responseTime.start();

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.location().shownOnSchemas() == false)
		{
			continue;
		}

		if (param.isAnalog() == false)
		{
			continue;
		}

		if (param.isOutput() == true)
		{
			continue;
		}

		if (param.isInput() == true)
		{
			if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
			{
				continue;
			}

			if (pSignal->param().electricRangeIsValid() == false)
			{
				continue;
			}
		}

		int comparatorCount = param.comparatorCount();
		if (comparatorCount == 0)
		{
			continue;
		}

		for(int с = 0; с < comparatorCount; с++)
		{
			StatisticsItem si(pSignal, param.comparator(с));

			/*
			if (param.isInternal() == true)
			{
				if (si.signalConnectionType() == CONNECTION_TYPE_UNDEFINED)
				{
					continue;
				}
			}
			*/

			m_statisticList[MEASURE_TYPE_COMPARATOR].append(si);
		}
	}

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::updateConnections()
{
	QMutexLocker l(&m_signalMutex);

	int listCount = m_statisticList.count();
	for(int n = 0; n < listCount; n++)
	{
		int itemCount = m_statisticList[n].count();
		for(int i = 0; i < itemCount; i++)
		{
			m_statisticList[n][i].setSignalConnectionType(m_statisticList[n][i].signal());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem StatisticsBase::item(int index) const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return StatisticsItem();
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_statisticList[m_measureType].count())
	{
		return StatisticsItem();
	}

	return m_statisticList[m_measureType][index];
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem StatisticsBase::item(int measureType, int index) const
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return StatisticsItem();
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_statisticList[measureType].count())
	{
		return StatisticsItem();
	}

	return m_statisticList[measureType][index];
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem* StatisticsBase::itemPtr(int measureType, int index)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return nullptr;
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_statisticList[measureType].count())
	{
		return nullptr;
	}

	return &m_statisticList[measureType][index];
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::setItem(int measureType, int index, const StatisticsItem& item)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_statisticList[measureType].count())
	{
		return;
	}

	m_statisticList[measureType][index] = item;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

