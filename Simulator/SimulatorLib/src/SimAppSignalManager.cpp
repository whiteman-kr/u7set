#include "SimAppSignalManagerImpl.h"
#include <SimulatorLib/SimAppSignalManager.h>

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

	std::optional<AppSignalState> AppSignalManager::signalState(const QString& appSignalId, bool applyOverride) const
	{
		return m_impl.signalState(appSignalId, applyOverride);
	}

	std::optional<AppSignalState> AppSignalManager::signalState(Hash signalHash, bool applyOverride) const
	{
		return m_impl.signalState(signalHash, applyOverride);
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

	std::vector<Hash> AppSignalManager::signalHashes() const
	{
		return m_impl.signalHashes();
	}

	bool AppSignalManager::signalExists(Hash hash) const
	{
		return m_impl.signalExists(hash);
	}

	bool AppSignalManager::signalsExist(const QStringList& signalIds) const
	{
		return m_impl.signalsExist(signalIds);
	}

	std::optional<AppSignalParam> AppSignalManager::signalParam(Hash signalHash) const
	{
		return m_impl.signalParam(signalHash);
	}

	std::optional<AppSignalState> AppSignalManager::signalState(Hash signalHash) const
	{
		return m_impl.signalState(signalHash);
	}

	std::optional<AppSignalState> AppSignalManager::signalState(Hash signalHash, Hash dataServerHash) const
	{
		return m_impl.signalState(signalHash, dataServerHash);
	}

	void AppSignalManager::signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const
	{
		m_impl.signalState(appSignalHashes, result);
	}

	void AppSignalManager::signalState(std::span<const Hash> appSignalHashes,
									   Hash dataServerHash,
									   std::vector<std::optional<AppSignalState>>* result) const
	{
		m_impl.signalState(appSignalHashes, dataServerHash, result);
	}

	QStringList AppSignalManager::signalTags(Hash signalHash) const
	{
		return m_impl.signalTags(signalHash);
	}

	bool AppSignalManager::signalHasTag(Hash signalHash, const QString& tag) const
	{
		return m_impl.signalHasTag(signalHash, tag);
	}

	QStringList AppSignalManager::signalIdsByTag(const QString& tag) const
	{
		return m_impl.signalIdsByTag(tag);
	}

	E::SignalType AppSignalManager::signalType(Hash signalHash, bool* found) const
	{
		return m_impl.signalType(signalHash, found);
	}

	QString AppSignalManager::equipmentToAppSignalId(const QString& equipmentId) const
	{
		return m_impl.equipmentToAppSignalId(equipmentId);
	}

	std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInput(const QString& appSignalId) const
	{
		return m_impl.setpointsByInput(appSignalId);
	}

	std::shared_ptr<Comparator> AppSignalManager::setpointByOutput(const QString& appSignalId) const
	{
		return m_impl.setpointByOutput(appSignalId);
	}

	QStringList AppSignalManager::tags() const
	{
		return m_impl.tags();
	}

} // namespace Sim