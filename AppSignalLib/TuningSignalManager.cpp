#ifndef APP_SIGNAL_LIB_DOMAIN
#error Don't include this file in the project! Link AppSignalLib instead.
#endif

#include "TuningSignalManager.h"

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
		QWriteLocker l(&m_signalsLock);
		m_signals.clear();
		m_tagToAppSignals.clear();
	}

	{
		QWriteLocker l(&m_statesLock);
		m_states.clear();
	}

	return;
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

	emit signalsLoaded();

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
	if (hash == 0)
	{
		assert(hash != 0);
		return TuningSignalState();
	}

	QReadLocker l(&m_statesLock);

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

void TuningSignalManager::invalidateStates()
{
	QWriteLocker l(&m_statesLock);

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

	QWriteLocker l(&m_statesLock);

	m_states[signalHash] = state;

	return;
}

void TuningSignalManager::setState(const std::vector<TuningSignalState>& states)
{
	QWriteLocker l(&m_statesLock);

	for (const TuningSignalState& state : states)
	{
		m_states[state.hash()] = state;
	}

	return;
}

TuningValue TuningSignalManager::newValue(Hash signalHash) const
{
	QReadLocker l(&m_newValuesLock);

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

	QReadLocker ls(&m_statesLock);

	auto foundState = m_states.find(signalHash);

	if (foundState == m_states.end())
	{
		assert(false);
		return;
	}

	const TuningSignalState state = foundState->second;

	ls.unlock();

	// Compare new value to old value and set unapplied flag

	QWriteLocker l(&m_newValuesLock);

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
	QReadLocker l(&m_newValuesLock);

	auto it = m_newValues.find(signalHash);
	if (it == m_newValues.end())
	{
		return false;
	}

	return it->second.isUnapplied;
}

void TuningSignalManager::setNewValueAsApplied(Hash signalHash)
{
	QWriteLocker l(&m_newValuesLock);
	m_newValues[signalHash].isUnapplied = false;
}
