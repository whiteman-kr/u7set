#include "StatisticBase.h"

#include "SignalBase.h"
#include "MeasureBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticItem::StatisticItem()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

StatisticItem::StatisticItem(Metrology::Signal* pSignal)
{
	clear();
	setSignal(pSignal);
}

// -------------------------------------------------------------------------------------------------------------------

StatisticItem::StatisticItem(Metrology::Signal* pSignal, std::shared_ptr<Metrology::ComparatorEx> pComparator)
{
	clear();
	setSignal(pSignal);
	setComparator(pComparator);
}

// -------------------------------------------------------------------------------------------------------------------

StatisticItem::~StatisticItem()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticItem::setSignal(Metrology::Signal* pSignal)
{
	m_pSignal = pSignal;

	setSignalConnectionType(pSignal);
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::signalConnectionTypeStr() const
{
	if (m_signalConnectionType < 0 || m_signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return QT_TRANSLATE_NOOP("StatisticBase.cpp", "Input is not set");
	}

	return qApp->translate("SignalConnectionBase.h", SignalConnectionType[m_signalConnectionType]);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticItem::setSignalConnectionType(Metrology::Signal* pSignal)
{
	m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	if (pSignal->param().isInput() == true)
	{
		m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNUSED;
		return;
	}

	int connectionIndex = theSignalBase.signalConnections().findIndex(MEASURE_IO_SIGNAL_TYPE_OUTPUT, pSignal);
	if (connectionIndex == -1)
	{
		return;
	}

	const SignalConnection& connection = theSignalBase.signalConnections().connection(connectionIndex);
	if (connection.isValid() == false)
	{
		return;
	}

	m_signalConnectionType = connection.type();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::measureCountStr() const
{
	if (m_measureCount == 0)
	{
		return QString();
	}

	return QString::number(m_measureCount);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticItem::clear()
{
	m_pSignal = nullptr;
	m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;
	m_pComparator = nullptr;

	m_measureCount = 0;
	m_state = State::Success;
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::stateStr() const
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

StatisticBase::StatisticBase(QObject *parent) :
	QObject(parent)
{
	QMutexLocker l(&m_signalMutex);

	m_statisticList.resize(MEASURE_TYPE_COUNT);
}

// -------------------------------------------------------------------------------------------------------------------

 StatisticBase::~StatisticBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::clear()
{
	QMutexLocker l(&m_signalMutex);

	int listCount = m_statisticList.count();
	for(int i = 0; i < listCount; i++)
	{
		m_statisticList[i].clear();
	}

	m_signalList.clear();
	m_signalCount = 0;

	m_measuredCount = 0;
	m_invalidMeasureCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticBase::count() const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return m_statisticList[m_measureType].count();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::createStatisticSignalList()
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

		StatisticItem si(pSignal);

		if (param.isInternal() == true /*|| param.isOutput() == true */)
		{
			if (si.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNDEFINED)
			{
				continue;
			}
		}

		m_statisticList[MEASURE_TYPE_LINEARITY].append(si);
	}

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::createStatisticComparatorList()
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
			StatisticItem si(pSignal, param.comparator(с));

			/*
			if (param.isInternal() == true)
			{
				if (si.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNDEFINED)
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

void StatisticBase::updateConnections()
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

StatisticItem StatisticBase::item(int index) const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return StatisticItem();
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_statisticList[m_measureType].count())
	{
		return StatisticItem();
	}

	return m_statisticList[m_measureType][index];
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::updateStatistics()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	QMutexLocker l(&m_signalMutex);

	QElapsedTimer responseTime;
	responseTime.start();

	m_measuredCount = 0;
	m_invalidMeasureCount = 0;

	int count = m_statisticList[m_measureType].count();
	for(int i = 0; i < count; i++)
	{
		StatisticItem& si = m_statisticList[m_measureType][i];

		theMeasureBase.updateStatistics(m_measureType, si);

		if (si.isMeasured() == true)
		{
			m_measuredCount++;
		}

		if (si.state() == StatisticItem::State::Failed)
		{
			m_invalidMeasureCount ++;
		}
	}

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::updateStatistics(Hash signalHash)
{
	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	QMutexLocker l(&m_signalMutex);

	int count = m_statisticList[m_measureType].count();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = m_statisticList[m_measureType][i].signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->param().hash() == signalHash)
		{
			theMeasureBase.updateStatistics(m_measureType, m_statisticList[m_measureType][i]);
		}
	}

	QElapsedTimer responseTime;
	responseTime.start();

	m_measuredCount = 0;
	m_invalidMeasureCount = 0;

	count = m_statisticList[m_measureType].count();
	for(int i = 0; i < count; i++)
	{
		StatisticItem& si = m_statisticList[m_measureType][i];

		if (si.isMeasured() == true)
		{
			m_measuredCount++;
		}

		if (si.state() == StatisticItem::State::Failed)
		{
			m_invalidMeasureCount ++;
		}
	}

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

