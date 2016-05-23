#include "../include/AppSignalManager.h"

AppSignalManager::AppSignalManager()
{

}

AppSignalManager::~AppSignalManager()
{

}

void AppSignalManager::reset()
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

void AppSignalManager::addSignal(const Signal& signal)
{
	QMutexLocker l(&m_paramMutex);

	m_signals[signal.hash()] = signal;

	return;
}

void AppSignalManager::setState(Hash signalHash, const AppSignalState& state)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	QMutexLocker l(&m_stateMutex);

	m_states[signalHash] = state;

	return;
}

AppSignalState AppSignalManager::signalState(Hash signalHash)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return AppSignalState();
	}

	QMutexLocker l(&m_stateMutex);

	return m_states[signalHash];
}

AppSignalState AppSignalManager::signalState(const QString& appSignalId)
{
	Hash h = ::calcHash(appSignalId);
	return signalState(h);
}
