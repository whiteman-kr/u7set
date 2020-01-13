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
	QTime responseTime;
	responseTime.start();

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

			if (param.isAnalog() == false)
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

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::createComparatorList()
{
	QTime responseTime;
	responseTime.start();

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
			if (param.isValid() == false || param.hasComparators() == false)
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
				if (param.electricUnitID() == E::ElectricUnit::NoUnit)
				{
					continue;
				}
			}

			if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
			{
				continue;
			}

			m_comparatorList.append(pSignal);
		}

		m_comparatorCount = m_comparatorList.count();

	m_signalMutex.unlock();

	qDebug() << __FUNCTION__ << " Time for create: " << responseTime.elapsed() << " ms";
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

void StatisticBase::updateSignalState(QTableView* pView, Hash signalHash)
{
	MeasureView* pMeasureView = dynamic_cast<MeasureView*> (pView);
	if (pMeasureView == nullptr)
	{
		return;
	}

	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	QTime responseTime;
	responseTime.start();

	Metrology::Signal* pSignal = theSignalBase.signalPtr(signalHash);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	Metrology::SignalStatistic ss = pMeasureView->table().m_measureBase.getSignalStatistic(pSignal->param().hash());
	pSignal->setStatistic(ss);

	m_signalMutex.lock();

		m_measuredCount = 0;
		m_invalidMeasureCount = 0;

		int count = m_signalList.count();
		for(int i = 0; i < count; i++)
		{
			pSignal = m_signalList[i];
			if (pSignal == nullptr)
			{
				continue;
			}

			if (pSignal->statistic().isMeasured() == true)
			{
				m_measuredCount++;
			}

			if (pSignal->statistic().state() == Metrology::SignalStatistic::State::Failed)
			{
				m_invalidMeasureCount ++;
			}
		}

	m_signalMutex.unlock();

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticBase::updateSignalsState(QTableView* pView)
{
	MeasureView* pMeasureView = dynamic_cast<MeasureView*> (pView);
	if (pMeasureView == nullptr)
	{
		return;
	}

	QTime responseTime;
	responseTime.start();

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

			if (pSignal->statistic().isMeasured() == true)
			{
				m_measuredCount++;
			}

			if (pSignal->statistic().state() == Metrology::SignalStatistic::State::Failed)
			{
				m_invalidMeasureCount ++;
			}
		}

	m_signalMutex.unlock();

	qDebug() << __FUNCTION__ << " Time for update: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

