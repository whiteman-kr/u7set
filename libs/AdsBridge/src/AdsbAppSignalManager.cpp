#include "AdsbAppSignalManager.h"

namespace
{
	::MatsAppSignalParam loadAppSignalParam(const ::Proto::AppSignal& ps, AdsBridge::Resources& resources)
	{
		::MatsAppSignalParam signalParam{};

		signalParam.hash = ::calcHash(ps.appsignalid());

		signalParam.appSignalId = resources.getString(ps.appsignalid());
		signalParam.customSignalId = resources.getString(ps.customappsignalid());

		signalParam.caption = resources.getString(ps.caption());
		signalParam.equipmentId = resources.getString(ps.equipmentid());
		signalParam.lmEquipmentId = resources.getString(ps.lmequipmentid());
		signalParam.units = resources.getString(ps.unit());
		{
			auto joinedView = ps.tags() | std::views::join_with(' ');
			signalParam.tags = resources.getString(std::string(std::ranges::begin(joinedView), std::ranges::end(joinedView)));
		}

		signalParam.channel = static_cast<MatsChannel>(ps.channel());
		signalParam.inOutType = static_cast<MatsSignalInOutType>(ps.inouttype());
		signalParam.type = static_cast<MatsSignalType>(ps.signaltype());
		signalParam.decimalPlaces = ps.decimalplaces();

		signalParam.tuning = ps.enabletuning();

		Proto::SignalSpecPropValues protoValues;
		bool ok =
			protoValues.ParseFromArray(static_cast<const void*>(ps.specpropvalues().data()), static_cast<int>(ps.specpropvalues().size()));
		if (ok == true)
		{
			auto getDoubleValue = [&protoValues](std::string_view name) -> std::optional<double>
			{
				auto it = std::find_if(protoValues.value().begin(),
									   protoValues.value().end(),
									   [name](const Proto::SignalSpecPropValue& v)
									   {
										   return v.name() == name && v.has_doubleval() == true;
									   });
				if (it != protoValues.value().end())
				{
					return it->doubleval();
				}

				return std::nullopt;
			};

			signalParam.lowValidRange = getDoubleValue("LowValidRange").value_or(0.0);
			signalParam.highValidRange = getDoubleValue("HighValidRange").value_or(0.0);
		}

		return signalParam;
	}
} // namespace


namespace AdsBridge
{
	AdsbAppSignalManager::AdsbAppSignalManager(AdsBridge::Resources& res) :
		m_res{res}
	{
		return;
	}

	void AdsbAppSignalManager::reset()
	{
		m_core.reset();
		return;
	}

	void AdsbAppSignalManager::notifySignalParamsUpdated()
	{
		// emit signalParamsUpdated();
		return;
	}

