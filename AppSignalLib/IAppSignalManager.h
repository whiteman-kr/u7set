#pragma once

#include "ISignalManager.h"

#include <concepts>
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
concept QStringRange =
	std::ranges::range<Range> && !std::same_as<std::remove_cvref_t<Range>, std::span<const Hash>> &&
	!std::same_as<std::remove_cvref_t<Range>, std::span<Hash>> && std::convertible_to<std::ranges::range_value_t<Range>, QString>;

class IAppSignalManager : public ISignalManager
{
public:
	virtual ~IAppSignalManager() = default;

	[[nodiscard]] virtual std::optional<AppSignalState> signalState(Hash signalHash) const = 0;
	[[nodiscard]] std::optional<AppSignalState> signalState(const QString& appSignalId) const;

	[[nodiscard]] virtual std::optional<AppSignalState> signalState(Hash signalHash, Hash dataServerHash) const = 0;
	[[nodiscard]] std::optional<AppSignalState> signalState(const QString& appSignalId, const QString& dataServerId) const;

	virtual void signalState(std::span<const Hash> appSignalHashes, std::vector<std::optional<AppSignalState>>* result) const = 0;
	virtual void signalState(std::span<const Hash> appSignalHashes,
							 Hash dataServerHash,
							 std::vector<std::optional<AppSignalState>>* result) const = 0;

	template<QStringRange Range>
	void signalState(const Range& appSignalIds, std::vector<std::optional<AppSignalState>>* result) const;

	template<QStringRange Range>
	void signalState(const Range& appSignalIds, const QString& dataServerId, std::vector<std::optional<AppSignalState>>* result) const;

	[[nodiscard]] virtual QStringList signalTags(Hash signalHash) const = 0;
	[[nodiscard]] QStringList signalTags(const QString& appSignalId) const { return signalTags(::calcHash(appSignalId)); }

	[[nodiscard]] virtual bool signalHasTag(Hash signalHash, const QString& tag) const = 0;
	[[nodiscard]] bool signalHasTag(const QString& appSignalId, const QString& tag) const;

	[[nodiscard]] virtual QStringList signalIdsByTag(const QString& tag) const = 0;

	[[nodiscard]] virtual E::SignalType signalType(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] E::SignalType signalType(const QString& appSignalId, bool* found) const;

	[[nodiscard]] virtual QString equipmentToAppSignalId(const QString& equipmentId) const = 0;

	// Setpoints
	//
	[[nodiscard]] virtual std::vector<std::shared_ptr<Comparator>> setpointsByInput(const QString& appSignalId) const = 0;
	[[nodiscard]] virtual std::shared_ptr<Comparator> setpointByOutput(const QString& appSignalId) const = 0;

	// Tags
	//
	[[nodiscard]] virtual QStringList tags() const = 0;
};


inline std::optional<AppSignalState> IAppSignalManager::signalState(const QString& appSignalId) const
{
	return signalState(::calcHash(appSignalId));
}

template<QStringRange Range>
void IAppSignalManager::signalState(const Range& appSignalIds, std::vector<std::optional<AppSignalState>>* result) const
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

template<QStringRange Range>
void IAppSignalManager::signalState(const Range& appSignalIds,
									const QString& dataServerId,
									std::vector<std::optional<AppSignalState>>* result) const
{
	std::vector<Hash> appSignalHashes;
	appSignalHashes.reserve(appSignalIds.size());
	for (const QString& id : appSignalIds)
	{
		appSignalHashes.push_back(::calcHash(id));
	}

	return signalState(std::span<const Hash>(appSignalHashes), ::calcHash(dataServerId), result);
}

inline std::optional<AppSignalState> IAppSignalManager::signalState(const QString& appSignalId, const QString& dataServerId) const
{
	return signalState(::calcHash(appSignalId), ::calcHash(dataServerId));
}

inline bool IAppSignalManager::signalHasTag(const QString& appSignalId, const QString& tag) const
{
	return signalHasTag(::calcHash(appSignalId), tag);
}

inline E::SignalType IAppSignalManager::signalType(const QString& appSignalId, bool* found) const
{
	return signalType(::calcHash(appSignalId), found);
}