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

	setConnectionType(pSignal);
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsItem::connectionTypeStr() const
{
	if (ERR_METROLOGY_CONNECTION_TYPE(m_connectionType) == true)
	{
		return QT_TRANSLATE_NOOP("StatisticBase.cpp", "Input is not set");
	}

	return qApp->translate("MetrologyConnection", Metrology::ConnectionTypeCaption(m_connectionType).toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsItem::setConnectionType(Metrology::Signal* pSignal)
{
	m_connectionType = Metrology::ConnectionType::NoConnectionType;

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	if (pSignal->param().isInput() == true)
	{
		m_connectionType = Metrology::ConnectionType::Unused;
		return;
	}

	int connectionIndex = theSignalBase.connections().findConnectionIndex(Metrology::ConnectionIoType::Destination, pSignal);
	if (connectionIndex == -1)
	{
		return;
	}

	const Metrology::Connection& connection = theSignalBase.connections().connection(connectionIndex);
	if (connection.isValid() == false)
	{
		return;
	}

	m_connectionType = connection.type();
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
	m_connectionType = Metrology::ConnectionType::NoConnectionType;
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
		default:				assert(0);
	}

	return state;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsBase::StatisticsBase(QObject* parent) :
	QObject(parent)
{
	QMutexLocker l(&m_signalMutex);

	m_statisticList.resize(MeasureTypeCount);
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
	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return m_statisticList[m_measureType].count();
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsBase::count(int measureType) const
{
	if (ERR_MEASURE_TYPE(measureType) == true)
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

	if (m_statisticList.count() <= MeasureType::Linearity)
	{
		return;
	}

	m_statisticList[MeasureType::Linearity].clear();

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
			if (ERR_METROLOGY_CONNECTION_TYPE(si.connectionType()) == true)
			{
				continue;
			}
		}

		m_statisticList[MeasureType::Linearity].append(si);
	}

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::createComparatorList()
{
	QMutexLocker l(&m_signalMutex);

	if (m_statisticList.count() <= MeasureType::Comparators)
	{
		return;
	}

	m_statisticList[MeasureType::Comparators].clear();

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
				if (ERR_METROLOGY_CONNECTION_TYPE(si.connectionType()) == true)
				{
					continue;
				}
			}
			*/

			m_statisticList[MeasureType::Comparators].append(si);
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
			m_statisticList[n][i].setConnectionType(m_statisticList[n][i].signal());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem StatisticsBase::item(int index) const
{
	if (ERR_MEASURE_TYPE(m_measureType) == true)
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
	if (ERR_MEASURE_TYPE(measureType) == true)
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
	if (ERR_MEASURE_TYPE(measureType) == true)
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
	if (ERR_MEASURE_TYPE(measureType) == true)
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

