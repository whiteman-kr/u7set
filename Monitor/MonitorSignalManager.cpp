#include "MonitorSignalManager.h"
#include "MonitorConfigController.h"


void Sources::set(const AppSignalState& state, Qt::HANDLE sourceThreadId)
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

		if (sourceState.sourceThreadId == 0)
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

	*emptyState = SourceState{state, sourceThreadId, std::chrono::system_clock::now()};

	return;
}

void Sources::invalidateSource(Qt::HANDLE sourceThreadId)
{
	for (SourceState& sourceState : sources)
	{
		if (sourceState.sourceThreadId == sourceThreadId)
		{
			sourceState.state = AppSignalState{};
			sourceState.lastUpdateTime = std::chrono::system_clock::now();
			return;
		}
	}

	return;
}

const AppSignalState& Sources::get() const
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


MonitorSignalManager::MonitorSignalManager(MonitorConfigController& configController, ILogFile* logFile, QObject* parent) :
	QObject(parent),
	m_configController(configController),
	m_logFile(logFile, "SignalManager")
{
	{
		QWriteLocker wl(&m_paramsLocker);
		m_signalParams.reserve(128000);
		m_signalParamByEquipmentId.reserve(128000);
	}

	{
		QWriteLocker wl(&m_statesLocker);
		m_states.reserve(64000);
	}

	connect(&configController, &MonitorConfigController::configurationUpdated, this, &MonitorSignalManager::configurationUpdated);

	return;
}

void MonitorSignalManager::reset()
{
	{
		QWriteLocker wl(&m_paramsLocker);
		m_signalParams.clear();
		m_signalParamByEquipmentId.clear();
		m_tagToAppSignals.clear();
		m_tags.clear();
		m_appDataServiceToSignalLis.clear();
	}

	{
		QWriteLocker wl(&m_statesLocker);
		m_states.clear();
	}

	m_setpoints.clear();	// m_setpoints is threadself itslef

	// --
	//
	emitSignalParamsUpdated();

	return;
}

void MonitorSignalManager::addSignal(const AppSignalParam& appSignal, QString appDataServiceId)
{
	QWriteLocker wl(&m_paramsLocker);

	addSignalPrivate(appSignal, appDataServiceId);

	return;
}

void MonitorSignalManager::addSignals(const std::vector<AppSignalParam>& appSignals, QString appDataServiceId)
{
	QWriteLocker wl(&m_paramsLocker);

	for (const AppSignalParam& s : appSignals)
	{
		addSignalPrivate(s, appDataServiceId);
	}

	return;
}

void MonitorSignalManager::addSignalPrivate(const AppSignalParam& appSignal, QString appDataServiceId)
{
	m_signalParams[appSignal.hash()] = appSignal;

	// Actually, EquipmentID does not starts from the symbol '@',
	// but we need it particularly for Monitor to distinct AppSignalID from EquimpentID.
	//
	m_signalParamByEquipmentId[QStringLiteral("@") + appSignal.equipmentId()] = appSignal.appSignalId();

	// --
	//
	m_appDataServiceToSignalLis[appDataServiceId].insert(appSignal.appSignalId());

	// Add tags to m_signaIdsByTag
	//
	const QString& appSignalId = appSignal.appSignalId();
	const std::set<QString>& tags = appSignal.tags();

	for (const QString& tag : tags)
	{
		QStringList& l = m_tagToAppSignals[tag];

		if (l.isEmpty() == true)
		{
			l.reserve(1024);
		}

		l.push_back(appSignalId);
	}

	// Add tags to commot tag set
	//
	m_tags.insert(tags.begin(), tags.end());

	return;
}

std::vector<Hash> MonitorSignalManager::signalHashes() const
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

void MonitorSignalManager::invalidateSignalStates(Qt::HANDLE sourceThreadId)
{
	QWriteLocker wl(&m_statesLocker);

	for (auto&[signalHash, source] : m_states)
	{
		source.invalidateSource(sourceThreadId);
	}

	return;
}

void MonitorSignalManager::setState(const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId)
{
	Hash signalHash = ::calcHash(appSignalId);
	return setState(signalHash, state, sourceThreadId);
}

void MonitorSignalManager::setState(Hash signalHash, const AppSignalState& arrivedState, Qt::HANDLE sourceThreadId)
{
	if (signalHash == 0)
	{
		Q_ASSERT(signalHash != 0);
		return;
	}

	QWriteLocker wl(&m_statesLocker);

	Sources& currentState = m_states[signalHash];
	currentState.set(arrivedState, sourceThreadId);

	return;
}

void MonitorSignalManager::setState(const std::vector<AppSignalState>& states, Qt::HANDLE sourceThreadId)
{
	QWriteLocker wl(&m_statesLocker);

	for (const AppSignalState& newState : states)
	{
		Sources& currentStateAndSources = m_states[newState.hash()];
		currentStateAndSources.set(newState, sourceThreadId);
	}

	return;
}

