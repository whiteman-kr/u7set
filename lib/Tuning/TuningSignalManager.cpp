#include "../lib/Tuning/TuningSignalManager.h"

//
//TuningSignalManager
//
TuningSignalManager::TuningSignalManager(QObject* parent) :
	QObject(parent)
{
}

TuningSignalManager::~TuningSignalManager()
{
}

void TuningSignalManager::reset()
{
	{
		QMutexLocker l(&m_signalsMutex);
		m_signals.clear();
		m_tagToAppSignals.clear();
	}

	{
		QMutexLocker l(&m_statesMutex);
		m_states.clear();
	}

	return;
}

bool TuningSignalManager::load(const QByteArray& data)
{
	reset();

	::Proto::AppSignalSet message;
	bool ok = message.ParseFromArray(data.constData(), data.size());

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
		QMutexLocker l(&m_signalsMutex);

		std::swap(loadedSignals, m_signals);
		std::swap(tagToAppSignals, m_tagToAppSignals);
	}

	emit signalsLoaded();

	return ok;
}

int TuningSignalManager::signalsCount() const
{
	QMutexLocker l(&m_signalsMutex);
	return static_cast<int>(m_signals.size());
}

std::vector<AppSignalParam> TuningSignalManager::signalList() const
{
	std::vector<AppSignalParam> result;
	result.reserve(m_signals.size());

	QMutexLocker l(&m_signalsMutex);

	for (auto p : m_signals)
	{
		result.emplace_back(p.second);
	}

	return result;
}

std::vector<Hash> TuningSignalManager::signalHashes() const
{
	std::vector<Hash> result;
	result.reserve(m_signals.size());

	QMutexLocker l(&m_signalsMutex);

	for (auto p : m_signals)
	{
		result.push_back(p.first);
	}

	return result;
}

std::vector<Hash> TuningSignalManager::signalHashes(const std::vector<Hash> lmEquipmentIdHashes) const
{
	std::vector<Hash> result;
	result.reserve(m_signals.size());

	QMutexLocker l(&m_signalsMutex);

	for (auto p : m_signals)
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

bool TuningSignalManager::signalExists(Hash hash) const
{
	QMutexLocker l(&m_signalsMutex);
	return m_signals.find(hash) != m_signals.end();
}

bool TuningSignalManager::signalExists(const QString& appSignalId) const
{
	Hash hash = ::calcHash(appSignalId);
	return TuningSignalManager::signalExists(hash);
}

AppSignalParam TuningSignalManager::signalParam(Hash hash, bool* found) const
{
	QMutexLocker l(&m_signalsMutex);

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

	QMutexLocker l(&m_signalsMutex);

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
	if (hash == 0)
	{
		assert(hash != 0);
		return TuningSignalState();
	}

	QMutexLocker l(&m_statesMutex);

	auto foundState = m_states.find(hash);

	if (found != nullptr)
	{
		*found = !(foundState == m_states.end());
	}

	if (foundState != m_states.end())
	{
		return foundState->second;
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

QStringList TuningSignalManager::signalIdsByTag(const QString& tag) const
{
	QMutexLocker rl(&m_signalsMutex);

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

void TuningSignalManager::invalidateStates()
{
	QMutexLocker l(&m_statesMutex);

	for (auto& p : m_states)
	{
		p.second.invalidate();
	}

	return;
}

void TuningSignalManager::setState(const QString& appSignalId, const TuningSignalState& state)
{
	Hash signalHash = ::calcHash(appSignalId);
	return setState(signalHash, state);
}

void TuningSignalManager::setState(Hash signalHash, const TuningSignalState& state)
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

void TuningSignalManager::setState(const std::vector<TuningSignalState>& states)
{
	QMutexLocker l(&m_statesMutex);

	for (const TuningSignalState& state : states)
	{
		m_states[state.hash()] = state;
	}

	return;
}

TuningValue TuningSignalManager::newValue(Hash signalHash) const
{
	QMutexLocker l(&m_newValuesMutex);

	auto it = m_newValues.find(signalHash);
	if (it == m_newValues.end())
	{
		return TuningValue();
	}

	return it->second.value;
}

void TuningSignalManager::setNewValue(Hash signalHash, const TuningValue& value)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	// Get the old value

	QMutexLocker ls(&m_statesMutex);

	auto foundState = m_states.find(signalHash);

	if (foundState == m_states.end())
	{
		assert(false);
		return;
	}

	TuningSignalState& state = foundState->second;

	ls.unlock();

	// Compare new value to old value and set unapplied flag

	QMutexLocker ln(&m_newValuesMutex);

	TuningNewValue tnv;
	tnv.value = value;

	if (state.valid() == true)
	{
		if (state.value() == value)
		{
			tnv.isUnapplied = false;
		}
		else
		{
			tnv.isUnapplied = true;
		}
	}

	m_newValues[signalHash] = tnv;

	return;
}

bool TuningSignalManager::newValueIsUnapplied(Hash signalHash) const
{
	QMutexLocker l(&m_newValuesMutex);

	auto it = m_newValues.find(signalHash);
	if (it == m_newValues.end())
	{
		return false;
	}

	return it->second.isUnapplied;
}

void TuningSignalManager::setNewValueAsApplied(Hash signalHash)
{
	QMutexLocker l(&m_newValuesMutex);
	m_newValues[signalHash].isUnapplied = false;
}
