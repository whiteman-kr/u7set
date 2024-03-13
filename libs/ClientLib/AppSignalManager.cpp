#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "./include/ClientLib/AppSignalManager.h"

namespace ClientLib
{
	AppSignalManager::AppSignalManager(ILogFile* logFile, QObject* parent) :
		QObject(parent),
		m_logFile(logFile, "SignalManager")
	{
		{
			QWriteLocker wl(&m_paramsLocker);
			m_signalParams.reserve(64000);
			m_signalParamByEquipmentId.reserve(64000);
		}

		{
			QWriteLocker wl(&m_statesLocker);
			m_states.max_load_factor(0.75);
			m_states.reserve(64000);
		}

		return;
	}

	void AppSignalManager::reset()
	{
		{
			QWriteLocker wl(&m_paramsLocker);
			m_signalParams.clear();
			m_signalParamByEquipmentId.clear();
			m_tagToAppSignals.clear();
			m_tags.clear();
			m_appDataServiceToSignalHashList.clear();
		}

		{
			QWriteLocker wl(&m_statesLocker);
			m_states.clear();
		}

		m_setpoints.clear();	// m_setpoints is threadself itslef

		// --
		//
		notifySignalParamsUpdated();

		return;
	}

	void AppSignalManager::notifySignalParamsUpdated()
	{
		emit signalParamsUpdated();
		return;
	}

	void AppSignalManager::addSignal(const AppSignalParam& appSignal, const QString& appDataServiceId)
	{
		QWriteLocker wl(&m_paramsLocker);

		addSignalPrivate(appSignal, appDataServiceId);

		return;
	}

	void AppSignalManager::addSignals(const std::vector<AppSignalParam>& appSignals, const QString& appDataServiceId)
	{
		QWriteLocker wl(&m_paramsLocker);

		for (const AppSignalParam& s : appSignals)
		{
			addSignalPrivate(s, appDataServiceId);
		}

		return;
	}

	void AppSignalManager::addSignalPrivate(const AppSignalParam& appSignal, const QString& appDataServiceId)
	{
		m_signalParams.emplace(appSignal.hash(), appSignal);

		// Actually, EquipmentID does not starts from the symbol '@',
		// but we need it particularly for Monitor to distinct AppSignalID from EquipmentID.
		//
		m_signalParamByEquipmentId[QStringLiteral("@") + appSignal.equipmentId()] = appSignal.appSignalId();

		// --
		//
		m_appDataServiceToSignalHashList[appDataServiceId].insert(appSignal.hash());

		// Add tags to m_signaIdsByTag
		//
		const QString& appSignalId = appSignal.appSignalId();
		const std::set<QString>& tags = appSignal.tags();

		for (const QString& tag : tags)
		{
			QStringList& l = m_tagToAppSignals[tag];

			if (l.isEmpty() == true)
			{
				l.reserve(512);
			}

			l.push_back(appSignalId);
		}

		// Add tags to commot tag set
		//
		m_tags.insert(tags.begin(), tags.end());

		return;
	}

	void AppSignalManager::addRecentAppSignal(Hash hash)
	{
		QMutexLocker locker(&m_recentUsedMutex);
		m_recentUsed.add(hash);
	}

	void AppSignalManager::addRecentAppSignals(const std::vector<Hash>& hashes)
	{
		QMutexLocker locker(&m_recentUsedMutex);
		m_recentUsed.add(hashes);
	}

	std::vector<Hash> AppSignalManager::recentlyUsedAppSignals(const QString& appDataServiceId)
	{
		std::vector<Hash> result;

		{
			QMutexLocker locker(&m_recentUsedMutex);
			m_recentUsed.removeOutdated();

			result = m_recentUsed.hashes();
		}

		filterByDataService(appDataServiceId, result);
	
		return result;
	}

	bool AppSignalManager::hasRecentlyUsedAppSignals()
	{
		QMutexLocker locker(&m_recentUsedMutex);
		return m_recentUsed.hashes().empty() == false;
	}