void MonitorSignalManager::setSetpoints(ComparatorSet&& setpoints)
{
	m_setpoints = std::move(setpoints);
	return;
}

void MonitorSignalManager::setSetpoints(const ComparatorSet& setpoints)
{
	m_setpoints = setpoints;
	return;
}

int MonitorSignalManager::signalsCount() const
{
	QReadLocker rl(&m_paramsLocker);
	return static_cast<int>(m_signalParams.size());
}

std::vector<AppSignalParam> MonitorSignalManager::signalList() const
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

bool MonitorSignalManager::signalExists(Hash hash) const
{
	QReadLocker rl(&m_paramsLocker);

	auto result = m_signalParams.find(hash);
	return result != m_signalParams.end();
}

bool MonitorSignalManager::signalExists(const QString& appSignalId) const
{
	Hash signalHash = ::calcHash(appSignalId);
	return signalExists(signalHash);
}

AppSignalParam MonitorSignalManager::signalParam(Hash signalHash, bool* found) const
{
	QReadLocker rl(&m_paramsLocker);

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

AppSignalParam MonitorSignalManager::signalParam(const QString& appSignalId, bool* found) const
{
	Hash signalHash = ::calcHash(appSignalId);
	return signalParam(signalHash, found);
}

AppSignalState MonitorSignalManager::signalState(Hash signalHash, bool* found) const
{
	if (signalHash == 0)
	{
		return AppSignalState();
	}

	emit addSignalToPriorityList(signalHash);

	QReadLocker rl(&m_statesLocker);

	auto foundState = m_states.find(signalHash);

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
		AppSignalState result;
		result.m_flags.valid = false;

		return result;
	}
}

AppSignalState MonitorSignalManager::signalState(const QString& appSignalId, bool* found) const
{
	Hash h = ::calcHash(appSignalId);
	return signalState(h, found);
}

void MonitorSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
{
	if (result == nullptr)
	{
		assert(result);
		return;
	}

	result->clear();
	result->reserve(appSignalHashes.size());

	emit addSignalsToPriorityList(QVector<Hash>{appSignalHashes.begin(), appSignalHashes.end()});

	int foundCount = 0;

	{
		QReadLocker rl(&m_statesLocker);

		for (Hash signalHash : appSignalHashes)
		{
			auto foundState = m_states.find(signalHash);

			if (foundState != m_states.end())
			{
				result->push_back(foundState->second.get());
				foundCount ++;
			}
			else
			{
				AppSignalState state;				// Non valid state, hash will be 0 or something like UNDEFINED
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

void MonitorSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
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

QStringList MonitorSignalManager::signalTags(Hash signalHash) const
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

QStringList MonitorSignalManager::signalTags(const QString& appSignalId) const
{
	return signalTags(::calcHash(appSignalId));
}

bool MonitorSignalManager::signalHasTag(Hash signalHash, const QString& tag) const
{
	QReadLocker rl(&m_paramsLocker);

	auto result = m_signalParams.find(signalHash);
	return result == m_signalParams.end() ? false : result->second.hasTag(tag);
}

bool MonitorSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
{
	return signalHasTag(::calcHash(appSignalId), tag);
}

E::SignalType MonitorSignalManager::signalType(Hash signalHash, bool* found) const
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

QStringList MonitorSignalManager::signalIdsByTag(const QString& tag) const
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

E::SignalType MonitorSignalManager::signalType(const QString& appSignalId, bool* found) const
{
	return signalType(::calcHash(appSignalId), found);
}

QString MonitorSignalManager::equipmentToAppSiganlId(const QString& equipmentId) const
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


std::vector<std::shared_ptr<Comparator>> MonitorSignalManager::setpointsByInputSignalId(const QString& appSignalId) const
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
QStringList MonitorSignalManager::dataServiceIds(const QString& appSignalId) const
{
	QStringList result;
	for (const auto& [appDataServcieId, signalSet] : m_appDataServiceToSignalLis)
	{
		if (signalSet.contains(appSignalId) == true)
		{
			result.push_back(appDataServcieId);
		}
	}

	return result;
}

/// Return true if AppDataService contains signal.
///
bool MonitorSignalManager::dataServiceHasSignal(const QString& serviceEquipmentId, const QString& appSignalId) const
{
	QReadLocker rl(&m_paramsLocker);

	auto it = m_appDataServiceToSignalLis.find(serviceEquipmentId);
	if (it == m_appDataServiceToSignalLis.end())
	{
		return false;
	}

	return it->second.contains(appSignalId);
}

QStringList MonitorSignalManager::tags() const
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


AppSignalParam MonitorSignalManager::signalParamByEquipemntId(const QString& equipmentId, bool* found) const
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

void MonitorSignalManager::emitSignalParamsUpdated()
{
	emit signalParamsUpdated();
	return;
}

void MonitorSignalManager::configurationUpdated()
{
	m_setpoints = m_configController.setpoints();
}

