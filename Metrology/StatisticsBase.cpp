#include "StatisticsBase.h"

#include "MeasureBase.h"
#include "SignalBase.h"

#include <QApplication>

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
		return QT_TRANSLATE_NOOP("StatisticsBase", "Input is not set");
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
		return QT_TRANSLATE_NOOP("StatisticsBase", "Not measured");
	}

	QString state;

	switch (m_state)
	{
		case State::Failed:		state = QT_TRANSLATE_NOOP("StatisticsBase", "Failed");	break;
		case State::Success:	state = QT_TRANSLATE_NOOP("StatisticsBase", "Ok");		break;

		default:
			assert(0);
			state = QT_TRANSLATE_NOOP("StatisticsBase", "Unknown");
	}

	return state;
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsItem::positionID() const
{
	if (m_pSignal == nullptr)
	{
		return QString();
	}

	const Metrology::SignalParam& param = m_pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	QString posID =	QString::number(TO_INT(param.inOutType())).rightJustified(2, '0') +
					QString::number(m_connectionType).rightJustified(4, '0') +
					param.location().rack().caption() +
					QString::number(param.location().chassis()).rightJustified(4, '0') +
					QString::number(param.location().module()).rightJustified(4, '0') +
					QString::number(param.location().place()).rightJustified(4, '0');

	return posID;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsBase::StatisticsBase(QObject* parent) :
	QObject(parent)
{
	QMutexLocker l(&m_signalMutex);

	m_statisticList.resize(Measure::TYPE_COUNT);
}

// -------------------------------------------------------------------------------------------------------------------

 StatisticsBase::~StatisticsBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::clear()
{
	QMutexLocker l(&m_signalMutex);

	quint64 listCount = m_statisticList.size();
	for(quint64 i = 0; i < listCount; i++)
	{
		m_statisticList[i].clear();
	}

	m_measuredCount = 0;
	m_invalidMeasureCount = 0;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsBase::count() const
{
	if (Measure::ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return TO_INT(m_statisticList[static_cast<quint64>(m_measureType)].size());
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsBase::count(int measureType) const
{
	if (Measure::ERR_MEASURE_TYPE(measureType) == true)
	{
		return 0;
	}

	QMutexLocker l(&m_signalMutex);

	return TO_INT(m_statisticList[static_cast<quint64>(measureType)].size());
}


// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::createSignalList(bool shownOnSchemas)
{
	QMutexLocker l(&m_signalMutex);

	if (m_statisticList.size() <= Measure::Type::Linearity)
	{
		return;
	}

	m_statisticList[Measure::Type::Linearity].clear();

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

		if (shownOnSchemas == true)
		{
			if (param.location().shownOnSchemas() == false)
			{
				continue;
			}
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

		m_statisticList[Measure::Type::Linearity].push_back(si);
	}


	// sort by position
	//
	StatisticList& list = m_statisticList[Measure::Type::Linearity];
	std::sort(list.begin(), list.end(),
				[](const StatisticsItem& s1, const StatisticsItem& s2)
				{ return s1.positionID() < s2.positionID(); });

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::createComparatorList(bool shownOnSchemas)
{
	QMutexLocker l(&m_signalMutex);

	if (m_statisticList.size() <= Measure::Type::Comparators)
	{
		return;
	}

	m_statisticList[Measure::Type::Comparators].clear();

	QElapsedTimer responseTime;
	responseTime.start();

	std::vector<Metrology::Signal*> signalList;

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

		if (shownOnSchemas == true)
		{
			if (param.location().shownOnSchemas() == false)
			{
				continue;
			}
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

		if (param.hasComparators() == false)
		{
			continue;
		}

		signalList.push_back(pSignal);
	}

	std::sort(signalList.begin(), signalList.end(),
				[](Metrology::Signal* s1, Metrology::Signal* s2)
				{ return s1->param().location().positionID() < s2->param().location().positionID(); });

	for(Metrology::Signal* pSignal : signalList)
	{
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

        int comparatorCount = pSignal->param().comparatorCount();
        for(int c = 0; c < comparatorCount; c++)
		{
            StatisticsItem si(pSignal, pSignal->param().comparator(c));

			/*
			if (param.isInternal() == true)
			{
				if (ERR_METROLOGY_CONNECTION_TYPE(si.connectionType()) == true)
				{
					continue;
				}
			}
			*/

			m_statisticList[Measure::Type::Comparators].push_back(si);
		}


	}

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem StatisticsBase::item(int index) const
{
	if (Measure::ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return StatisticsItem();
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_statisticList[static_cast<quint64>(m_measureType)].size()))
	{
		return StatisticsItem();
	}

	return m_statisticList[static_cast<quint64>(m_measureType)][static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem StatisticsBase::item(int measureType, int index) const
{
	if (Measure::ERR_MEASURE_TYPE(measureType) == true)
	{
		return StatisticsItem();
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_statisticList[static_cast<quint64>(measureType)].size()))
	{
		return StatisticsItem();
	}

	return m_statisticList[static_cast<quint64>(measureType)][static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsItem* StatisticsBase::itemPtr(int measureType, int index)
{
	if (Measure::ERR_MEASURE_TYPE(measureType) == true)
	{
		return nullptr;
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_statisticList[static_cast<quint64>(measureType)].size()))
	{
		return nullptr;
	}

	return &m_statisticList[static_cast<quint64>(measureType)][static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsBase::setItem(int measureType, int index, const StatisticsItem& item)
{
	if (Measure::ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_statisticList[static_cast<quint64>(measureType)].size()))
	{
		return;
	}

	m_statisticList[static_cast<quint64>(measureType)][static_cast<quint64>(index)] = item;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

