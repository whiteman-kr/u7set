#include "../lib/AppSignalManager.h"

ScriptSignalManager::ScriptSignalManager(const AppSignalManager* signalManager)	:
	m_signalManager(signalManager)
{
	assert(m_signalManager);
}

QVariant ScriptSignalManager::signalParam(QString signalId) const
{
	bool ok = false;
	AppSignalParam s = m_signalManager->signalParam(signalId, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return QVariant::fromValue(s);
}

QVariant ScriptSignalManager::signalParam(Hash signalHash) const
{
	bool ok = false;
	AppSignalParam s = m_signalManager->signalParam(signalHash, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return QVariant::fromValue(s);
}

QVariant ScriptSignalManager::signalState(QString signalId) const
{
	bool ok = false;
	AppSignalState s = m_signalManager->signalState(signalId, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return QVariant::fromValue(s);
}

QVariant ScriptSignalManager::signalState(Hash signalHash) const
{
	bool ok = false;
	AppSignalState s = m_signalManager->signalState(signalHash, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return QVariant::fromValue(s);
}


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
		m_signalParams.clear();
	}

	{
		QMutexLocker l(&m_statesMutex);
		m_signalStates.clear();
	}

	return;
}

void AppSignalManager::setUnits(const std::vector<AppSignalUnits>& units)
{
	std::map<int, QString> unitsCopy;

	{
		QMutexLocker l(&m_unitsMutex);

		m_units.clear();

		for (const AppSignalUnits& u : units)
		{
			m_units[u.id] = u.unit;
		}

		unitsCopy = m_units;
	}

	//  units in appsignals
	//
	{
		QMutexLocker l(&m_paramsMutex);

		for (auto& spair : m_signalParams)		// & is must be here, in other case pair will be a copy and AppSignalParam will be copy also
		{
			AppSignalParam& s = spair.second;
			auto foundUnitIt = unitsCopy.find(s.unitId());

			if (foundUnitIt == unitsCopy.end())
			{
				qDebug() << Q_FUNC_INFO << " Can't find unit, UnitID = " << s.unitId() << ", AppSiagnalID = " << s.appSignalId();
			}
			else
			{
				s.setUnit(foundUnitIt->second);
			}
		}
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

void AppSignalManager::addSignal(const AppSignalParam& signal)
{
	QMutexLocker l(&m_paramsMutex);

	m_signalParams[signal.hash()] = signal;

	return;
}

std::vector<AppSignalParam> AppSignalManager::signalList() const
{
	QMutexLocker l(&m_paramsMutex);

	std::vector<AppSignalParam> result;
	result.reserve(m_signalParams.size());

	for (auto& s : m_signalParams)
	{
		result.push_back(s.second);
	}

	return result;
}

std::vector<Hash> AppSignalManager::signalHashes() const
{
	QMutexLocker l(&m_paramsMutex);

	std::vector<Hash> result;
	result.reserve(m_signalParams.size());

	for (auto& s : m_signalParams)
	{
		result.push_back(s.first);
	}

	return result;
}

AppSignalParam AppSignalManager::signalParam(const QString& appSignalId, bool* found) const
{
	Hash signalHash = ::calcHash(appSignalId);

	return signalParam(signalHash, found);
}

AppSignalParam AppSignalManager::signalParam(Hash signalHash, bool* found) const
{
	QMutexLocker l(&m_paramsMutex);

	auto result = m_signalParams.find(signalHash);

	if (result == m_signalParams.end())
	{
		if (found != nullptr)
		{
			*found = false;
		}

		return AppSignalParam();
	}

	if (found != nullptr)
	{
		*found = true;
	}

	return result->second;
}

void AppSignalManager::invalidateAllSignalStates()
{
	QMutexLocker l(&m_statesMutex);

	for (auto it = m_signalStates.begin(); it != m_signalStates.end(); ++it)
	{
		it->second.m_flags.valid = false;
	}

	return;
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

	m_signalStates[signalHash] = state;

	return;
}

AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found) const
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return AppSignalState();
	}

	QMutexLocker l(&m_statesMutex);

	AppSignalState result;
	result.m_flags.valid = false;

	auto foundState = m_signalStates.find(signalHash);

	if (foundState != m_signalStates.end())
	{
		result = foundState->second;
	}

	if (found != nullptr)
	{
		*found = !(foundState == m_signalStates.end());
	}

	return result;
}

AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
{
	Hash h = ::calcHash(appSignalId);
	return signalState(h, found);
}

int AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result) const
{
	if (result == nullptr)
	{
		assert(result);
		return 0;
	}

	int found = 0;

	result->clear();
	result->reserve(appSignalHashes.size());

	QMutexLocker l(&m_statesMutex);

	for (const Hash& signalHash : appSignalHashes)
	{
		AppSignalState state;
		state.m_flags.valid = false;

		auto foundState = m_signalStates.find(signalHash);

		if (foundState != m_signalStates.end())
		{
			state = foundState->second;
			found ++;
		}

		result->push_back(state);
	}

	assert(appSignalHashes.size() == result->size());

	return found;
}

int AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result) const
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



