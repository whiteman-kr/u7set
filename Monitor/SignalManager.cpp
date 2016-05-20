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
		//m_signals.clear();
	}

	{
		QMutexLocker l(&m_stateMutex);
		//m_states.clear();
	}

	return;
}
