#include "StatisticBase.h"

#include "SignalBase.h"
#include "MeasureView.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticBase::StatisticBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

 StatisticBase::~StatisticBase()
 {
 }

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();
		m_signalCount = 0;

		m_measuredCount = 0;
		m_invalidMeasureCount = 0;

	m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::createSignalList()
{
	m_signalMutex.lock();

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

			if (param.isInternal() == true)
			{
				continue;
			}

			if (param.isInput() == true || param.isOutput() == true)
			{
				if (param.electricUnitID() == E::ElectricUnit::NoUnit)
				{
					continue;
				}
			}

			if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
			{
				continue;
			}

			m_signalList.append(pSignal);
		}

		m_signalCount = m_signalList.count();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* StatisticBase::signal(int index) const
{
	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 || index < m_signalList.count())
		{
			pSignal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::updateState(QTableView* pView)
{
	MeasureView* pMeasureView = dynamic_cast<MeasureView*> (pView);
	if (pMeasureView == nullptr)
	{
		return;
	}

	m_signalMutex.lock();

		m_measuredCount = 0;
		m_invalidMeasureCount = 0;

		int count = m_signalList.count();
		for(int i = 0; i < count; i++)
		{
			Metrology::Signal* pSignal = m_signalList[i];
			if (pSignal == nullptr)
			{
				continue;
			}

			Metrology::SignalStatistic ss = pMeasureView->table().m_measureBase.getSignalStatistic(pSignal->param().hash());
			pSignal->setStatistic(ss);

			if (pSignal->statistic().measureCount() != 0)
			{
				m_measuredCount++;
			}

			if (pSignal->statistic().state() == Metrology::StatisticStateFailed)
			{
				m_invalidMeasureCount ++;
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

