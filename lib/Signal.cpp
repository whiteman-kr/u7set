#include "../include/Signal.h"

Signal::Signal()
{

}


SignalSet::SignalSet()
{
}


SignalSet::~SignalSet()
{
	removeAll();
}


void SignalSet::insert(const Signal& signal)
{
	int signalID = signal.ID();

	if (m_signalSet.contains(signalID))
	{
		assert(false);			// signal with this ID alredy in set

		Signal* oldSignal = m_signalSet.value(signalID);

		*oldSignal = signal;
	}
	else
	{
		Signal* newSignal = new Signal;

		*newSignal = signal;

		m_signalSet.insert(signalID, newSignal);
	}
}


Signal* SignalSet::getSignal(int signalID)
{
	if (m_signalSet.contains(signalID))
	{
		return m_signalSet.value(signalID);
	}

	assert(false);			// signal not found

	return nullptr;
}


void SignalSet::removeAll()
{
	QHashIterator<int, Signal*> i(m_signalSet);

	while (i.hasNext())
	{
		i.next();
		delete i.value();
	}

	m_signalSet.clear();
}
