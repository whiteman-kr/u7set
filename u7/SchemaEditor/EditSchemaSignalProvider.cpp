#include "EditSchemaSignalProvider.h"
#include "../../Builder/AppSignalSetProvider.h"

//
//
// EditSchemaSignalProvider - this calss is used to provide app signals for drawing schemas, showing and getting signal ids, description, preciosion, etc...
//
//

EditSchemaAppSignalProvider::EditSchemaAppSignalProvider(AppSignalSetProvider* signalSetProvider) :
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(signalSetProvider);
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

bool EditSchemaAppSignalProvider::signalExists(const QString& appSignalId) const
{
	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);
	return s != nullptr;
}

AppSignalParam EditSchemaAppSignalProvider::signalParam(Hash signalHash, bool* found) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(signalHash);
	Q_UNUSED(found);
	Q_ASSERT(false);
	return {};
}

AppSignalParam EditSchemaAppSignalProvider::signalParam(const QString& appSignalId, bool* found) const
{
	AppSignalParam result;

	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);

	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		result.load(*s);
	}

	return result;
}

AppSignalState EditSchemaAppSignalProvider::signalState(Hash signalHash, bool* found) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(signalHash);
	Q_UNUSED(found);
	Q_ASSERT(false);
	return {};
}

AppSignalState EditSchemaAppSignalProvider::signalState(const QString& appSignalId, bool* found) const
{
	AppSignalState result;
	result.m_hash = ::calcHash(appSignalId);

	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);
	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		result.m_flags.valid = 1;
		result.m_value = 0;

//		result.m_time.plant = TimeStamp{QDateTime::currentDateTime()};
//		result.m_time.local = result.m_time.plant;
//		result.m_time.system = TimeStamp{QDateTime::currentDateTimeUtc()};
	}

	return result;
}

void EditSchemaAppSignalProvider::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(appSignalHashes);
	Q_UNUSED(result);
	Q_UNUSED(found);
	Q_ASSERT(false);
	return;
}

void EditSchemaAppSignalProvider::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	if (found != nullptr)
	{
		*found = 0;
	}

	result->clear();
	result->reserve(appSignalIds.size());

	for (const QString& id : appSignalIds)
	{
		bool signalFound = false;

		result->emplace_back(this->signalState(id, &signalFound));

		if (signalFound && found != nullptr)
		{
			(*found)++;
		}
	}

	return;
}

QStringList EditSchemaAppSignalProvider::signalTags(Hash signalHash) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(signalHash);
	Q_ASSERT(false);
	return {};
}

QStringList EditSchemaAppSignalProvider::signalTags(const QString& appSignalId) const
{
	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);

	if (s != nullptr)
	{
		return s->tags();
	}

	return {};
}

bool EditSchemaAppSignalProvider::signalHasTag(Hash signalHash, const QString& tag) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(signalHash);
	Q_UNUSED(tag);
	Q_ASSERT(false);
	return false;
}

bool EditSchemaAppSignalProvider::signalHasTag(const QString& appSignalId, const QString& tag) const
{
	return signalTags(appSignalId).contains(tag, Qt::CaseInsensitive);
}

QString EditSchemaAppSignalProvider::equipmentToAppSiganlId(const QString& /*equipmentId*/) const
{
	Q_ASSERT(false);	// todo
	return {};
}

std::vector<std::shared_ptr<Comparator>> EditSchemaAppSignalProvider::setpointsByInputSignalId(const QString& appSignalId) const
{
	// No simulation of this function in edit schema mode
	//
	Q_UNUSED(appSignalId);
	return {};
}

EditSchemaTuningSignalProvider::EditSchemaTuningSignalProvider(AppSignalSetProvider* signalSetProvider) :
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(m_signalSetProvider);
}


bool EditSchemaTuningSignalProvider::signalExists(Hash hash) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(hash);
	Q_ASSERT(false);
	return false;
}

bool EditSchemaTuningSignalProvider::signalExists(const QString& appSignalId) const
{
	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);
	return s != nullptr;
}

AppSignalParam EditSchemaTuningSignalProvider::signalParam(Hash hash, bool* found) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(hash);
	Q_UNUSED(found);
	Q_ASSERT(false);
	return {};
}

AppSignalParam EditSchemaTuningSignalProvider::signalParam(const QString& appSignalId, bool* found) const
{
	AppSignalParam result;

	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);

	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		result.load(*s);
	}

	return result;
}

bool EditSchemaTuningSignalProvider::signalParam(Hash hash, AppSignalParam* result) const
{
	// Unlikely this function required for schema editing
	//
	Q_UNUSED(hash);
	Q_UNUSED(result);
	Q_ASSERT(false);
	return {};
}

bool EditSchemaTuningSignalProvider::signalParam(const QString& appSignalId, AppSignalParam* result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return false;
	}

	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);

	if (s != nullptr)
	{
		(*result).load(*s);
		return true;
	}

	return false;
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

	AppSignal* s = m_signalSetProvider->getSignalByStrID(appSignalId);
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
