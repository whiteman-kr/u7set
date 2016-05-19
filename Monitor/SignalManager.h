#ifndef SIGNALSET_H
#define SIGNALSET_H

#include <QMutex>
#include <unordered_map>
#include "../include/Signal.h"

typedef quint64 SignalHash;

class SignalManager
{
public:
	SignalManager();
	virtual ~SignalManager();

private:
	QMutex m_paramMutex;
	//std::unordered_map<SignalHash, Signal> m_signals;

	QMutex m_stateMutex;
	//std::unordered_map<SignalHash, Signal> m_states;

};

#endif // SIGNALSET_H
