#include "EditSchemaSignalProvider.h"
#include "../AppSignalSetProvider.h"

//
// EditSchemaSignalProvider - this class is used to provide app signals for drawing schemas, showing and getting signal ids, description,
// preciosion, etc...
//

EditSchemaAppSignalProvider::EditSchemaAppSignalProvider(AppSignalSetProvider* signalSetProvider) :
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(signalSetProvider);
}

int EditSchemaAppSignalProvider::signalsCount() const
{
	// Unlikely this function required for schema editing
	//
	Q_ASSERT(false);
	return 0;
}

std::vector<Hash> EditSchemaAppSignalProvider::signalHashes() const
{
	// Unlikely this function required for schema editing
	//
	Q_ASSERT(false);
	return {};
}

std::vector<AppSignalParam> EditSchemaAppSignalProvider::signalList() const
{
	// Unlikely this function required for schema editing
	//
	Q_ASSERT(false);
	return {};
}

bool EditSchemaAppSignalProvider::signalExists(Hash hash) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(hash);
	Q_ASSERT(false);
	return {};
}

bool EditSchemaAppSignalProvider::signalsExist(const QStringList& signalIds) const
{
	return std::all_of(signalIds.begin(),
					   signalIds.end(),
					   [this](const QString& appSignalId)
					   {
						   return m_signalSetProvider->signalExists(appSignalId);
					   });
}

std::optional<AppSignalParam> EditSchemaAppSignalProvider::signalParam(Hash signalHash) const
{
	const AppSignal* s = m_signalSetProvider->getSignalByHash(signalHash);
	if (s == nullptr)
	{
		return std::nullopt;
	}

	return AppSignalParam{*s};
}

std::optional<AppSignalState> EditSchemaAppSignalProvider::signalState(Hash signalHash) const
{
	AppSignal* s = m_signalSetProvider->getSignalByHash(signalHash);
	if (s == nullptr)
	{
		return std::nullopt;
	}

	std::optional<AppSignalState> result;
	result->m_hash = signalHash;
	result->m_flags.valid = 1;

	return result;
}

std::optional<AppSignalState> EditSchemaAppSignalProvider::signalState(Hash signalHash, Hash /*dataServerHash*/) const
{
	return signalState(signalHash);
}

void EditSchemaAppSignalProvider::signalState(std::span<const Hash> appSignalHashes,
											  std::vector<std::optional<AppSignalState>>* result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	result->clear();
	result->reserve(appSignalHashes.size());

	for (const Hash& hash : appSignalHashes)
	{
		result->emplace_back(signalState(hash));
	}

	return;
}

void EditSchemaAppSignalProvider::signalState(std::span<const Hash> appSignalHashes,
											  Hash /*dataServerHash*/,
											  std::vector<std::optional<AppSignalState>>* result) const
{
	return signalState(appSignalHashes, result);
}

QStringList EditSchemaAppSignalProvider::signalTags(Hash signalHash) const
{
	AppSignal* s = m_signalSetProvider->getSignalByHash(signalHash);

	if (s != nullptr)
	{
		return s->tags();
	}

	return {};
}

bool EditSchemaAppSignalProvider::signalHasTag(Hash signalHash, const QString& tag) const
{
	return signalTags(signalHash).contains(tag, Qt::CaseInsensitive);
}

QStringList EditSchemaAppSignalProvider::signalIdsByTag(const QString& /*tag*/) const
{
	// No simulation of this function in edit schema mode
	//
	Q_ASSERT(false);
	return {};
}

E::SignalType EditSchemaAppSignalProvider::signalType(Hash signalHash, bool* found) const
{
	AppSignal* s = m_signalSetProvider->getSignalByHash(signalHash);

	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		return s->signalType();
	}

	return {};
}

QString EditSchemaAppSignalProvider::equipmentToAppSignalId(const QString& /*equipmentId*/) const
{
	return {};
}

