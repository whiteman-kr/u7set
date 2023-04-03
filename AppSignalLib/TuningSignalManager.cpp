#ifndef APP_SIGNAL_LIB_DOMAIN
#error Don't include this file in the project! Link AppSignalLib instead.
#endif

#include "TuningSignalManager.h"

#define ANY_HASH UNDEFINED_HASH

//
//TuningSignalManager
//
TuningSignalManager::TuningSignalManager(const QString& clientEquipmentId, ILogFile* logFile, QObject* parent) :
	QObject(parent),
	m_tuningClientHash(::calcHash(clientEquipmentId)),
	m_logFile(logFile, "TuningSignalManager")
{
}

TuningSignalManager::~TuningSignalManager()
{
}

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

	std::unordered_map<Hash, AppSignalParam> loadedSignals;
	loadedSignals.reserve(message.appsignal_size());

	std::unordered_map<QString, QStringList> tagToAppSignals;
	tagToAppSignals.reserve(256);

	for (int i = 0; i < message.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = message.appsignal(i);

		Hash hash = ::calcHash(QString::fromStdString(appSignalMessage.appsignalid()));

		AppSignalParam& appSignalParam = loadedSignals[hash];
		ok &= appSignalParam.load(appSignalMessage);

		// Add tags to m_signaIdsByTag
		//
		const QString& appSignalId = appSignalParam.appSignalId();

		for (const std::set<QString>& tags = appSignalParam.tags();
			 const QString& tag : tags)
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


bool TuningSignalManager::signalParam(Hash hash, AppSignalParam* result) const
{
	if (result == nullptr)
	{
		assert(result);
		return false;
	}

	QReadLocker rl(&m_signalsLock);

	auto it = m_signals.find(hash);
	if (it == m_signals.end())
	{
		return false;
	}

	*result = it->second;

	return true;
}

bool TuningSignalManager::signalParam(const QString& appSignalId, AppSignalParam* result) const
{
	Hash signalHash = ::calcHash(appSignalId);
	return signalParam(signalHash, result);
}

TuningSignalState TuningSignalManager::state(Hash hash, bool* found) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return TuningSignalState();
	}

	QReadLocker l(&m_statesLocker);

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

	QReadLocker l(&m_statesLocker);

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
		QWriteLocker l(&m_statesLocker);
		m_states.clear();
	}

	return;
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
	QWriteLocker l(&m_statesLocker);

	for (auto& p : m_states)
	{
		p.second.invalidateSource(tuningServiceHash);
	}

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
		QWriteLocker l(&m_statesLocker);

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

						unsuccessfulWrites.push_back({currentSources.getUnappliedValue(), arrivedState.hash(), arrivedState.writeErrorCode()});
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
	for (const UnsuccessfulWrite&  u: unsuccessfulWrites)
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
							 .arg(E::valueToString(static_cast<E::NetworkError>(u.writeErrorCode)))
							 );
	}


	return;
}

void TuningSignalManager::notifySignalParamsUpdated()
{
	emit signalsLoaded();
}

void TuningSignalManager::setUnappliedValue(Hash hash, const TuningValue& value)
{
	QWriteLocker l(&m_statesLocker);

	auto foundState = m_states.find(hash);
	if (foundState != m_states.end())
	{
		Sources& sources = foundState->second;
		sources.setUnappliedValue(value);
	}
}

TuningValue TuningSignalManager::unappliedValue(Hash hash) const
{
	QWriteLocker l(&m_statesLocker);

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
	QWriteLocker l(&m_statesLocker);

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

	*emptyState = SourceState{state, tuningServiceHash, std::chrono::system_clock::now(), false/*isUnapplied*/};

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
			if (stateAvailable == nullptr ||
				stateAvailable->state.m_successfulReadTime < sourceState.state.m_successfulReadTime)
			{
				stateAvailable = &sourceState;	// the first state with state available flag
			}
		}
		else
		{
			// sourceState.state.valid() == false
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
