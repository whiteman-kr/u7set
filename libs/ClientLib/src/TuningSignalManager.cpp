#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include <AppSignal.pb.h>
#include <ClientLib/TuningSignalManager.h>

#define ANY_HASH UNDEFINED_HASH

namespace ClientLib
{
	//
	// TuningSignalManager
	//
	TuningSignalManager::TuningSignalManager(const QString& clientEquipmentId, ILogFile* logFile, QObject* parent) :
		QObject(parent),
		m_tuningClientHash(::calcHash(clientEquipmentId)),
		m_logFile(logFile, "TuningSignalManager"),
		m_recentUsed(MaxRecentCount)
	{
	}

	TuningSignalManager::~TuningSignalManager() {}

	bool TuningSignalManager::load(const QByteArray& data)
	{
		reset();

		::Proto::AppSignalSet message;
		bool ok = message.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			return false;
		}

		ok = load(message);

		return ok;
	}

	bool TuningSignalManager::load(const ::Proto::AppSignalSet& message)
	{
		reset();

		bool ok = true;

		std::unordered_map<Hash, const AppSignalParam> loadedSignals;
		loadedSignals.reserve(message.appsignal_size());

		std::unordered_map<QString, QStringList> tagToAppSignals;
		tagToAppSignals.reserve(256);

		for (int i = 0; i < message.appsignal_size(); i++)
		{
			const ::Proto::AppSignal& appSignalMessage = message.appsignal(i);

			Hash hash = ::calcHash(QString::fromStdString(appSignalMessage.appsignalid()));

			AppSignalParam appSignalParam;

			bool loadSignalOk = appSignalParam.load(appSignalMessage);
			ok &= loadSignalOk;

			if (loadSignalOk == false)
			{
				continue;
			}

			loadedSignals.emplace(hash, appSignalParam);

			// Add tags to m_signaIdsByTag
			//
			const QString& appSignalId = appSignalParam.appSignalId();

			for (const auto& tags = appSignalParam.tags(); const QString& tag : tags)
			{
				QStringList& l = tagToAppSignals[tag];

				if (l.isEmpty() == true)
				{
					l.reserve(1024);
				}

				l.push_back(appSignalId);
			}
		}

		{
			QWriteLocker l(&m_signalsLock);

			std::swap(loadedSignals, m_signals);
			std::swap(tagToAppSignals, m_tagToAppSignals);
		}

		notifySignalParamsUpdated();

		return ok;
	}

	int TuningSignalManager::signalsCount() const
	{
		QReadLocker rl(&m_signalsLock);
		return static_cast<int>(m_signals.size());
	}

	std::vector<Hash> TuningSignalManager::signalHashes() const
	{
		std::vector<Hash> result;
		result.reserve(m_signals.size());

		QReadLocker rl(&m_signalsLock);

		for (const auto& p : m_signals)
		{
			result.push_back(p.first);
		}

		return result;
	}

	std::vector<AppSignalParam> TuningSignalManager::signalList() const
	{
		std::vector<AppSignalParam> result;
		result.reserve(m_signals.size());

		QReadLocker rl(&m_signalsLock);

		for (auto p : m_signals)
		{
			result.emplace_back(p.second);
		}

		return result;
	}

	bool TuningSignalManager::signalExists(Hash hash) const
	{
		QReadLocker rl(&m_signalsLock);
		return m_signals.find(hash) != m_signals.end();
	}

	bool TuningSignalManager::signalExists(const QString& appSignalId) const
	{
		Hash hash = ::calcHash(appSignalId);
		return TuningSignalManager::signalExists(hash);
	}

	bool TuningSignalManager::signalsExist(const QStringList& signalIds) const
	{
		QReadLocker rl(&m_signalsLock);
		return std::all_of(signalIds.begin(),
						   signalIds.end(),
						   [this](const QString& appSignalId)
						   {
							   return m_signals.contains(::calcHash(appSignalId));
						   });
	}

	AppSignalParam TuningSignalManager::signalParam(Hash hash, bool* found) const
	{
		QReadLocker rl(&m_signalsLock);

		auto result = m_signals.find(hash);

		if (result == m_signals.end())
		{
			if (found != nullptr)
			{
				*found = false;
			}

			return {};
		}

		if (found != nullptr)
		{
			*found = true;
		}

		return result->second;
	}

	AppSignalParam TuningSignalManager::signalParam(const QString& appSignalId, bool* found) const
	{
		Hash signalHash = ::calcHash(appSignalId);
		return signalParam(signalHash, found);
	}

	TuningSignalState TuningSignalManager::state(Hash hash, bool* found) const
	{
		if (hash == UNDEFINED_HASH)
		{
			assert(hash != UNDEFINED_HASH);
			return TuningSignalState();
		}

		const_cast<TuningSignalManager*>(this)->addRecentAppSignal(hash);

		std::scoped_lock l(m_statesMutex);

		auto foundState = m_states.find(hash);

		if (found != nullptr)
		{
			*found = !(foundState == m_states.end());
		}

		if (foundState != m_states.end())
		{
			return foundState->second.get();
		}
		else
		{
			TuningSignalState result;
			result.m_flags.valid = false;

			return result;
		}
	}

	TuningSignalState TuningSignalManager::state(const QString& appSignalId, bool* found) const
	{
		Hash signalHash = ::calcHash(appSignalId);
		return state(signalHash, found);
	}

	TuningSignalState TuningSignalManager::state(Hash hash, Hash tuningServiceHash, bool* found) const
	{
		if (hash == UNDEFINED_HASH)
		{
			assert(hash != UNDEFINED_HASH);
			return TuningSignalState();
		}

		const_cast<TuningSignalManager*>(this)->addRecentAppSignal(hash);

		std::scoped_lock l(m_statesMutex);

		auto foundState = m_states.find(hash);

		if (foundState != m_states.end())
		{
			return foundState->second.get(tuningServiceHash, found);
		}
		else
		{
			if (found != nullptr)
			{
				*found = false;
			}

			TuningSignalState result;
			result.m_flags.valid = false;

			return result;
		}
	}

	TuningSignalState TuningSignalManager::state(const QString& appSignalId, Hash tuningServiceHash, bool* found) const
	{
		Hash signalHash = ::calcHash(appSignalId);
		return state(signalHash, tuningServiceHash, found);
	}

	void TuningSignalManager::state(std::span<const Hash> appSignalHashes, std::vector<TuningSignalState>* result, int* found) const
	{
		if (result == nullptr)
		{
			assert(result);
			return;
		}

		const_cast<TuningSignalManager*>(this)->addRecentAppSignals(appSignalHashes);

		result->clear();
		result->reserve(appSignalHashes.size());

		int foundCount = 0;

		{
			std::scoped_lock l(m_statesMutex);

			for (Hash signalHash : appSignalHashes)
			{
				auto foundState = m_states.find(signalHash);

				if (foundState != m_states.end())
				{
					result->push_back(foundState->second.get());
					foundCount++;
				}
				else
				{
					TuningSignalState state;
					state.m_hash = signalHash;
					state.m_flags.valid = false;

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

	void TuningSignalManager::state(std::span<const QString> appSignalIds, std::vector<TuningSignalState>* result, int* found) const
	{
		std::vector<Hash> appSignalHashes;
		appSignalHashes.reserve(appSignalIds.size());

		for (const QString& id : appSignalIds)
		{
			Hash h = ::calcHash(id);
			appSignalHashes.push_back(h);
		}

		return state(appSignalHashes, result, found);
	}


	TuningSignalState TuningSignalManager::queuedState(Hash hash, bool* found) const
	{
		m_recentEnabled = false;
		return state(hash, found);
	}

	TuningSignalState TuningSignalManager::queuedState(const QString& appSignalId, bool* found) const
	{
		m_recentEnabled = false;
		return state(appSignalId, found);
	}

	TuningSignalState TuningSignalManager::queuedState(Hash hash, Hash tuningServiceHash, bool* found) const
	{
		m_recentEnabled = false;
		return state(hash, tuningServiceHash, found);
	}

	TuningSignalState TuningSignalManager::queuedState(const QString& appSignalId, Hash tuningServiceHash, bool* found) const
	{
		m_recentEnabled = false;
		return state(appSignalId, tuningServiceHash, found);
	}

	void TuningSignalManager::queuedState(const std::vector<Hash>& appSignalHashes,
										  std::vector<TuningSignalState>* result,
										  int* found) const
	{
		m_recentEnabled = false;
		return state(appSignalHashes, result, found);
	}

	void TuningSignalManager::queuedState(const std::vector<QString>& appSignalIds,
										  std::vector<TuningSignalState>* result,
										  int* found) const
	{
		m_recentEnabled = false;
		return state(appSignalIds, result, found);
	}

	QStringList TuningSignalManager::signalIdsByTag(const QString& tag) const
	{
		QReadLocker rl(&m_signalsLock);

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

	void TuningSignalManager::reset()
	{
		{
			QWriteLocker l(&m_signalsLock);
			m_signals.clear();
			m_tagToAppSignals.clear();
		}

		{
			std::scoped_lock l(m_statesMutex);
			m_states.clear();

			m_unappliedStates.clear();
			m_allStatesApplied.notify_all();
		}

		return;
	}

	std::vector<Hash> TuningSignalManager::signalHashes(const std::vector<Hash> lmEquipmentIdHashes) const
	{
		std::vector<Hash> result;
		result.reserve(m_signals.size());

		QReadLocker rl(&m_signalsLock);

		for (const auto& p : m_signals)
		{
			const AppSignalParam& param = p.second;
			Hash signalEquipmentHash = ::calcHash(param.lmEquipmentId());

			if (std::find(lmEquipmentIdHashes.begin(), lmEquipmentIdHashes.end(), signalEquipmentHash) != lmEquipmentIdHashes.end())
			{
				result.push_back(p.first);
			}
		}

		return result;
	}

	void TuningSignalManager::invalidateSignalStates(Hash tuningServiceHash)
	{
		std::scoped_lock l(m_statesMutex);

		for (auto& p : m_states)
		{
			p.second.invalidateSource(tuningServiceHash);
		}

		m_unappliedStates.clear();
		m_allStatesApplied.notify_all();

		return;
	}

	void TuningSignalManager::setState(const TuningSignalState& state, Hash tuningServiceHash)
	{
		return setStates({state}, tuningServiceHash);
	}

	void TuningSignalManager::setStates(const std::vector<TuningSignalState>& states, Hash tuningServiceHash)
	{
		struct UnsuccessfulWrite
		{
			TuningValue value;
			Hash appSignalHash = UNDEFINED_HASH;
			int writeErrorCode = 0;
		};

		std::vector<UnsuccessfulWrite> unsuccessfulWrites;

		{
			std::scoped_lock l(m_statesMutex);

			// If writing has been finished - set new values as applied or display a writing error
			//
			for (const TuningSignalState& arrivedState : states)
			{
				if (m_tuningClientHash != arrivedState.writeClient())
				{
					continue;
				}

				auto sourcesIt = m_states.find(arrivedState.hash());
				if (sourcesIt == m_states.end())
				{
					continue;
				}
				Sources& currentSources = sourcesIt->second;

				if (currentSources.isValueUnapplied(tuningServiceHash) == true)
				{
					bool found = false;
					TuningSignalState currentState = currentSources.get(tuningServiceHash, &found);
					if (found == false)
					{
						continue;
					}

					if (static_cast<E::NetworkError>(arrivedState.writeErrorCode()) == E::NetworkError::Success)
					{
						if (arrivedState.successfulWriteTime() > currentState.successfulWriteTime())
						{
							currentSources.setAsApplied(tuningServiceHash);
						}
					}
					else
					{
						if (arrivedState.unsuccessfulWriteTime() > currentState.unsuccessfulWriteTime())
						{
							currentSources.setAsApplied(tuningServiceHash);

							unsuccessfulWrites.push_back(
								{currentSources.getUnappliedValue(), arrivedState.hash(), arrivedState.writeErrorCode()});
						}
					}

					// If value was unapplied and became fully applied - clear its hash from unapplied states set
					//
					if (currentSources.isValueUnapplied() == false)
					{
						// qDebug() << "-Unapplied: " << arrivedState.hash();
						m_unappliedStates.erase(arrivedState.hash());

						if (m_unappliedStates.empty() == true)
						{
							m_allStatesApplied.notify_all();
						}
					}
				}
			}

			// Write new states to states array
			//
			for (const TuningSignalState& arrivedState : states)
			{
				m_states[arrivedState.hash()].set(arrivedState, tuningServiceHash);
			}
		}

		// Log unsuccessful writes
		//
		for (const UnsuccessfulWrite& u : unsuccessfulWrites)
		{
			bool paramFound = false;
			AppSignalParam param = signalParam(u.appSignalHash, &paramFound);
			if (paramFound == false)
			{
				assert(false);
				continue;
			}

			m_logFile.writeAlert(tr("TuningSignalManager::setStates(), Error writing value '%1' to signal '%2' (%3), logic module '%4': %5")
									 .arg(u.value.toString())
									 .arg(param.customSignalId())
									 .arg(param.caption())
									 .arg(param.lmEquipmentId())
									 .arg(E::valueToString(static_cast<E::NetworkError>(u.writeErrorCode))));
		}


		return;
	}

	bool TuningSignalManager::waitForAllApplied(std::chrono::milliseconds timeout) const
	{
		std::unique_lock l(m_statesMutex);
		return m_allStatesApplied.wait_for(l,
										   timeout,
										   [this]()
										   {
											   return m_unappliedStates.empty() == true;
										   });
	}

	void TuningSignalManager::notifySignalParamsUpdated()
	{
		emit signalsLoaded();
	}

	void TuningSignalManager::addRecentAppSignal(Hash h)
	{
		if (m_recentEnabled == true)
		{
			QMutexLocker locker(&m_recentUsedMutex);
			m_recentUsed.add(h);
		}
		m_recentEnabled = true;
	}

	void TuningSignalManager::addRecentAppSignals(std::span<const Hash> hashes)
	{
		if (m_recentEnabled == true)
		{
			QMutexLocker locker(&m_recentUsedMutex);
			m_recentUsed.add(hashes);
		}
		m_recentEnabled = true;
	}

	std::vector<Hash> TuningSignalManager::recentlyUsedAppSignals(const QString& dataServiceId)
	{
		std::vector<Hash> result;

		{
			QMutexLocker locker(&m_recentUsedMutex);
			m_recentUsed.removeOutdated();

			result = m_recentUsed.hashes();
		}

		std::erase_if(result,
					  [&dataServiceId, this](Hash hash)
					  {
						  return !this->dataServiceHasSignal(::calcHash(dataServiceId), hash);
					  });

		return result;
	}

	bool TuningSignalManager::hasRecentlyUsedAppSignals()
	{
		QMutexLocker locker(&m_recentUsedMutex);
		return m_recentUsed.hashes().empty() == false;
	}

	bool TuningSignalManager::dataServiceHasSignal(Hash dataServiceHash, Hash signalHash) const
	{
		auto it = m_states.find(signalHash);
		if (it == m_states.end())
		{
			return false;
		}

		const Sources& srcs = it->second;
		for (const SourceState& src : srcs.sources)
		{
			if (src.tuningServiceHash == dataServiceHash)
			{
				return true;
			}
		}
		return false;
	}

	void TuningSignalManager::setUnappliedValue(Hash hash, const TuningValue& value)
	{
		std::scoped_lock l(m_statesMutex);

		auto foundState = m_states.find(hash);
		if (foundState != m_states.end())
		{
			Sources& sources = foundState->second;
			sources.setUnappliedValue(value);

			if (sources.isValueUnapplied() == true)
			{
				// qDebug() << "+Unapplied: " << hash;
				m_unappliedStates.insert(hash);
			}
			else
			{
				if (m_unappliedStates.contains(hash))
				{
					// qDebug() << "Undo Unapplied: " << hash;
					m_unappliedStates.erase(hash);
				}
			}
		}
	}

	TuningValue TuningSignalManager::unappliedValue(Hash hash) const
	{
		std::scoped_lock l(m_statesMutex);

		auto foundState = m_states.find(hash);

		if (foundState != m_states.end())
		{
			const Sources& sources = foundState->second;
			return sources.getUnappliedValue();
		}

		static TuningValue empty;
		return empty;
	}

	bool TuningSignalManager::isUnapplied(Hash hash) const
	{
		std::scoped_lock l(m_statesMutex);

		auto foundState = m_states.find(hash);
		if (foundState != m_states.end())
		{
			const Sources& sources = foundState->second;
			if (sources.isValueUnapplied() == true)
			{
				return true;
			}
		}

		return false;
	}

	void TuningSignalManager::setClientEquipmentId(const QString& clientEquipmentId)
	{
		m_tuningClientHash = ::calcHash(clientEquipmentId);
	}

	void TuningSignalManager::Sources::set(const TuningSignalState& state, Hash tuningServiceHash)
	{
		SourceState* emptyState = nullptr;
		for (SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == tuningServiceHash)
			{
				sourceState.state = state;
				sourceState.lastUpdateTime = std::chrono::system_clock::now();
				return;
			}

			if (sourceState.tuningServiceHash == UNDEFINED_HASH)
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

		*emptyState = SourceState{state, tuningServiceHash, std::chrono::system_clock::now(), false /*isUnapplied*/};

		return;
	}

	void TuningSignalManager::Sources::invalidateSource(Hash tuningServiceHash)
	{
		for (SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == tuningServiceHash)
			{
				sourceState.state = TuningSignalState{};
				sourceState.lastUpdateTime = std::chrono::system_clock::now();
				break;
			}
		}

		return;
	}

	const TuningSignalState& TuningSignalManager::Sources::get() const
	{
		return get(ANY_HASH, nullptr);
	}

	const TuningSignalState& TuningSignalManager::Sources::get(Hash tuningServiceHash, bool* found) const
	{
		if (found != nullptr)
		{
			*found = false;
		}

		// Find the newest available state
		//
		const SourceState* stateAvailable = nullptr;
		const SourceState* stateNewest = nullptr;

		for (const SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == UNDEFINED_HASH)
			{
				continue;
			}

			if (tuningServiceHash != ANY_HASH && tuningServiceHash != sourceState.tuningServiceHash)
			{
				continue;
			}

			if (found != nullptr)
			{
				*found = true;
			}

			if (sourceState.state.valid() == true)
			{
				if (stateAvailable == nullptr || stateAvailable->state.m_successfulReadTime < sourceState.state.m_successfulReadTime)
				{
					stateAvailable = &sourceState; // the first state with state available flag
				}
			}
			else
			{
				// sourceState.state.valid() == false
				//
				if (stateNewest == nullptr || stateNewest->lastUpdateTime < sourceState.lastUpdateTime)
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

		static const TuningSignalState NotValidState{};
		return NotValidState;
	}

	void TuningSignalManager::Sources::setUnappliedValue(const TuningValue& value)
	{
		// Unapplied value is set to all sources
		//
		unappliedValue = value;

		for (SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == UNDEFINED_HASH)
			{
				continue;
			}

			if (sourceState.state.valid() == true)
			{
				if (sourceState.state.value() == value)
				{
					sourceState.isUnapplied = false;
				}
				else
				{
					sourceState.isUnapplied = true;
				}
			}
			else
			{
				sourceState.isUnapplied = false;
			}
		}
	}

	const TuningValue& TuningSignalManager::Sources::getUnappliedValue() const
	{
		return unappliedValue;
	}

	bool TuningSignalManager::Sources::isValueUnapplied() const
	{
		for (const SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == UNDEFINED_HASH)
			{
				continue;
			}

			if (sourceState.state.valid() == true)
			{
				if (sourceState.isUnapplied == true)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool TuningSignalManager::Sources::isValueUnapplied(Hash tuningServiceHash) const
	{
		for (const SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == tuningServiceHash)
			{
				return sourceState.isUnapplied;
			}
		}
		return false;
	}

	void TuningSignalManager::Sources::setAsApplied(Hash tuningServiceHash)
	{
		for (SourceState& sourceState : sources)
		{
			if (sourceState.tuningServiceHash == tuningServiceHash)
			{
				sourceState.isUnapplied = false;
			}
		}
	}
} // namespace ClientLib