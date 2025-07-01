#pragma once
#include <vector>

// #include "../AppSignalLib/AppSignalState.h" // This must be included via precompiled header

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
	[[nodiscard]] virtual bool signalExists(const QString& appSignalId) const = 0;
	[[nodiscard]] virtual bool signalsExist(const QStringList& signalIds) const = 0;

	[[nodiscard]] virtual AppSignalParam signalParam(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;
};
