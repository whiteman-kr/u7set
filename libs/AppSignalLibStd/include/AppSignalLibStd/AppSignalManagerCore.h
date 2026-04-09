#pragma once

#include <AppSignalLibStd/AppSignalAccessor.h>
#include <AppSignalLibStd/IAppSignalUpdater.h>

#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace AppSignalStdLib
{
	template<typename SignalParamType, // AppSignalParam../..MatsAppSignalParam
			 typename SignalStateType, // AppSignalState../..MatsAppSignalState
			 typename StringType,      // QString........./..std::string
			 typename StringListType>  // QStringList...../..std::vector<std::string>
	class AppSignalManagerCore
	{
	public:
		using SourceIdType = ClientLib::IAppSignalUpdater::SourceIdType;

		struct SourceState
		{
			SignalStateType state{};
			Hash dataServerHash{UNDEFINED_HASH};
			SourceIdType sourceThreadId{};
			std::chrono::time_point<std::chrono::system_clock> lastUpdateTime{}; // State last time received or updated
		};

	public:
		void reserve(size_t size)
		{
			{
				std::unique_lock wl{m_paramsLocker};
				m_signalParams.reserve(size);
				m_signalParamByEquipmentId.reserve(size);
			}

			{
				std::unique_lock wl{m_statesLocker};
				m_states.max_load_factor(0.75);
				m_states.reserve(size);
			}
		}

		void reset()
		{
			{
				std::unique_lock wl{m_paramsLocker};
				m_signalParams.clear();
				m_signalParamByEquipmentId.clear();
				m_tagToAppSignals.clear();
				m_tags.clear();
				m_appDataServiceToSignalHashList.clear();
			}

			{
				std::unique_lock wl{m_statesLocker};
				m_states.clear();
			}
		}

		void addSignals(std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId, const auto& transformFunc)
		{
			std::unique_lock wl{m_paramsLocker};

			for (const auto& ps : appSignals)
			{
				addSignalPrivate(transformFunc(ps), appDataServiceId);
			}

			return;
		}

		void addSignalPrivate(SignalParamType&& appSignal, const std::string& appDataServiceId)
		{
			// Actually, EquipmentID does not starts from the symbol '@',
			// but we need it particularly for Monitor to distinct AppSignalID from EquipmentID.
			//
			m_signalParamByEquipmentId[StringType{"@"} + AppSignalParamAccessor<SignalParamType>::equipmentId(appSignal)] =
				AppSignalParamAccessor<SignalParamType>::appSignalId(appSignal);

			// --
			//
			m_appDataServiceToSignalHashList[appDataServiceId].insert(AppSignalParamAccessor<SignalParamType>::hash(appSignal));

			// Add AppSignalId to tags, so it will be possible to find all signals by tag.
			//
			const StringType& appSignalId = AppSignalParamAccessor<SignalParamType>::appSignalId(appSignal);
			const auto& tags = AppSignalParamAccessor<SignalParamType>::tags(appSignal);

			for (const auto& tag : tags)
			{
				auto& l = m_tagToAppSignals[tag];

				if (l.empty() == true)
				{
					l.reserve(512);
				}

				l.push_back(appSignalId);
			}

			// Add tags to commot tag set
			//
			m_tags.insert(tags.begin(), tags.end());

			// Finally, add signal param which is moved!
			//
			m_signalParams.emplace(AppSignalParamAccessor<SignalParamType>::hash(appSignal), std::move(appSignal));

			return;
		}

		void invalidateSignalStates(SourceIdType sourceThreadId)
		{
			auto now = std::chrono::system_clock::now();

			std::unique_lock wl{m_statesLocker};
			for (auto& [signalHash, source] : m_states)
			{
				source.invalidateSource(sourceThreadId, now);
			}

			return;
		}

		void setStates(std::span<const ::Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId)
		{
			auto now = std::chrono::system_clock::now();

			std::unique_lock wl{m_statesLocker};

			for (const auto& protoState : states)
			{
				Sources& currentStateAndSources = m_states[protoState.hash()];
				SignalStateType state = AppSignalStateAccessor<SignalStateType>::fromProto(protoState);
				currentStateAndSources.set(state, dataServerHash, sourceThreadId, now);
			}

			return;
		}

		std::vector<Hash> signalHashes() const
		{
			std::shared_lock rl{m_paramsLocker};

			std::vector<Hash> result;
			result.reserve(m_signalParams.size());

			for (auto& s : m_signalParams)
			{
				result.push_back(s.first);
			}

			return result;
		}

		int signalsCount() const
		{
			std::shared_lock rl{m_paramsLocker};
			return static_cast<int>(m_signalParams.size());
		}

		std::vector<SignalParamType> signalList() const
		{
			std::shared_lock rl{m_paramsLocker};

			std::vector<SignalParamType> result;
			result.reserve(m_signalParams.size());

			for (auto& s : m_signalParams)
			{
				result.push_back(s.second);
			}

			return result;
		}

		bool signalExists(Hash hash) const
		{
			std::shared_lock rl{m_paramsLocker};
			return m_signalParams.contains(hash);
		}

		bool signalsExist(const auto& signalIds) const
		{
			std::shared_lock rl{m_paramsLocker};
			return std::all_of(signalIds.begin(),
							   signalIds.end(),
							   [this](const auto& appSignalId)
							   {
								   return m_signalParams.contains(::calcHash(appSignalId));
							   });
		}

		std::optional<SignalParamType> signalParam(Hash signalHash) const
		{
			std::shared_lock rl{m_paramsLocker};

			auto it = m_signalParams.find(signalHash);
			if (it != m_signalParams.end())
			{
				return it->second;
			}
			else
			{
				return std::nullopt;
			}
		}

		std::optional<SignalStateType> signalState(Hash signalHash, Hash dataServerHash) const
		{
			if (signalHash == UNDEFINED_HASH)
			{
				return std::nullopt;
			}

			std::shared_lock rl{m_statesLocker};

			auto foundState = m_states.find(signalHash);
			if (foundState != m_states.end())
			{
				if (dataServerHash == UNDEFINED_HASH)
				{
					return foundState->second.get();
				}
				else
				{
					return foundState->second.getForDataServer(dataServerHash);
				}
			}
			else
			{
				// State is not found, but maybe it is just not received yet.
				// Check if such signal exists, then create invalid state.
				//
				rl.unlock();

				std::shared_lock prl{m_paramsLocker};

				auto foundParam = m_signalParams.find(signalHash);
				if (foundParam != m_signalParams.end())
				{
					SignalStateType s{};
					auto hash = AppSignalParamAccessor<SignalParamType>::hash(foundParam->second);
					AppSignalStateAccessor<SignalStateType>::setHash(s, hash);
					return s;
				}

				return std::nullopt;
			}
		}

		void signalState(std::span<const Hash> appSignalHashes,
						 Hash dataServerHash,
						 std::vector<std::optional<SignalStateType>>* result) const
		{
			assert(result);

			result->clear();
			result->reserve(appSignalHashes.size());

			std::shared_lock rl{m_statesLocker};

			for (Hash signalHash : appSignalHashes)
			{
				auto foundState = m_states.find(signalHash);

				if (foundState != m_states.end())
				{
					if (dataServerHash == UNDEFINED_HASH)
					{
						result->push_back(foundState->second.get());
					}
					else
					{
						result->push_back(foundState->second.getForDataServer(dataServerHash));
					}
				}
				else
				{
					result->push_back(std::nullopt);
				}
			}

			return;
		}

		StringListType signalTags(Hash signalHash) const
		{
			StringListType result;

			std::shared_lock rl{m_paramsLocker};

			if (auto it = m_signalParams.find(signalHash); it != m_signalParams.end())
			{
				result = AppSignalParamAccessor<SignalParamType>::tagStringList(it->second);
			}

			return result;
		}

		bool signalHasTag(Hash signalHash, const StringType& tag) const
		{
			std::shared_lock rl{m_paramsLocker};

			auto result = m_signalParams.find(signalHash);
			return result == m_signalParams.end() ? false : AppSignalParamAccessor<SignalParamType>::hasTag(result->second, tag);
		}

		// Return type is int, user must cast it to E::SignalType or MatsSignalType
		//
		int signalType(Hash signalHash, bool* found) const
		{
			std::shared_lock rl{m_paramsLocker};

			auto result = m_signalParams.find(signalHash);
			if (found != nullptr)
			{
				*found = (result != m_signalParams.end());
			}

			// Analog,   -- 0
			// Discrete, -- 1
			// Bus       -- 2
			//
			return result == m_signalParams.end() ? 1 : static_cast<int>(AppSignalParamAccessor<SignalParamType>::type(result->second));
		}

		StringListType signalIdsByTag(const StringType& tag) const
		{
			std::shared_lock rl{m_paramsLocker};

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

		StringType equipmentToAppSignalId(const StringType& equipmentId) const
		{
			StringType result;

			std::shared_lock rl{m_paramsLocker};

			auto it = m_signalParamByEquipmentId.find(equipmentId);
			if (it != m_signalParamByEquipmentId.end())
			{
				result = it->second;
			}

			return result;
		}

		/// Get AppDataService EquipmentIDs list by AppSignalID.
		///
		std::vector<std::string> dataServiceIds(const std::string& appSignalId) const
		{
			std::shared_lock rl{m_paramsLocker};

			Hash hash = calcHash(appSignalId);

			std::vector<std::string> result;
			result.reserve(2);

			for (const auto& [appDataServcieId, signalSet] : m_appDataServiceToSignalHashList)
			{
				if (signalSet.contains(hash) == true)
				{
					result.push_back(appDataServcieId);
				}
			}

			return result;
		}

		bool dataServiceHasSignal(const std::string& serviceEquipmentId, Hash signalHash) const
		{
			std::shared_lock rl{m_paramsLocker};

			auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
			if (it == m_appDataServiceToSignalHashList.end())
			{
				return false;
			}

			return it->second.contains(signalHash);
		}

		void filterByDataService(const std::string& serviceEquipmentId, std::vector<Hash>& inOutSignalHashes) const
		{
			std::shared_lock rl{m_paramsLocker};

			auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
			if (it == m_appDataServiceToSignalHashList.end())
			{
				inOutSignalHashes.clear();
				return;
			}

			const std::unordered_set<Hash>& sh = it->second;

			// Filter all signals which are not belong to serviceEquipmentId.
			//
			std::erase_if(inOutSignalHashes,
						  [&sh](Hash hash)
						  {
							  return sh.contains(hash) == false;
						  });

			return;
		}

		std::vector<Hash> dataServiceSignals(const std::string& serviceEquipmentId) const
		{
			std::vector<Hash> result;

			std::shared_lock rl{m_paramsLocker};

			auto it = m_appDataServiceToSignalHashList.find(serviceEquipmentId);
			if (it != m_appDataServiceToSignalHashList.end())
			{
				const auto& signalsByDataService = it->second;

				result.reserve(signalsByDataService.size());
				std::copy(signalsByDataService.begin(), signalsByDataService.end(), std::back_inserter(result));
			}

			return result;
		}

		StringListType tags() const
		{
			std::shared_lock rl{m_paramsLocker};

			StringListType result;
			result.reserve(m_tags.size());

			for (const auto& t : m_tags)
			{
				result.push_back(t);
			}

			return result;
		}

		std::optional<SignalParamType> signalParamByEquipmentId(const StringType& equipmentId) const
		{
			std::shared_lock rl{m_paramsLocker};

			Hash signalIdHash = UNDEFINED_HASH;

			if (auto it = m_signalParamByEquipmentId.find(equipmentId); //
				it == m_signalParamByEquipmentId.end())
			{
				return std::nullopt;
			}
			else
			{
				signalIdHash = ::calcHash(it->second);
			}

			if (auto it = m_signalParams.find(signalIdHash); //
				it != m_signalParams.end())
			{
				return it->second;
			}
			else
			{
				return std::nullopt;
			}
		}

		std::vector<SourceState> signalStateAllSources(const StringType& appSignalId) const
		{
			std::vector<SourceState> result;
			result.reserve(4);

			std::shared_lock rl{m_statesLocker};

			auto foundState = m_states.find(::calcHash(appSignalId));
			if (foundState != m_states.end())
			{
				const Sources& sources = foundState->second;

				for (const auto& source : sources.sources)
				{
					result.push_back(source);
				}
			}

			return result;
		}

	private:
		struct Sources
		{
			size_t size = 0;
			std::array<SourceState, 4> sources{}; // 4 maximum possible channels of getting signal (2 regular, 2 recent)

			void set(const SignalStateType& state,
					 Hash dataServerHash,
					 SourceIdType sourceThreadId,
					 std::chrono::time_point<std::chrono::system_clock> now /*= std::chrono::system_clock::now()*/)
			{
				SourceState* emptyState = nullptr;
				for (SourceState& sourceState : sources)
				{
					if (sourceState.sourceThreadId == sourceThreadId)
					{
						sourceState.state = state;
						sourceState.lastUpdateTime = now;
						return;
					}

					if (sourceState.sourceThreadId == 0)
					{
						emptyState = &sourceState;
					}
				}

				if (emptyState == nullptr)
				{
					// No empty space in sources
					//
					assert(emptyState);

					// Try to mitigate it, and set value to the last item
					//
					emptyState = &sources.back();
				}

				*emptyState = SourceState{state, dataServerHash, sourceThreadId, now};
				return;
			}

			void invalidateSource(SourceIdType sourceThreadId,
								  std::chrono::time_point<std::chrono::system_clock> now /* = std::chrono::system_clock::now()*/)
			{
				for (SourceState& sourceState : sources)
				{
					if (sourceState.sourceThreadId == sourceThreadId)
					{
						sourceState.state = SignalStateType{};
						sourceState.lastUpdateTime = now;
						break;
					}
				}

				return;
			}

			[[nodiscard]] const SignalStateType& get() const
			{
				// Find the newest available state
				//
				const SourceState* stateAvailable = nullptr;
				const SourceState* stateNewest = nullptr;

				for (const SourceState& sourceState : sources)
				{
					if (sourceState.sourceThreadId == 0)
					{
						continue;
					}

					if (AppSignalStateAccessor<SignalStateType>::isStateAvailable(sourceState.state) == true)
					{
						if (stateAvailable == nullptr || AppSignalStateAccessor<SignalStateType>::plantTime(stateAvailable->state) <
															 AppSignalStateAccessor<SignalStateType>::plantTime(sourceState.state))
						{
							stateAvailable = &sourceState; // the first state with state available flag
						}
					}
					else
					{
						// sourceState.state.isStateAvailable() == false
						//
						if (stateNewest == nullptr || stateNewest->lastUpdateTime < sourceState.lastUpdateTime)
						{
							stateNewest = &sourceState;
						}
					}
				}

				if (stateAvailable != nullptr)
				{
					return stateAvailable->state;
				}

				if (stateNewest != nullptr)
				{
					return stateNewest->state;
				}

				static const SignalStateType NotValidState{};
				return NotValidState;
			}
			[[nodiscard]] const SignalStateType& getForDataServer(Hash dataServerHash) const
			{
				// Find the newest available state
				//
				const SourceState* stateAvailable = nullptr;
				const SourceState* stateNewest = nullptr;

				for (const SourceState& sourceState : sources)
				{
					if (sourceState.sourceThreadId == 0 || sourceState.dataServerHash != dataServerHash)
					{
						continue;
					}

					if (AppSignalStateAccessor<SignalStateType>::isStateAvailable(sourceState.state) == true)
					{
						if (stateAvailable == nullptr || AppSignalStateAccessor<SignalStateType>::plantTime(stateAvailable->state) <
															 AppSignalStateAccessor<SignalStateType>::plantTime(sourceState.state))
						{
							stateAvailable = &sourceState; // the first state with state available flag
						}
					}
					else
					{
						// sourceState.state.isStateAvailable() == false
						//
						if (stateNewest == nullptr || stateNewest->lastUpdateTime < sourceState.lastUpdateTime)
						{
							stateNewest = &sourceState;
						}
					}
				}

				if (stateAvailable != nullptr)
				{
					return stateAvailable->state;
				}

				if (stateNewest != nullptr)
				{
					return stateNewest->state;
				}

				static const SignalStateType NotValidState{};
				return NotValidState;
			}
		};

		mutable std::shared_mutex m_paramsLocker;
		std::unordered_map<Hash, const SignalParamType> m_signalParams;        // Key is hash from AppSignalID
		std::unordered_map<StringType, StringType> m_signalParamByEquipmentId; // Key is EquipmentId - value is AppSignalID
		std::unordered_map<StringType, StringListType> m_tagToAppSignals;      // Key is tag - value is list of AppSignalIDs with this tag
		std::set<StringType> m_tags;                                           // All tags for received AppSignals
		std::map<std::string, std::unordered_set<Hash>>
			m_appDataServiceToSignalHashList; // Key is AppDataServiceID, value is AppSignals received via this AppDataService

		mutable std::shared_mutex m_statesLocker;
		std::unordered_map<Hash, Sources> m_states;
	};

} // namespace AppSignalStdLib