std::vector<std::shared_ptr<Comparator>> EditSchemaAppSignalProvider::setpointsByInput(const QString& appSignalId) const
{
	// No simulation of this function in edit schema mode
	//
	Q_UNUSED(appSignalId);
	return {};
}

std::shared_ptr<Comparator> EditSchemaAppSignalProvider::setpointByOutput(const QString& appSignalId) const
{
	// No simulation of this function in edit schema mode
	//
	Q_UNUSED(appSignalId);
	return {};
}

QStringList EditSchemaAppSignalProvider::tags() const
{
	// No simulation of this function in edit schema mode
	//
	return {};
}

EditSchemaTuningSignalProvider::EditSchemaTuningSignalProvider(AppSignalSetProvider* signalSetProvider) :
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(m_signalSetProvider);
}


bool EditSchemaTuningSignalProvider::signalExists(Hash hash) const
{
	AppSignal* s = m_signalSetProvider->getSignalByHash(hash);
	return s != nullptr;
}

bool EditSchemaTuningSignalProvider::signalsExist(const QStringList& signalIds) const
{
	return std::all_of(signalIds.begin(),
					   signalIds.end(),
					   [this](const QString& appSignalId)
					   {
						   return m_signalSetProvider->getSignal(appSignalId) != nullptr;
					   });
}

std::optional<AppSignalParam> EditSchemaTuningSignalProvider::signalParam(Hash hash) const
{
	AppSignal* s = m_signalSetProvider->getSignalByHash(hash);
	if (s == nullptr)
	{
		return std::nullopt;
	}

	return AppSignalParam(*s);
}

int EditSchemaTuningSignalProvider::signalsCount() const
{
	// Unlikely this function required for schema editing
	//
	return 0;
}

std::vector<Hash> EditSchemaTuningSignalProvider::signalHashes() const
{
	// Unlikely this function required for schema editing
	//
	return {};
}

std::vector<AppSignalParam> EditSchemaTuningSignalProvider::signalList() const
{
	// Unlikely this function required for schema editing
	//
	return {};
}

TuningSignalState EditSchemaTuningSignalProvider::state(Hash hash, bool* found) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(hash);
	Q_UNUSED(found);
	Q_ASSERT(false);
	return {};
}

TuningSignalState EditSchemaTuningSignalProvider::state(const QString& appSignalId, bool* found) const
{
	TuningSignalState result;
	result.m_hash = ::calcHash(appSignalId);

	AppSignal* s = m_signalSetProvider->getSignal(appSignalId);
	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		if (s->isAnalog() == true)
		{
			switch (s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				result.m_value.setType(TuningValueType::Float);
				break;
			case E::AnalogAppSignalFormat::SignedInt32:
				result.m_value.setType(TuningValueType::SignedInt32);
				break;
			}
		}

		if (s->isDiscrete() == true)
		{
			result.m_value.setType(TuningValueType::Discrete);
		}

		result.m_flags.valid = 1;
	}

	return result;
}

TuningSignalState EditSchemaTuningSignalProvider::state(Hash /*hash*/, Hash /*tuningServiceHash*/, bool* /*found*/) const
{
	Q_ASSERT(false);
	return {};
}

TuningSignalState EditSchemaTuningSignalProvider::state(const QString& /*appSignalId*/, Hash /*tuningServiceHash*/, bool* /*found*/) const
{
	Q_ASSERT(false);
	return {};
}

void EditSchemaTuningSignalProvider::state(std::span<const Hash> /*appSignalHashes*/,
										   std::vector<TuningSignalState>* /*result*/,
										   int* /*found*/) const
{
	Q_ASSERT(false);
	return;
}

void EditSchemaTuningSignalProvider::state(std::span<const QString> /*appSignalIds*/,
										   std::vector<TuningSignalState>* /*result*/,
										   int* /*found*/) const
{
	Q_ASSERT(false);
	return;
}


QStringList EditSchemaTuningSignalProvider::signalIdsByTag(const QString& /*tag*/) const
{
	Q_ASSERT(false);
	return {};
}
