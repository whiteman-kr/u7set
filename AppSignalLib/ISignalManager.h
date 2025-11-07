#pragma once
#include <optional>
#include <vector>

// #include "../AppSignalLib/AppSignalState.h" // This must be included via precompiled header

// Use this to unhide base class methods in derived classes, if needed:
//		using ISignalManager::signalExists;
//		using ISignalManager::signalParam;
//

class ISignalManager
{
public:
	virtual ~ISignalManager() = default;

	// AppSignals
	//
	[[nodiscard]] virtual int signalsCount() const = 0;
	[[nodiscard]] virtual std::vector<Hash> signalHashes() const = 0;
	[[nodiscard]] virtual std::vector<AppSignalParam> signalList() const = 0;

	[[nodiscard]] virtual bool signalExists(Hash hash) const = 0;
	[[nodiscard]] bool signalExists(const QString& appSignalId) const { return signalExists(::calcHash(appSignalId)); }
	[[nodiscard]] virtual bool signalsExist(const QStringList& signalIds) const = 0;

	[[nodiscard]] virtual std::optional<AppSignalParam> signalParam(Hash signalHash) const = 0;
	[[nodiscard]] std::optional<AppSignalParam> signalParam(const QString& appSignalId) const
	{
		return signalParam(::calcHash(appSignalId));
	}
};