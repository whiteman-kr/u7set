#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include <ClientLib/AppSignalManager.h>

namespace ClientLib
{
	AppSignalManager::AppSignalManager(ILogFile* logFile, QObject* parent) :
		QObject(parent),
		m_logFile(logFile, "SignalManager")
	{
		m_core.reserve(64000);
		return;
	}

	void AppSignalManager::reset()
	{
		m_core.reset();
		m_setpoints.clear(); // m_setpoints is thread-safe

		notifySignalParamsUpdated();
		return;
	}

	void AppSignalManager::notifySignalParamsUpdated()
	{
		emit signalParamsUpdated();
		return;
	}

	void AppSignalManager::addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId)
	{
		return m_core.addSignals(appSignals,
								 appDataServiceId,
								 [](const auto& ps) -> AppSignalParam
								 {
									 AppSignalParam sp;
									 sp.load(ps);
									 return sp;
								 });
	}

	void AppSignalManager::invalidateSignalStates(SourceIdType sourceThreadId)
	{
		return m_core.invalidateSignalStates(sourceThreadId);
	}

	void AppSignalManager::setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId)
	{
		return m_core.setStates(states, dataServerHash, sourceThreadId);
	}

	void AppSignalManager::addRecentAppSignal(Hash hash)
	{
		QMutexLocker locker(&m_recentUsedMutex);
		m_recentUsed.add(hash);
	}

	void AppSignalManager::addRecentAppSignals(std::span<const Hash> hashes)
	{
		QMutexLocker locker(&m_recentUsedMutex);
		m_recentUsed.add(hashes);
	}

	std::vector<Hash> AppSignalManager::recentlyUsedAppSignals(const std::string& appDataServiceId)
	{
		std::vector<Hash> result;

		{
			QMutexLocker locker(&m_recentUsedMutex);
			m_recentUsed.removeOutdated();

			result = m_recentUsed.hashes();
		}

		filterByDataService(QString::fromStdString(appDataServiceId), result);

		return result;
	}

	bool AppSignalManager::hasRecentlyUsedAppSignals()
	{
		QMutexLocker locker(&m_recentUsedMutex);
		return m_recentUsed.hashes().empty() == false;
	}

	void AppSignalManager::setSetpoints(ComparatorSet&& setpoints)
	{
		m_setpoints = std::move(setpoints);
		return;
	}

	void AppSignalManager::setSetpoints(const ComparatorSet& setpoints)
	{
		m_setpoints = setpoints;
		return;
	}

	std::vector<Hash> AppSignalManager::signalHashes() const
	{
		return m_core.signalHashes();
	}

	int AppSignalManager::signalsCount() const
	{
		return m_core.signalsCount();
	}

	std::vector<AppSignalParam> AppSignalManager::signalList() const
	{
		return m_core.signalList();
	}

	bool AppSignalManager::signalExists(Hash hash) const
	{
		return m_core.signalExists(hash);
	}

	bool AppSignalManager::signalsExist(const QStringList& signalIds) const
	{
		return m_core.signalsExist(signalIds);
	}

	std::optional<AppSignalParam> AppSignalManager::signalParam(Hash signalHash) const
	{
		return m_core.signalParam(signalHash);
	}

	std::optional<AppSignalState> AppSignalManager::signalState(Hash signalHash) const
	{
		return signalState(signalHash, {});
	}

	std::optional<AppSignalState> AppSignalManager::signalState(Hash signalHash, Hash dataServerHash) const
	{
		const_cast<AppSignalManager*>(this)->addRecentAppSignal(signalHash);
		return m_core.signalState(signalHash, dataServerHash);
	}

	// Ok
	//
	void AppSignalManager::signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const
	{
		return signalState(appSignalHashes, {}, result);
	}

	void AppSignalManager::signalState(std::span<const Hash> appSignalHashes,
									   Hash dataServerHash,
									   std::vector<std::optional<AppSignalState>>* result) const
	{
		assert(result);
		const_cast<AppSignalManager*>(this)->addRecentAppSignals(appSignalHashes);

		return m_core.signalState(appSignalHashes, dataServerHash, result);
	}

	QStringList AppSignalManager::signalTags(Hash signalHash) const
	{
		return m_core.signalTags(signalHash);
	}

	bool AppSignalManager::signalHasTag(Hash signalHash, const QString& tag) const
	{
		return m_core.signalHasTag(signalHash, tag);
	}

	E::SignalType AppSignalManager::signalType(Hash signalHash, bool* found) const
	{
		return static_cast<E::SignalType>(m_core.signalType(signalHash, found));
	}

	QStringList AppSignalManager::signalIdsByTag(const QString& tag) const
	{
		return m_core.signalIdsByTag(tag);
	}

	QString AppSignalManager::equipmentToAppSignalId(const QString& equipmentId) const
	{
		return m_core.equipmentToAppSignalId(equipmentId);
	}

	std::vector<std::shared_ptr<Comparator>> AppSignalManager::setpointsByInput(const QString& appSignalId) const
	{
		return m_setpoints.getByInputSignalID(appSignalId);
	}

	std::shared_ptr<Comparator> AppSignalManager::setpointByOutput(const QString& appSignalId) const
	{
		return m_setpoints.getByOutputSignalID(appSignalId);
	}

	/// Get AppDataService EquipmentIDs list by AppSignalID.
	///
	std::vector<std::string> AppSignalManager::dataServiceIds(const std::string& appSignalId) const
	{
		return m_core.dataServiceIds(appSignalId);
	}

	/// Return true if AppDataService contains signal.
	///
	bool AppSignalManager::dataServiceHasSignal(const std::string& serviceEquipmentId, const std::string& appSignalId) const
	{
		Hash hash = calcHash(appSignalId);
		return dataServiceHasSignal(serviceEquipmentId, hash);
	}

	bool AppSignalManager::dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const
	{
		return m_core.dataServiceHasSignal(serviceEquipmentId, signalHash);
	}

	void AppSignalManager::filterByDataService(const QString& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const
	{
		return m_core.filterByDataService(serviceEquipmentId.toStdString(), inOutSignalHashes);
	}

	std::vector<Hash> AppSignalManager::dataServiceSignals(const std::string& serviceEquipmentId) const
	{
		return m_core.dataServiceSignals(serviceEquipmentId);
	}

	QStringList AppSignalManager::tags() const
	{
		return m_core.tags();
	}

	std::optional<AppSignalParam> AppSignalManager::signalParamByEquipmentId(const QString& equipmentId) const
	{
		return m_core.signalParamByEquipmentId(equipmentId);
	}

	std::vector<AppSignalManager::SourceState> AppSignalManager::signalStateAllSources(const QString& appSignalId) const
	{
		return m_core.signalStateAllSources(appSignalId);
	}

} // namespace ClientLib
