#ifndef SIGNALSET_H
#define SIGNALSET_H

#include <QMutex>
#include <unordered_map>
#include "../include/Hash.h"
#include "../include/Signal.h"

typedef quint64 SignalHash;

class SignalManager
{
public:
	SignalManager();
	virtual ~SignalManager();

public:
	void reset();

	void addSignal(const Signal& signal);

private:
	QMutex m_paramMutex;
	std::unordered_map<Hash, Signal> m_signals;

	QMutex m_stateMutex;
	std::unordered_map<Hash, uint> m_states;

};

extern SignalManager theSignals;

#endif // SIGNALSET_H
