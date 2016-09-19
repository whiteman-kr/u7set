#include "../lib/AppSignalManager.h"

AppSignalManager::AppSignalManager()
{

}

AppSignalManager::~AppSignalManager()
{

}

void AppSignalManager::reset()
{
	{
		QMutexLocker l(&m_unitsMutex);
		m_units.clear();
	}

	{
		QMutexLocker l(&m_paramsMutex);
		m_signals.clear();
	}

	{
		QMutexLocker l(&m_statesMutex);
		m_states.clear();
	}

	return;
}

void AppSignalManager::setUnits(const std::vector<AppSignalUnits>& units)
{
	QMutexLocker l(&m_unitsMutex);

	m_units.clear();

	for (const AppSignalUnits& u : units)
	{
		m_units[u.id] = u.unit;
	}

	return;
}

std::map<int, QString> AppSignalManager::units() const
{
	QMutexLocker l(&m_unitsMutex);
	return std::map<int, QString>(m_units);
}

QString AppSignalManager::units(int id) const
{
	QMutexLocker l(&m_unitsMutex);

	auto it = m_units.find(id);

	if (it == m_units.end())
	{
		return QString();
	}
	else
	{
		return it->second;
	}
}

void AppSignalManager::addSignal(const Signal& signal)
{
	QMutexLocker l(&m_paramsMutex);

	m_signals[signal.hash()] = signal;

	return;
}

std::vector<Signal> AppSignalManager::signalList() const
{
	QMutexLocker l(&m_paramsMutex);

	std::vector<Signal> result;
	result.reserve(m_signals.size());

	for (auto& s : m_signals)
	{
		result.push_back(s.second);
	}

	return result;
}

std::vector<Hash> AppSignalManager::signalHashes() const
{
	QMutexLocker l(&m_paramsMutex);

	std::vector<Hash> result;
	result.reserve(m_signals.size());

	for (auto& s : m_signals)
	{
		result.push_back(s.first);
	}

	return result;
}

bool AppSignalManager::signal(const QString& appSignalId, Signal* out) const
{
	Hash h = ::calcHash(appSignalId);
	return signal(h, out);
}

bool AppSignalManager::signal(Hash signalHash, Signal* out) const
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	QMutexLocker l(&m_paramsMutex);

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

Signal AppSignalManager::signal(const QString& appSignalId, bool* found) const
{
	Hash signalHash = ::calcHash(appSignalId);

	Signal resultSignal;

	bool ok = signal(signalHash, &resultSignal);

	if (found != nullptr)
	{
		*found = ok;
	}

	return resultSignal;
}

Signal AppSignalManager::signal(Hash signalHash, bool* found) const
{
	Signal resultSignal;

	bool ok = signal(signalHash, &resultSignal);

	if (found != nullptr)
	{
		*found = ok;
	}

	return resultSignal;
}

void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state)
{
	Hash signalHash = ::calcHash(appSignalId);
	return setState(signalHash, state);
}

void AppSignalManager::setState(Hash signalHash, const AppSignalState& state)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	QMutexLocker l(&m_statesMutex);

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

	QMutexLocker l(&m_statesMutex);

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

int AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result)
{
	if (result == nullptr)
	{
		assert(result);
		return 0;
	}

	int found = 0;
	result->reserve(appSignalHashes.size());

	QMutexLocker l(&m_statesMutex);

	for (const Hash& signalHash : appSignalHashes)
	{
		AppSignalState state;
		state.flags.valid = false;

		auto foundState = m_states.find(signalHash);

		if (foundState != m_states.end())
		{
			state = foundState->second;
			found ++;
		}

		result->push_back(state);
	}

	assert(appSignalHashes.size() == result->size());

	return found;
}

int AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result)
{
	std::vector<Hash> appSignalHashes;
	appSignalHashes.reserve(appSignalIds.size());

	for (const QString& id : appSignalIds)
	{
		Hash h = ::calcHash(id);
		appSignalHashes.push_back(h);
	}

	assert(appSignalIds.size() == appSignalHashes.size());

	return signalState(appSignalHashes, result);
}
