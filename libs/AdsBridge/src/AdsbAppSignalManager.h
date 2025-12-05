#pragma once

#include "../include/AdsBridge/Common.h"
#include "AdsBridgeResources.h"

#include <AdsConnectionLib/ILoggerStd.h>
#include <AppSignalLibStd/AppSignalManagerCore.h>
#include <AppSignalLibStd/IAppSignalManagerT.h>
#include <AppSignalLibStd/IAppSignalUpdater.h>
#include <AppSignalLibStd/IRecentAppSignals.h>
#include <AppSignalLibStd/ISignalDataServer.h>
#include <AppSignalLibStd/RecentUsed.h>


template<>
struct AppSignalParamAccessor<::MatsAppSignalParam>
{
	static Hash hash(const ::MatsAppSignalParam& appSignalParam) { return appSignalParam.hash; }

	static std::string appSignalId(const ::MatsAppSignalParam& appSignalParam)
	{
		if (appSignalParam.appSignalId == nullptr)
		{
			assert(appSignalParam.appSignalId);
			return std::string{};
		}

		return std::string{appSignalParam.appSignalId};
	}

	static std::string equipmentId(const ::MatsAppSignalParam& appSignalParam)
	{
		if (appSignalParam.equipmentId == nullptr)
		{
			assert(appSignalParam.equipmentId);
			return std::string{};
		}

		return std::string{appSignalParam.equipmentId};
	}

	static MatsSignalType type(const ::MatsAppSignalParam& appSignalParam) { return appSignalParam.type; }

	static std::set<std::string> tags(const ::MatsAppSignalParam& appSignalParam)
	{
		std::set<std::string> result;

		if (appSignalParam.tags == nullptr)
		{
			assert(appSignalParam.tags);
			return result;
		}

		std::istringstream iss{appSignalParam.tags};
		std::string tag;
		while (iss >> tag)
		{
			result.insert(std::move(tag));
		}

		return result;
	}

	static bool hasTag(const ::MatsAppSignalParam& appSignalParam, const std::string& tag) { return tags(appSignalParam).contains(tag); }

	static std::vector<std::string> tagStringList(const ::MatsAppSignalParam& appSignalParam)
	{
		std::vector<std::string> result;
		if (appSignalParam.tags == nullptr)
		{
			assert(appSignalParam.tags);
			return result;
		}

		std::istringstream iss{appSignalParam.tags};
		std::string tag;
		while (iss >> tag)
		{
			result.push_back(std::move(tag));
		}

		return result;
	}
};

template<>
struct AppSignalStateAccessor<::MatsAppSignalState>
{
	static Hash hash(const ::MatsAppSignalState& state) { return state.hash; }
	static void setHash(::MatsAppSignalState& state, Hash hash) { state.hash = hash; }

	static bool isStateAvailable(const ::MatsAppSignalState& state) { return state.flags & MATS_FLAG_STATE_AVAILABLE ? true : false; }

	static MatsTimeStamp plantTime(const ::MatsAppSignalState& state) { return state.plantTime; }

	static ::MatsAppSignalState fromProto(const ::Proto::AppSignalState& protoState)
	{
		MatsAppSignalState state{};
		state.hash = protoState.hash();

		state.plantTime = protoState.planttime();
		state.serverTime = protoState.systemtime();

		state.value = protoState.value();
		state.flags = static_cast<uint32_t>(protoState.flags());

		return state;
	}
};


using ISignalManagerStd = ISignalManagerT<::MatsAppSignalParam, std::string, std::vector<std::string>>;
using IAppSignalManagerStd =
	IAppSignalManagerT<::MatsAppSignalParam, ::MatsAppSignalState, std::string, std::vector<std::string>, ::MatsSignalType>;

namespace AdsBridge
{
	class AdsbAppSignalManager final : public IAppSignalManagerStd,
									   public ClientLib::IAppSignalUpdater,
									   public ClientLib::IRecentAppSignals,
									   public ClientLib::ISignalDataServer
	{
	public:
		explicit AdsbAppSignalManager(AdsBridge::Resources& res);
		virtual ~AdsbAppSignalManager() = default;

		// IAppSignalUpdater implementation
		//
	public:
		virtual void reset() override;

		/// This should be called manually when all signal params are added.
		///
		virtual void notifySignalParamsUpdated() override;

		virtual void addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId) override;

