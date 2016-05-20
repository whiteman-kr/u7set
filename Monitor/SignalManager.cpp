#include "SignalManager.h"

SignalManager theSignals;

SignalManager::SignalManager()
{

}

SignalManager::~SignalManager()
{

}

void SignalManager::reset()
{
	{
		QMutexLocker l(&m_paramMutex);
		m_signals.clear();
	}

	{
		QMutexLocker l(&m_stateMutex);
		m_states.clear();
	}

	return;
}

void SignalManager::addSignal(const Signal& signal)
{
	QMutexLocker l(&m_paramMutex);
	m_signals[signal.hash()] = signal;

	return;
}
