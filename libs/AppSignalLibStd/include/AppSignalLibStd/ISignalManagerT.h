#pragma once

#include <concepts>
#include <optional>
#include <string>
#include <vector>

// #include "../AppSignalLib/AppSignalState.h" // This must be included via precompiled header

// Use this to unhide base class methods in derived classes, if needed:
//		using ISignalManager::signalExists;
//		using ISignalManager::signalParam;
//

// Concept to check if a type is a valid string type
//
template<typename T>
concept StringLike = requires(const T& str) {
	//	{ str.empty() } -> std::convertible_to<bool>;
	{ str.size() } -> std::convertible_to<std::size_t>;
	//	{ str.data() } -> std::convertible_to<const char*>;
} || std::same_as<std::remove_cvref_t<T>, std::string>;


template<typename SignalParamType, // AppSignalParam../..MatsAppSignalParam
		 StringLike StringType,    //
		 typename StringListType>  //
class ISignalManagerT
{
public:
	virtual ~ISignalManagerT() = default;

	// AppSignals
	//
	[[nodiscard]] virtual int signalsCount() const = 0;
	[[nodiscard]] virtual std::vector<Hash> signalHashes() const = 0;
	[[nodiscard]] virtual std::vector<SignalParamType> signalList() const = 0;

	[[nodiscard]] virtual bool signalExists(Hash hash) const = 0;
	[[nodiscard]] bool signalExists(const StringType& appSignalId) const { return signalExists(::calcHash(appSignalId)); }
	[[nodiscard]] virtual bool signalsExist(const StringListType& signalIds) const = 0;

	[[nodiscard]] virtual std::optional<SignalParamType> signalParam(Hash signalHash) const = 0;
	[[nodiscard]] std::optional<SignalParamType> signalParam(const StringType& appSignalId) const
	{
		return signalParam(::calcHash(appSignalId));
	}
};