		virtual void invalidateSignalStates(SourceIdType sourceThreadId) override;

		virtual void setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId) override;

		//
		// End of IAppSignalUpdater implementation
		//
		// IRecentAppSignals implementation
		//
	public:
		virtual void addRecentAppSignal(Hash hash) override;
		virtual void addRecentAppSignals(std::span<const Hash> hashes) override;

		virtual std::vector<Hash> recentlyUsedAppSignals(const std::string& appDataServivceId) override;
		virtual bool hasRecentlyUsedAppSignals() override;

		//
		// End of IRecentAppSignals implementation

		// IAppSignalManager implementation - AppSignals
		//
		using ISignalManagerStd::signalExists;
		using ISignalManagerStd::signalParam;
		using IAppSignalManagerStd::signalState;
		using IAppSignalManagerStd::signalTags;
		using IAppSignalManagerStd::signalHasTag;
		using IAppSignalManagerStd::signalType;

		std::vector<Hash> signalHashes() const override;

		int signalsCount() const override;
		std::vector<MatsAppSignalParam> signalList() const override;

		bool signalExists(Hash hash) const override;
		bool signalsExist(const std::vector<std::string>& signalIds) const override;

		std::optional<MatsAppSignalParam> signalParam(Hash signalHash) const override;

		std::optional<MatsAppSignalState> signalState(Hash signalHash) const override;
		std::optional<MatsAppSignalState> signalState(Hash signalHash, Hash dataServerHash) const override;

		void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<MatsAppSignalState>>* result) const override;
		void signalState(std::span<const Hash> appSignalHashes,
						 Hash dataServerHash,
						 std::vector<std::optional<MatsAppSignalState>>* result) const override;

		std::vector<std::string> signalTags(Hash signalHash) const override;

		bool signalHasTag(Hash signalHash, const std::string& tag) const override;

		std::vector<std::string> signalIdsByTag(const std::string& tag) const override;

		MatsSignalType signalType(Hash signalHash, bool* found) const override;

		std::string equipmentToAppSignalId(const std::string& equipmentId) const override;

		// IAppSignalManager implementation - Setpoints
		//
		[[nodiscard]] std::vector<std::shared_ptr<Comparator>> setpointsByInput(const std::string& appSignalId) const override;
		[[nodiscard]] std::shared_ptr<Comparator> setpointByOutput(const std::string& appSignalId) const override;

		//
		// ISignalDataServer implementation
		//

		/// Get AppDataService EquipmentIDs list by AppSignalID.
		///
		std::vector<std::string> dataServiceIds(const std::string& appSignalId) const override;

		/// Return true if AppDataService contains signal.
		///
		bool dataServiceHasSignal(const std::string& serviceEquipmentId, const std::string& appSignalId) const override;
		bool dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const override;

		/// Get all signals for the specified DataServiceID (AppDataService or DiagDataService).
		///
		std::vector<Hash> dataServiceSignals(const std::string& serviceEquipmentId) const override;

		/// Extension, not part of ISignalDataServer, at least yet.
		///
		void filterByDataService(const std::string& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const;

		// Tags
		//
		std::vector<std::string> tags() const override;

	private:
		using CoreType =
			AppSignalStdLib::AppSignalManagerCore<::MatsAppSignalParam, ::MatsAppSignalState, std::string, std::vector<std::string>>;

	public:
		using SourceState = CoreType::SourceState;
		std::vector<SourceState> signalStateAllSources(const std::string& appSignalId) const;

	private:
		AdsBridge::Resources& m_res;
		CoreType m_core;

		mutable std::mutex m_recentUsedMutex; // It cannot be read/write locker, as every fetch the time inside RecentUsed is reset (what is
											  // write operation).
		AppSignalLib::RecentUsed m_recentUsed;
	};
} // namespace AdsBridge