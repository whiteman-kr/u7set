#pragma once

#include "ISignalManagerT.h"

#include <algorithm>
#include <memory>
#include <ranges>
#include <span>


// #include "AppSignalParam.h" -- Commented out to speed up compilation
// #include "AppSignalState.h" -- Commented out to speed up compilation
// These files MUST BE included in the precompiled header of the project:
// 	"../AppSignalLib/AppSignalParam.h"
//  "../AppSignalLib/AppSignalState.h"
//

class Comparator;

// using ISignalManager::signalExists;
// using ISignalManager::signalParam;
// using IAppSignalManager::signalState;
// using IAppSignalManager::signalTags;
// using IAppSignalManager::signalHasTag;
// using IAppSignalManager::signalType;

// Concept to simplify and avoid long requires expressions (fixes GCC parsing issues)
//
template<typename Range>
concept StringRange = std::ranges::range<Range> && !std::same_as<std::remove_cvref_t<Range>, std::span<const Hash>> &&
					  !std::same_as<std::remove_cvref_t<Range>, std::span<Hash>> && StringLike<std::ranges::range_value_t<Range>>;


template<StringLike StringType, typename StringListType>
class IAppSignalManagerT : public ISignalManagerT<StringType, StringListType>
{
public:
	virtual ~IAppSignalManagerT() = default;

	[[nodiscard]] virtual std::optional<AppSignalState> signalState(Hash signalHash) const = 0;
	[[nodiscard]] std::optional<AppSignalState> signalState(const StringType& appSignalId) const;

	[[nodiscard]] virtual std::optional<AppSignalState> signalState(Hash signalHash, Hash dataServerHash) const = 0;
	[[nodiscard]] std::optional<AppSignalState> signalState(const StringType& appSignalId, const StringType& dataServerId) const;

	virtual void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const = 0;
	virtual void signalState(std::span<const Hash> appSignalHashes,
							 Hash dataServerHash,
							 std::vector<std::optional<AppSignalState>>* result) const = 0;

	template<StringRange Range>
	void signalState(const Range& appSignalIds, std::vector<std::optional<AppSignalState>>* result) const;

	template<StringRange Range>
	void signalState(const Range& appSignalIds, const StringType& dataServerId, std::vector<std::optional<AppSignalState>>* result) const;

	[[nodiscard]] virtual StringListType signalTags(Hash signalHash) const = 0;
	[[nodiscard]] StringListType signalTags(const StringType& appSignalId) const { return signalTags(::calcHash(appSignalId)); }

	[[nodiscard]] virtual bool signalHasTag(Hash signalHash, const StringType& tag) const = 0;
	[[nodiscard]] bool signalHasTag(const StringType& appSignalId, const StringType& tag) const;

	[[nodiscard]] virtual StringListType signalIdsByTag(const StringType& tag) const = 0;

	[[nodiscard]] virtual E::SignalType signalType(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] E::SignalType signalType(const StringType& appSignalId, bool* found) const;

	[[nodiscard]] virtual StringType equipmentToAppSignalId(const StringType& equipmentId) const = 0;

	// Setpoints
	//
	[[nodiscard]] virtual std::vector<std::shared_ptr<Comparator>> setpointsByInput(const StringType& appSignalId) const = 0;
	[[nodiscard]] virtual std::shared_ptr<Comparator> setpointByOutput(const StringType& appSignalId) const = 0;

	// Tags
	//
	[[nodiscard]] virtual StringListType tags() const = 0;
};

template<StringLike StringType, typename StringListType>
inline std::optional<AppSignalState> IAppSignalManagerT<StringType, StringListType>::signalState(const StringType& appSignalId) const
{
	return signalState(::calcHash(appSignalId));
}

template<StringLike StringType, typename StringListType>
template<StringRange Range>
void IAppSignalManagerT<StringType, StringListType>::signalState(const Range& appSignalIds,
																 std::vector<std::optional<AppSignalState>>* result) const
{
	std::vector<Hash> appSignalHashes;
	appSignalHashes.reserve(std::size(appSignalIds));

	std::transform(std::begin(appSignalIds),
				   std::end(appSignalIds),
				   std::back_inserter(appSignalHashes),
				   [](const auto& id)
				   {
					   return ::calcHash(id);
				   });
	return signalState(std::span<const Hash>(appSignalHashes), result);
}

template<StringLike StringType, typename StringListType>
template<StringRange Range>
void IAppSignalManagerT<StringType, StringListType>::signalState(const Range& appSignalIds,
																 const StringType& dataServerId,
																 std::vector<std::optional<AppSignalState>>* result) const
{
	std::vector<Hash> appSignalHashes;
	appSignalHashes.reserve(appSignalIds.size());
	for (const StringType& id : appSignalIds)
	{
		appSignalHashes.push_back(::calcHash(id));
	}

	return signalState(std::span<const Hash>(appSignalHashes), ::calcHash(dataServerId), result);
}

template<StringLike StringType, typename StringListType>
inline std::optional<AppSignalState> IAppSignalManagerT<StringType, StringListType>::signalState(const StringType& appSignalId,
																								 const StringType& dataServerId) const
{
	return signalState(::calcHash(appSignalId), ::calcHash(dataServerId));
}

template<StringLike StringType, typename StringListType>
inline bool IAppSignalManagerT<StringType, StringListType>::signalHasTag(const StringType& appSignalId, const StringType& tag) const
{
	return signalHasTag(::calcHash(appSignalId), tag);
}

template<StringLike StringType, typename StringListType>
inline E::SignalType IAppSignalManagerT<StringType, StringListType>::signalType(const StringType& appSignalId, bool* found) const
{
	return signalType(::calcHash(appSignalId), found);
}
