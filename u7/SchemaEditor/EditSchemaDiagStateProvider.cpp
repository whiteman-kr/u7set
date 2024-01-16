#include "EditSchemaDiagStateProvider.h"
//#include "../AppSignalSetProvider.h"

//
//
// EditSchemaDiagStateProvider - this class is used to provide diag states for drawing schemas, showing and getting signal ids, description, preciosion, etc...
//
//
EditSchemaDiagStateProvider::EditSchemaDiagStateProvider(/*AppSignalSetProvider* signalSetProvider*/) /*:
	m_signalSetProvider(signalSetProvider)*/
{
	//Q_ASSERT(signalSetProvider);
}
//
//int EditSchemaAppSignalProvider::signalsCount() const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_ASSERT(false);
//	return 0;
//}
//
//std::vector<AppSignalParam> EditSchemaAppSignalProvider::signalList() const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_ASSERT(false);
//	return {};
//}
//
//bool EditSchemaAppSignalProvider::signalExists(Hash hash) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(hash);
//	Q_ASSERT(false);
//	return {};
//}
//
//bool EditSchemaAppSignalProvider::signalExists(const QString& appSignalId) const
//{
//	return m_signalSetProvider->signalExists(appSignalId);
//}
//
//bool EditSchemaAppSignalProvider::signalsExist(const QStringList& signalIds) const
//{
//	return std::all_of(signalIds.begin(), signalIds.end(), [this](const QString& appSignalId) {
//		return m_signalSetProvider->signalExists(appSignalId);
//	});
//}
//
//AppSignalParam EditSchemaAppSignalProvider::signalParam(Hash signalHash, bool* found) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(signalHash);
//	Q_UNUSED(found);
//	Q_ASSERT(false);
//	return {};
//}
//
//AppSignalParam EditSchemaAppSignalProvider::signalParam(const QString& appSignalId, bool* found) const
//{
//	AppSignalParam result;
//
//	AppSignal* s = m_signalSetProvider->getSignal(appSignalId);
//
//	if (found != nullptr)
//	{
//		*found = s != nullptr;
//	}
//
//	if (s != nullptr)
//	{
//		result.load(*s);
//	}
//
//	return result;
//}
//
//AppSignalState EditSchemaAppSignalProvider::signalState(Hash signalHash, bool* found) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(signalHash);
//	Q_UNUSED(found);
//	Q_ASSERT(false);
//	return {};
//}
//
//AppSignalState EditSchemaAppSignalProvider::signalState(const QString& appSignalId, bool* found) const
//{
//	AppSignalState result;
//	result.m_hash = ::calcHash(appSignalId);
//
//	AppSignal* s = m_signalSetProvider->getSignal(appSignalId);
//	if (found != nullptr)
//	{
//		*found = s != nullptr;
//	}
//
//	if (s != nullptr)
//	{
//		result.m_flags.valid = 1;
//		result.m_value = 0;
//	}
//
//	return result;
//}
//
//AppSignalState EditSchemaAppSignalProvider::signalState(Hash signalHash, Hash /*dataServerHash*/, bool* found) const
//{
//	return signalState(signalHash, found);
//}
//
//AppSignalState EditSchemaAppSignalProvider::signalState(const QString& appSignalId, const QString& /*dataServerId*/, bool* found) const
//{
//	return signalState(appSignalId, found);
//}
//
//void EditSchemaAppSignalProvider::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(appSignalHashes);
//	Q_UNUSED(result);
//	Q_UNUSED(found);
//	Q_ASSERT(false);
//	return;
//}
//
//void EditSchemaAppSignalProvider::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
//{
//	if (result == nullptr)
//	{
//		Q_ASSERT(result);
//		return;
//	}
//
//	if (found != nullptr)
//	{
//		*found = 0;
//	}
//
//	result->clear();
//	result->reserve(appSignalIds.size());
//
//	for (const QString& id : appSignalIds)
//	{
//		bool signalFound = false;
//
//		result->emplace_back(this->signalState(id, &signalFound));
//
//		if (signalFound && found != nullptr)
//		{
//			(*found)++;
//		}
//	}
//
//	return;
//}
//
//void EditSchemaAppSignalProvider::signalState(const std::vector<Hash>& appSignalHashes, Hash /*dataServerHash*/, std::vector<AppSignalState>* result, int* found) const
//{
//	return signalState(appSignalHashes, result, found);
//}
//
//void EditSchemaAppSignalProvider::signalState(const std::vector<QString>& appSignalIds, const QString& /*dataServerId*/, std::vector<AppSignalState>* result, int* found) const
//{
//	return signalState(appSignalIds, result, found);
//}
//
//QStringList EditSchemaAppSignalProvider::signalTags(Hash signalHash) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(signalHash);
//	Q_ASSERT(false);
//	return {};
//}
//
//QStringList EditSchemaAppSignalProvider::signalTags(const QString& appSignalId) const
//{
//	AppSignal* s = m_signalSetProvider->getSignal(appSignalId);
//
//	if (s != nullptr)
//	{
//		return s->tags();
//	}
//
//	return {};
//}
//
//bool EditSchemaAppSignalProvider::signalHasTag(Hash signalHash, const QString& tag) const
//{
//	// Unlikely this function required for schema editing
//	//
//	Q_UNUSED(signalHash);
//	Q_UNUSED(tag);
//	Q_ASSERT(false);
//	return false;
//}
//
//bool EditSchemaAppSignalProvider::signalHasTag(const QString& appSignalId, const QString& tag) const
//{
//	return signalTags(appSignalId).contains(tag, Qt::CaseInsensitive);
//}
//
//E::SignalType EditSchemaAppSignalProvider::signalType(Hash signalHash, bool* found) const
//{
//	Q_UNUSED(signalHash);
//	Q_UNUSED(found);
//	Q_ASSERT(false);	// to do
//	return E::SignalType::Analog;
//}
//
//QStringList EditSchemaAppSignalProvider::signalIdsByTag(const QString& /*tag*/) const
//{
//	// No simulation of this function in edit schema mode
//	//
//	Q_ASSERT(false);
//	return {};
//}
//
//E::SignalType EditSchemaAppSignalProvider::signalType(const QString& appSignalId, bool* found) const
//{
//	return signalType(::calcHash(appSignalId), found);
//}
//
//QString EditSchemaAppSignalProvider::equipmentToAppSignalId(const QString& /*equipmentId*/) const
//{
//	Q_ASSERT(false);	// todo
//	return {};
//}
//
//std::vector<std::shared_ptr<Comparator>> EditSchemaAppSignalProvider::setpointsByInputSignalId(const QString& appSignalId) const
//{
//	// No simulation of this function in edit schema mode
//	//
//	Q_UNUSED(appSignalId);
//	return {};
//}
//
//QStringList EditSchemaAppSignalProvider::tags() const
//{
//	// No simulation of this function in edit schema mode
//	//
//	return {};
//}