	void AdsbAppSignalManager::addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId)
	{
		return m_core.addSignals(appSignals,
								 appDataServiceId,
								 [this](const ::Proto::AppSignal& ps) -> ::MatsAppSignalParam
								 {
									 return loadAppSignalParam(ps, m_res);
								 });
	}

	void AdsbAppSignalManager::invalidateSignalStates(SourceIdType sourceThreadId)
	{
		return m_core.invalidateSignalStates(sourceThreadId);
	}

	void AdsbAppSignalManager::setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId)
	{
		return m_core.setStates(states, dataServerHash, sourceThreadId);
	}

	void AdsbAppSignalManager::addRecentAppSignal(Hash hash)
	{
		std::scoped_lock locker{m_recentUsedMutex};
		m_recentUsed.add(hash);
	}

	void AdsbAppSignalManager::addRecentAppSignals(std::span<const Hash> hashes)
	{
		std::scoped_lock locker{m_recentUsedMutex};
		m_recentUsed.add(hashes);
	}

	std::vector<Hash> AdsbAppSignalManager::recentlyUsedAppSignals(const std::string& appDataServiceId)
	{
		std::vector<Hash> result;

		{
			std::scoped_lock locker{m_recentUsedMutex};
			m_recentUsed.removeOutdated();

			result = m_recentUsed.hashes();
		}

		filterByDataService(appDataServiceId, result);

		return result;
	}

	bool AdsbAppSignalManager::hasRecentlyUsedAppSignals()
	{
		std::scoped_lock locker{m_recentUsedMutex};
		return m_recentUsed.hashes().empty() == false;
	}

	std::vector<Hash> AdsbAppSignalManager::signalHashes() const
	{
		return m_core.signalHashes();
	}

	int AdsbAppSignalManager::signalsCount() const
	{
		return m_core.signalsCount();
	}

	std::vector<MatsAppSignalParam> AdsbAppSignalManager::signalList() const
	{
		return m_core.signalList();
	}

	bool AdsbAppSignalManager::signalExists(Hash hash) const
	{
		return m_core.signalExists(hash);
	}

	bool AdsbAppSignalManager::signalsExist(const std::vector<std::string>& signalIds) const
	{
		return m_core.signalsExist(signalIds);
	}

	std::optional<MatsAppSignalParam> AdsbAppSignalManager::signalParam(Hash signalHash) const
	{
		return m_core.signalParam(signalHash);
	}

	std::optional<MatsAppSignalState> AdsbAppSignalManager::signalState(Hash signalHash) const
	{
		return signalState(signalHash, {});
	}

	std::optional<MatsAppSignalState> AdsbAppSignalManager::signalState(Hash signalHash, Hash dataServerHash) const
	{
		const_cast<AdsbAppSignalManager*>(this)->addRecentAppSignal(signalHash);
		return m_core.signalState(signalHash, dataServerHash);
	}

	// Ok
	//
	void AdsbAppSignalManager::signalState(std::span<const Hash> appSignalHashes,
										   std::vector<std::optional<MatsAppSignalState>>* result) const
	{
		return signalState(appSignalHashes, {}, result);
	}

	void AdsbAppSignalManager::signalState(std::span<const Hash> appSignalHashes,
										   Hash dataServerHash,
										   std::vector<std::optional<MatsAppSignalState>>* result) const
	{
		assert(result);
		const_cast<AdsbAppSignalManager*>(this)->addRecentAppSignals(appSignalHashes);

		return m_core.signalState(appSignalHashes, dataServerHash, result);
	}

	std::vector<std::string> AdsbAppSignalManager::signalTags(Hash signalHash) const
	{
		return m_core.signalTags(signalHash);
	}

	bool AdsbAppSignalManager::signalHasTag(Hash signalHash, const std::string& tag) const
	{
		return m_core.signalHasTag(signalHash, tag);
	}

	std::vector<std::string> AdsbAppSignalManager::signalIdsByTag(const std::string& tag) const
	{
		return m_core.signalIdsByTag(tag);
	}

	MatsSignalType AdsbAppSignalManager::signalType(Hash signalHash, bool* found) const
	{
		return static_cast<MatsSignalType>(m_core.signalType(signalHash, found));
	}

	std::string AdsbAppSignalManager::equipmentToAppSignalId(const std::string& equipmentId) const
	{
		return m_core.equipmentToAppSignalId(equipmentId);
	}

	std::vector<std::shared_ptr<Comparator>> AdsbAppSignalManager::setpointsByInput([[maybe_unused]] const std::string& appSignalId) const
	{
		// Comparators are not implemented yet.
		//
		assert(false);
		return {};
	}

	std::shared_ptr<Comparator> AdsbAppSignalManager::setpointByOutput([[maybe_unused]] const std::string& appSignalId) const
	{
		// Comparators are not implemented yet.
		//
		assert(false);
		return {};
	}

	/// Get AppDataService EquipmentIDs list by AppSignalID.
	///
	std::vector<std::string> AdsbAppSignalManager::dataServiceIds(const std::string& appSignalId) const
	{
		return m_core.dataServiceIds(appSignalId);
	}

	/// Return true if AppDataService contains signal.
	///
	bool AdsbAppSignalManager::dataServiceHasSignal(const std::string& serviceEquipmentId, const std::string& appSignalId) const
	{
		Hash hash = calcHash(appSignalId);
		return dataServiceHasSignal(serviceEquipmentId, hash);
	}

	bool AdsbAppSignalManager::dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const
	{
		return m_core.dataServiceHasSignal(serviceEquipmentId, signalHash);
	}

	std::vector<Hash> AdsbAppSignalManager::dataServiceSignals(const std::string& serviceEquipmentId) const
	{
		return m_core.dataServiceSignals(serviceEquipmentId);
	}

	void AdsbAppSignalManager::filterByDataService(const std::string& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const
	{
		return m_core.filterByDataService(serviceEquipmentId, inOutSignalHashes);
	}

	std::vector<std::string> AdsbAppSignalManager::tags() const
	{
		return m_core.tags();
	}
} // namespace AdsBridge