	std::vector<Hash> AppSignalManager::signalHashes() const
	{
		QReadLocker rl(&m_paramsLocker);

		std::vector<Hash> result;
		result.reserve(m_signalParams.size());

		for (auto& s : m_signalParams)
		{
			result.push_back(s.first);
		}

		return result;
	}

	void AppSignalManager::invalidateSignalStates(Qt::HANDLE sourceThreadId)
	{
		QWriteLocker wl(&m_statesLocker);

		for (auto&[signalHash, source] : m_states)
		{
			source.invalidateSource(sourceThreadId);
		}

		return;
	}

	void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId)
	{
		Hash signalHash = ::calcHash(appSignalId);
		return setState(signalHash, state, dataServerHash, sourceThreadId);
	}

	void AppSignalManager::setState(Hash signalHash, const AppSignalState& arrivedState, Hash dataServerHash, Qt::HANDLE sourceThreadId)
	{
		if (signalHash == 0)
		{
			assert(signalHash != 0);
			return;
		}

		QWriteLocker wl(&m_statesLocker);

		Sources& currentState = m_states[signalHash];
		currentState.set(arrivedState, dataServerHash, sourceThreadId);

		return;
	}

	void AppSignalManager::setState(const std::vector<AppSignalState>& states, Hash dataServerHash, Qt::HANDLE sourceThreadId)
	{
		QWriteLocker wl(&m_statesLocker);

		for (const AppSignalState& newState : states)
		{
			Sources& currentStateAndSources = m_states[newState.hash()];
			currentStateAndSources.set(newState, dataServerHash, sourceThreadId);
		}

		return;
	}

	void AppSignalManager::setSetpoints(ComparatorSet&& setpoints)
	{
		m_setpoints = std::move(setpoints);
		return;
	}

	void AppSignalManager::setSetpoints(const ComparatorSet& setpoints)
	{
		m_setpoints = setpoints;
		return;
	}

	int AppSignalManager::signalsCount() const
	{
		QReadLocker rl(&m_paramsLocker);
		return static_cast<int>(m_signalParams.size());
	}

	std::vector<AppSignalParam> AppSignalManager::signalList() const
	{
		QReadLocker rl(&m_paramsLocker);

		std::vector<AppSignalParam> result;
		result.reserve(m_signalParams.size());

		for (auto& s : m_signalParams)
		{
			result.push_back(s.second);
		}

		return result;
	}

	bool AppSignalManager::signalExists(Hash hash) const
	{
		QReadLocker rl(&m_paramsLocker);
		return m_signalParams.contains(hash);
	}

	bool AppSignalManager::signalExists(const QString& appSignalId) const
	{
		return signalExists(::calcHash(appSignalId));
	}

	bool AppSignalManager::signalsExist(const QStringList& signalIds) const
	{
		QReadLocker rl(&m_paramsLocker);
		return std::all_of(signalIds.begin(), signalIds.end(), [this](const QString& appSignalId) {
			return m_signalParams.contains(::calcHash(appSignalId));
		});
	}

	AppSignalParam AppSignalManager::signalParam(Hash signalHash, bool* found) const
	{
		AppSignalParam result;

		QReadLocker rl(&m_paramsLocker);

		auto it = m_signalParams.find(signalHash);

		if (found != nullptr)
		{
			*found = it != m_signalParams.end();
		}

		if (it != m_signalParams.end())
		{
			result = it->second;
		}

		return result;
	}

	AppSignalParam AppSignalManager::signalParam(const QString& appSignalId, bool* found) const
	{
		Hash signalHash = ::calcHash(appSignalId);
		return signalParam(signalHash, found);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found) const
	{
		return signalState(signalHash, {}, found);
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
	{
		Hash h = ::calcHash(appSignalId);
		return signalState(h, {}, found);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, Hash dataServerHash, bool* found) const
	{
		AppSignalState result;

		if (signalHash == UNDEFINED_HASH)
		{
			if (found != nullptr)
			{
				*found = false;
			}

			return result;
		}

		const_cast<AppSignalManager*>(this)->addRecentAppSignal(signalHash);

		QReadLocker rl(&m_statesLocker);

		auto foundState = m_states.find(signalHash);

		if (found != nullptr)
		{
			*found = !(foundState == m_states.end());
		}

		if (foundState != m_states.end())
		{
			if (dataServerHash == UNDEFINED_HASH)
			{
				result = foundState->second.get();
			}
			else
			{
				result = foundState->second.getForDataServer(dataServerHash);
			}
		}
		else
		{
			result.m_hash = signalHash;
			result.m_flags.valid = false;
		}

		return result;
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, const QString& dataServerId, bool* found) const
	{
		Hash signalHash = ::calcHash(appSignalId);
		Hash dataServerHash = ::calcHash(dataServerId);

		return signalState(signalHash, dataServerHash, found);
	}

	// Ok
	//
	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		return signalState(appSignalHashes, {}, result, found);
	}

	void AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	{
		return signalState(appSignalIds, {}, result, found);
	}

	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, Hash dataServerHash, std::vector<AppSignalState>* result, int* found) const
	{
		if (result == nullptr)
		{
			assert(result);
			return;
		}

		result->clear();
		result->reserve(appSignalHashes.size());

		const_cast<AppSignalManager*>(this)->addRecentAppSignals(appSignalHashes);

		int foundCount = 0;

		{
			QReadLocker rl(&m_statesLocker);

			for (Hash signalHash : appSignalHashes)
			{
				auto foundState = m_states.find(signalHash);

				if (foundState != m_states.end())
				{
					if (dataServerHash == UNDEFINED_HASH)
					{
						result->push_back(foundState->second.get());
					}
					else
					{
						result->push_back(foundState->second.getForDataServer(dataServerHash));
					}

					foundCount ++;
				}
				else
				{
					AppSignalState state;
					state.m_hash = signalHash;
					state.m_flags.valid = false;
					state.m_flags.stateAvailable = false;

					result->push_back(state);
				}
			}
		}

		if (found != nullptr)
		{
			*found = foundCount;
		}

		return;
	}

	void AppSignalManager::signalState(const std::vector<QString>& appSignalIds, const QString& dataServerId, std::vector<AppSignalState>* result, int* found) const
	{
		std::vector<Hash> appSignalHashes;
		appSignalHashes.reserve(appSignalIds.size());

		for (const QString& id : appSignalIds)
		{
			Hash h = ::calcHash(id);
			appSignalHashes.push_back(h);
		}

		Hash dataServerHash = ::calcHash(dataServerId);

		return signalState(appSignalHashes, dataServerHash, result, found);
	}

	QStringList AppSignalManager::signalTags(Hash signalHash) const
	{
		QStringList result;

		QReadLocker rl(&m_paramsLocker);

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
		QReadLocker rl(&m_paramsLocker);

		auto result = m_signalParams.find(signalHash);
		return result == m_signalParams.end() ? false : result->second.hasTag(tag);
	}

	bool AppSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
	{
		return signalHasTag(::calcHash(appSignalId), tag);
	}

	E::SignalType AppSignalManager::signalType(Hash signalHash, bool* found) const
	{
		QReadLocker rl(&m_paramsLocker);

		auto result = m_signalParams.find(signalHash);

		if (found != nullptr)
		{
			*found = (result != m_signalParams.end());
		}

		return result == m_signalParams.end() ?
					E::SignalType::Discrete :
					result->second.type();
	}

	QStringList AppSignalManager::signalIdsByTag(const QString& tag) const
	{
		QReadLocker rl(&m_paramsLocker);

		auto it = m_tagToAppSignals.find(tag);
		if (it == m_tagToAppSignals.end())
		{
			return {};
		}
		else
		{
			return it->second;
		}
	}

	E::SignalType AppSignalManager::signalType(const QString& appSignalId, bool* found) const
	{
		return signalType(::calcHash(appSignalId), found);
	}

	QString AppSignalManager::equipmentToAppSignalId(const QString& equipmentId) const
	{
		QString result;

		{
			QReadLocker rl(&m_paramsLocker);

			auto it = m_signalParamByEquipmentId.find(equipmentId);
			if (it != m_signalParamByEquipmentId.end())
			{
				result = it->second;
			}
		}

		return result;
	}


	std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInputSignalId(const QString& appSignalId) const
	{
		std::vector<std::shared_ptr<Comparator>> comparators = m_setpoints.getByInputSignalID(appSignalId);

		std::vector<std::shared_ptr<Comparator>> result;
		result.reserve(comparators.size());

		for (const auto& c : comparators)
		{
			result.push_back(c);
		}

		return result;
	}

	/// Get AppDataService EquipmentIDs list by AppSignalID.
	///
	QStringList AppSignalManager::dataServiceIds(const QString& appSignalId) const
	{
		QReadLocker rl(&m_paramsLocker);

		Hash hash = calcHash(appSignalId);

		QStringList result;
		for (const auto& [appDataServcieId, signalSet] : m_appDataServiceToSignalHashList)
		{
			if (signalSet.contains(hash) == true)
			{
				result.push_back(appDataServcieId);
			}
		}

		return result;
	}

	/// Return true if AppDataService contains signal.
	///
	bool AppSignalManager::dataServiceHasSignal(const QString& serviceEquipmentId, const QString& appSignalId) const
	{
		Hash hash = calcHash(appSignalId);
		return dataServiceHasSignal(serviceEquipmentId, hash);
	}

	bool AppSignalManager::dataServiceHasSignal(const QString& serviceEquipmentId, Hash signalHash) const
	{
		QReadLocker rl(&m_paramsLocker);

		auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
		if (it == m_appDataServiceToSignalHashList.end())
		{
			return false;
		}

		return it->second.contains(signalHash);
	}

	void AppSignalManager::filterByDataService(const QString& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const
	{
		QReadLocker rl(&m_paramsLocker);

		auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
		if (it == m_appDataServiceToSignalHashList.end())
		{
			inOutSignalHashes.clear();
			return;
		}

		const std::unordered_set<Hash>& sh = it->second;

		// Filter all signals which are not belong to serviceEquipmentId.
		//
		std::erase_if(inOutSignalHashes, [&sh, this](Hash hash) {
			return sh.contains(hash) == false;
		});

		return;
	}

	std::vector<Hash> AppSignalManager::dataServiceSignals(const QString& serviceEquipmentId) const
	{
		std::vector<Hash> result;

		QReadLocker rl(&m_paramsLocker);

		auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
		if (it != m_appDataServiceToSignalHashList.end())
		{
			const auto& signalsByDataService = it->second;

			result.reserve(signalsByDataService.size());
			std::copy(signalsByDataService.begin(), signalsByDataService.end(), std::back_inserter(result));
		}

		return result;
	}

	QStringList AppSignalManager::tags() const
	{
		QReadLocker rl(&m_paramsLocker);

		QStringList result;
		result.reserve(m_tags.size());

		for (const QString& t : m_tags)
		{
			result.push_back(t);
		}

		return result;
	}


	AppSignalParam AppSignalManager::signalParamByEquipemntId(const QString& equipmentId, bool* found) const
	{
		Hash appSignalIdHash = UNDEFINED_HASH;

		{
			QReadLocker rl(&m_paramsLocker);

			auto it = m_signalParamByEquipmentId.find(equipmentId);
			if (it != m_signalParamByEquipmentId.end())
			{
				appSignalIdHash = ::calcHash(it->second);
			}
		}

		return signalParam(appSignalIdHash, found);
	}

	std::vector<AppSignalManager::SourceState> AppSignalManager::signalStateAllSources(const QString& appSignalId) const
	{
		std::vector<AppSignalManager::SourceState> result;
		result.reserve(4);

		QReadLocker rl(&m_statesLocker);

		auto foundState = m_states.find(::calcHash(appSignalId));
		if (foundState != m_states.end())
		{
			const Sources& sources = foundState->second;

			for (const auto& source : sources.sources)
			{
				result.push_back(source);
			}
		}

		return result;
	}

	void AppSignalManager::Sources::set(const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId)
	{
		SourceState* emptyState = nullptr;
		for (SourceState& sourceState : sources)
		{
			if (sourceState.sourceThreadId == sourceThreadId)
			{
				sourceState.state = state;
				sourceState.lastUpdateTime = std::chrono::system_clock::now();
				return;
			}

			if (sourceState.sourceThreadId == nullptr)
			{
				emptyState = &sourceState;
			}
		}

		if (emptyState == nullptr)
		{
			// No emty space in sources
			//
			Q_ASSERT(emptyState);

			// Try to mitigate it, and set value to the last item
			//
			emptyState = &sources.back();
		}

		*emptyState = SourceState{state, dataServerHash, sourceThreadId, std::chrono::system_clock::now()};

		return;
	}

	void AppSignalManager::Sources::invalidateSource(Qt::HANDLE sourceThreadId)
	{
		for (SourceState& sourceState : sources)
		{
			if (sourceState.sourceThreadId == sourceThreadId)
			{
				sourceState.state = AppSignalState{};
				sourceState.lastUpdateTime = std::chrono::system_clock::now();
				break;
			}
		}

		return;
	}

	const AppSignalState& AppSignalManager::Sources::get() const
	{
		// Find the newest available state
		//
		const SourceState* stateAvailable = nullptr;
		const SourceState* stateNewest = nullptr;

		for (const SourceState& sourceState : sources)
		{
			if (sourceState.sourceThreadId == 0)
			{
				continue;
			}

			if (sourceState.state.isStateAvailable() == true)
			{
				if (stateAvailable == nullptr ||
					stateAvailable->state.time().plant < sourceState.state.time().plant)
				{
					stateAvailable = &sourceState;	// the first state with state available flag
				}
			}
			else
			{
				// sourceState.state.isStateAvailable() == false
				//
				if (stateNewest == nullptr ||
					stateNewest->lastUpdateTime < sourceState.lastUpdateTime)
				{
					stateNewest = &sourceState;
				}
			}
		}

		if (stateAvailable != nullptr)
		{
			return stateAvailable->state;
		}

		if (stateNewest != nullptr)
		{
			return stateNewest->state;
		}

		static const AppSignalState NotValidState{};
		return NotValidState;
	}


	const AppSignalState& AppSignalManager::Sources::getForDataServer(Hash dataServerHash) const
	{
		// Find the newest available state
		//
		const SourceState* stateAvailable = nullptr;
		const SourceState* stateNewest = nullptr;

		for (const SourceState& sourceState : sources)
		{
			if (sourceState.sourceThreadId == 0 || sourceState.dataServerHash != dataServerHash)
			{
				continue;
			}

			if (sourceState.state.isStateAvailable() == true)
			{
				if (stateAvailable == nullptr ||
					stateAvailable->state.time().plant < sourceState.state.time().plant)
				{
					stateAvailable = &sourceState;	// the first state with state available flag
				}
			}
			else
			{
				// sourceState.state.isStateAvailable() == false
				//
				if (stateNewest == nullptr ||
					stateNewest->lastUpdateTime < sourceState.lastUpdateTime)
				{
					stateNewest = &sourceState;
				}
			}
		}

		if (stateAvailable != nullptr)
		{
			return stateAvailable->state;
		}

		if (stateNewest != nullptr)
		{
			return stateNewest->state;
		}

		static const AppSignalState NotValidState{};
		return NotValidState;
	}

}
