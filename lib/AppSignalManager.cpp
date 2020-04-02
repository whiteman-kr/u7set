#include "../lib/AppSignalManager.h"


AppSignalManager::AppSignalManager(QObject* parent) :
	QObject(parent)
{
	{
		QMutexLocker l(&m_paramsMutex);
		m_signalParams.reserve(128000);
	}

	{
		QMutexLocker l(&m_statesMutex);
		m_signalStates.reserve(128000);
	}

	return;
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

	// Commented for reasone, it is not nice, but... ((((
	// m_setpoints is filled in monitor on slot configuration arrived,
	// after it the reconnection happens and AppSignalManager::reset() is called (theSignals.reset())
	// what resets m_setpoints (which was already filled in slot).
	// So, this reset is commented and it will not take place
	//

//	{
//		if (m_setpoints != nullptr)
//		{
//			m_setpoints->clear();
//		}
//	}

	return;
}

void AppSignalManager::addSignal(const AppSignalParam& appSignal)
{
	QMutexLocker l(&m_paramsMutex);
	m_signalParams[appSignal.hash()] = appSignal;
	return;
}

void AppSignalManager::addSignals(const std::vector<AppSignalParam>& appSignals)
{
	QMutexLocker l(&m_paramsMutex);

	for (const AppSignalParam& s : appSignals)
	{
		m_signalParams[s.hash()] = s;
	}

	return;
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

void AppSignalManager::invalidateSignalStates()
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

	AppSignalState& storedState = m_signalStates[signalHash];

	if (state.time().system.timeStamp >= storedState.time().system.timeStamp)
	{
		storedState = state;
	}
	else
	{
		// if difference more then 1h, something wrong and we update state
		//
		qint64 diff = storedState.time().system.timeStamp - state.time().system.timeStamp;

		if (diff > 1_hour)
		{
			storedState = state;
		}
	}

	return;
}

void AppSignalManager::setState(const std::vector<AppSignalState>& states)
{
	QMutexLocker l(&m_statesMutex);

	for (const AppSignalState& state : states)
	{
		AppSignalState& storedState = m_signalStates[state.hash()];

		if (state.time().system.timeStamp >= storedState.time().system.timeStamp)
		{
			storedState = state;
		}
		else
		{
			// if difference more then 1h, something wrong and we update state
			//
			qint64 diff = storedState.time().system.timeStamp - state.time().system.timeStamp;

			if (diff > 1_hour)
			{
				storedState = state;
			}
			else
			{
				// Skip setting state
				//
//				if (state.time().system.timeStamp != 0)
//				{
//					static int aaa = 0;
//					qDebug() << aaa++ << " Skip setting state, diff is " << diff << " storedTime: " << storedState.time().system.toDate() << ", State: " << state.time().system.toDate();
//				}
			}
		}
	}

	return;
}

void AppSignalManager::setSetpoints(ComparatorSet&& setpoints)
{
	m_setpoints = std::move(setpoints);
}

void AppSignalManager::setSetpoints(const ComparatorSet& setpoints)
{
	m_setpoints = setpoints;
}

bool AppSignalManager::signalExists(Hash hash) const
{
	QMutexLocker l(&m_paramsMutex);

	auto result = m_signalParams.find(hash);
	return result != m_signalParams.end();
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

bool AppSignalManager::signalExists(const QString& appSignalId) const
{
	Hash signalHash = ::calcHash(appSignalId);
	return signalExists(signalHash);
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

AppSignalParam AppSignalManager::signalParam(const QString& appSignalId, bool* found) const
{
	Hash signalHash = ::calcHash(appSignalId);
	return signalParam(signalHash, found);
}

AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found) const
{
	if (signalHash == 0)
	{
		return AppSignalState();
	}

	emit addSignalToPriorityList(signalHash);

	QMutexLocker l(&m_statesMutex);

	auto foundState = m_signalStates.find(signalHash);

	if (found != nullptr)
	{
		*found = !(foundState == m_signalStates.end());
	}

	if (foundState != m_signalStates.end())
	{
		return foundState->second;
	}
	else
	{
		AppSignalState result;
		result.m_flags.valid = false;

		return result;
	}
}

AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
{
	Hash h = ::calcHash(appSignalId);
	return signalState(h, found);
}

void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
{
	if (result == nullptr)
	{
		assert(result);
		return;
	}

	emit addSignalsToPriorityList(QVector<Hash>::fromStdVector(appSignalHashes));

	QMutexLocker l(&m_statesMutex);

	int foundCount = 0;

	for (const Hash& signalHash : appSignalHashes)
	{
		auto foundState = m_signalStates.find(signalHash);

		if (foundState != m_signalStates.end())
		{
			result->push_back(foundState->second);
			foundCount ++;
		}
		else
		{
			AppSignalState state;				// Non valid state, hash will be 0 or something like UNDEFINED
			state.m_flags.valid = false;

			result->push_back(state);
		}
	}

	if (found != nullptr)
	{
		*found = foundCount;
	}

	return;
}

void AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
{
	std::vector<Hash> appSignalHashes;
	appSignalHashes.reserve(appSignalIds.size());

	for (const QString& id : appSignalIds)
	{
		Hash h = ::calcHash(id);
		appSignalHashes.push_back(h);
	}

	if (appSignalIds.size() != appSignalHashes.size())
	{
		assert(appSignalIds.size() == appSignalHashes.size());
		return;
	}

	signalState(appSignalHashes, result, found);
	return;
}

QStringList AppSignalManager::signalTags(Hash signalHash) const
{
	QStringList result;

	QMutexLocker l(&m_paramsMutex);

	if (auto it = m_signalParams.find(signalHash);
		it != m_signalParams.end())
	{
		result = it->second.tagStringList();
	}

	return result;
}

QStringList AppSignalManager::signalTags(const QString& appSignalId) const
{
	return signalTags(::calcHash(appSignalId));
}

bool AppSignalManager::signalHasTag(Hash signalHash, const QString& tag) const
{
	QMutexLocker l(&m_paramsMutex);

	auto result = m_signalParams.find(signalHash);
	return result == m_signalParams.end() ? false : result->second.hasTag(tag);
}

bool AppSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
{
	return signalHasTag(::calcHash(appSignalId), tag);
}


std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInputSignalId(const QString& appSignalId) const
{
	QVector<std::shared_ptr<Comparator>> comparators = m_setpoints.getByInputSignalID(appSignalId);

	std::vector<std::shared_ptr<Comparator>> result;
	result.reserve(comparators.size());

	for (const auto& c : comparators)
	{
		result.push_back(c);
	}

	return result;
}
