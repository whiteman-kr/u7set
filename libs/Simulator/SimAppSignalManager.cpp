#include <Simulator/SimAppSignalManager.h>
#include "SimAppSignalManagerImpl.h"

namespace Sim
{
	AppSignalManager::AppSignalManager(AppSignalManagerImpl& impl) :
		m_impl{impl}
	{
	}

	AppSignalManager::~AppSignalManager() = default;

	QString AppSignalManager::ramDump(QString logicModuleId) const
	{
		return m_impl.ramDump(logicModuleId);
	}

	void AppSignalManager::resetAll()
	{
		m_impl.resetAll();
	}

	void AppSignalManager::resetSignalParam()
	{
		m_impl.resetSignalParam();
	}

	void AppSignalManager::resetRam()
	{
		m_impl.resetRam();
	}

	std::shared_ptr<TrendLib::RealtimeData> AppSignalManager::trendData(const QString& trendId,
																		const std::vector<Hash>& trendSignals,
																		TrendLib::TrendStateItem* minState,
																		TrendLib::TrendStateItem* maxState)
	{
		return m_impl.trendData(trendId, trendSignals, minState, maxState);
	}

	std::optional<AppSignal> AppSignalManager::signalParamExt(const QString& appSignalId) const
	{
		return m_impl.signalParamExt(appSignalId);
	}
	std::optional<AppSignal> AppSignalManager::signalParamExt(Hash hash) const
	{
		return m_impl.signalParamExt(hash);
	}

	Hash AppSignalManager::customToAppSignal(Hash customSignalHash) const
	{
		return m_impl.customToAppSignal(customSignalHash);
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found, bool applyOverride) const
	{
		return m_impl.signalState(appSignalId, found, applyOverride);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found, bool applyOverride) const
	{
		return m_impl.signalState(signalHash, found, applyOverride);
	}

	bool AppSignalManager::getUpdateForRam(const QString& equipmentId, Sim::Ram* ram) const
	{
		return m_impl.getUpdateForRam(equipmentId, ram);
	}

	int AppSignalManager::signalsCount() const
	{
		return m_impl.signalsCount();
	}

	std::vector<AppSignalParam> AppSignalManager::signalList() const
	{
		return m_impl.signalList();
	}

	bool AppSignalManager::signalExists(Hash hash) const
	{
		return m_impl.signalExists(hash);
	}

	bool AppSignalManager::signalExists(const QString& appSignalId) const
	{
		return m_impl.signalExists(appSignalId);
	}

	bool AppSignalManager::signalsExist(const QStringList& signalIds) const
	{
		return m_impl.signalsExist(signalIds);
	}

	AppSignalParam AppSignalManager::signalParam(Hash signalHash, bool* found) const
	{
		return m_impl.signalParam(signalHash, found);
	}

	AppSignalParam AppSignalManager::signalParam(const QString& appSignalId, bool* found) const
	{
		return m_impl.signalParam(appSignalId, found);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, bool* found) const
	{
		return m_impl.signalState(signalHash, found);
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, bool* found) const
	{
		return m_impl.signalState(appSignalId, found);
	}

	AppSignalState AppSignalManager::signalState(Hash signalHash, Hash dataServerHash, bool* found) const
	{
		return m_impl.signalState(signalHash, dataServerHash, found);
	}

	AppSignalState AppSignalManager::signalState(const QString& appSignalId, const QString& dataServerId, bool* found) const
	{
		return m_impl.signalState(appSignalId, dataServerId, found);
	}

	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		m_impl.signalState(appSignalHashes, result, found);
	}

	void AppSignalManager::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	{
		m_impl.signalState(appSignalIds, result, found);
	}

	void AppSignalManager::signalState(const std::vector<Hash>& appSignalHashes,
									   Hash dataServerHash,
									   std::vector<AppSignalState>* result,
									   int* found) const
	{
		m_impl.signalState(appSignalHashes, dataServerHash, result, found);
	}

	void AppSignalManager::signalState(const std::vector<QString>& appSignalIds,
									   const QString& dataServerId,
									   std::vector<AppSignalState>* result,
									   int* found) const
	{
		m_impl.signalState(appSignalIds, dataServerId, result, found);
	}

	QStringList AppSignalManager::signalTags(Hash signalHash) const
	{
		return m_impl.signalTags(signalHash);
	}

	QStringList AppSignalManager::signalTags(const QString& appSignalId) const
	{
		return m_impl.signalTags(appSignalId);
	}

	bool AppSignalManager::signalHasTag(Hash signalHash, const QString& tag) const
	{
		return m_impl.signalHasTag(signalHash, tag);
	}

	bool AppSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
	{
		return m_impl.signalHasTag(appSignalId, tag);
	}

	QStringList AppSignalManager::signalIdsByTag(const QString& tag) const
	{
		return m_impl.signalIdsByTag(tag);
	}

	E::SignalType AppSignalManager::signalType(Hash signalHash, bool* found) const
	{
		return m_impl.signalType(signalHash, found);
	}

	E::SignalType AppSignalManager::signalType(const QString& appSignalId, bool* found) const
	{
		return m_impl.signalType(appSignalId, found);
	}

	QString AppSignalManager::equipmentToAppSignalId(const QString& equipmentId) const
	{
		return m_impl.equipmentToAppSignalId(equipmentId);
	}

	std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInputSignalId(const QString& appSignalId) const
	{
		return m_impl.setpointsByInputSignalId(appSignalId);
	}

	QStringList AppSignalManager::tags() const
	{
		return m_impl.tags();
	}

} // namespace Sim