#include "../include/Signal.h"


SignalSet::SignalSet()
{
}


SignalSet::~SignalSet()
{
	clear();
}


void SignalSet::clear()
{
	OrderedHash<int, Signal>::clear();

	m_groupSignals.clear();
}


void SignalSet::append(const int& signalID, const Signal& signal)
{
	OrderedHash<int, Signal>::append(signalID, signal);

	m_groupSignals.insert(signal.signalGroupID(), signalID);
}


void SignalSet::remove(const int& signalID)
{
	Signal signal = value(signalID);

	OrderedHash<int, Signal>::remove(signalID);

	m_groupSignals.remove(signal.signalGroupID(), signalID);
}


void SignalSet::removeAt(const int index)
{
	const Signal& signal = OrderedHash<int, Signal>::operator [](index);

	int signalGroupID = signal.signalGroupID();
	int signalID = signal.ID();

	OrderedHash<int, Signal>::removeAt(index);

	m_groupSignals.remove(signalGroupID, signalID);
}


QVector<int> SignalSet::getChannelSignalsID(const Signal& signal)
{
	return getChannelSignalsID(signal.signalGroupID());
}


QVector<int> SignalSet::getChannelSignalsID(int signalGroupID)
{
	QVector<int> channelSignalsID;

	QList<int> signalsID = m_groupSignals.values(signalGroupID);

	int signalCount = signalsID.count();

	for(int i = 0; i< signalCount; i++)
	{
		channelSignalsID.append(signalsID.at(i));
	}

	return channelSignalsID;
}
