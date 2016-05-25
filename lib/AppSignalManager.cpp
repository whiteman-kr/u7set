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

bool AppSignalManager::signal(Hash signalHash, Signal* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	QMutexLocker l(&m_paramMutex);

	auto result = m_signals.find(signalHash);

	if (result == m_signals.end())
	{
		return false;
	}
	else
	{
		*out = result->second;
		return true;
	}
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

AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return AppSignalState();
	}

	QMutexLocker l(&m_stateMutex);

	AppSignalState result;
	result.flags.valid = false;

	auto foundState = m_states.find(signalHash);

	if (foundState != m_states.end())
	{
		result = foundState->second;
	}

	if (found != nullptr)
	{
		*found = !(foundState == m_states.end());
	}

	return result;
}

AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found)
{
	Hash h = ::calcHash(appSignalId);
	return signalState(h, found);
}
