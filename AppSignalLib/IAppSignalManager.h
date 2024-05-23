#pragma once

#include <span>
#include <vector>

// #include "AppSignalParam.h" -- Commented out to speed up compilation
// #include "AppSignalState.h" -- Commented out to speed up compilation
// These files MUST BE included in the precompiled header of the project:
// 	"../AppSignalLib/AppSignalParam.h"
//  "../AppSignalLib/AppSignalState.h"
//

class Comparator;

class IAppSignalManager
{
public:
	virtual ~IAppSignalManager() = default;

	// AppSignals
	//
	[[nodiscard]] virtual int signalsCount() const = 0;
	[[nodiscard]] virtual std::vector<AppSignalParam> signalList() const = 0;

	[[nodiscard]] virtual bool signalExists(Hash hash) const = 0;
	[[nodiscard]] virtual bool signalExists(const QString& appSignalId) const = 0;
	[[nodiscard]] virtual bool signalsExist(const QStringList& signalIds) const = 0;

	[[nodiscard]] virtual AppSignalParam signalParam(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

	[[nodiscard]] virtual AppSignalState signalState(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] virtual AppSignalState signalState(const QString& appSignalId, bool* found) const = 0;
	[[nodiscard]] virtual AppSignalState signalState(Hash signalHash, Hash dataServerHash, bool* found) const = 0;
	[[nodiscard]] virtual AppSignalState signalState(const QString& appSignalId, const QString& dataServerId, bool* found) const = 0;

	virtual void signalState(std::span<const Hash> appSignalHashes, std::vector<AppSignalState>* result, int* found) const = 0;
	virtual void signalState(std::span<const QString> appSignalIds, std::vector<AppSignalState>* result, int* found) const = 0;
	virtual void signalState(std::span<const Hash> appSignalHashes,
							 Hash dataServerHash,
							 std::vector<AppSignalState>* result,
							 int* found) const = 0;
	virtual void signalState(std::span<const QString> appSignalIds,
							 const QString& dataServerId,
							 std::vector<AppSignalState>* result,
							 int* found) const = 0;

	[[nodiscard]] virtual QStringList signalTags(Hash signalHash) const = 0;
	[[nodiscard]] virtual QStringList signalTags(const QString& appSignalId) const = 0;

	[[nodiscard]] virtual bool signalHasTag(Hash signalHash, const QString& tag) const = 0;
	[[nodiscard]] virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const = 0;

	[[nodiscard]] virtual QStringList signalIdsByTag(const QString& tag) const = 0;

	[[nodiscard]] virtual E::SignalType signalType(Hash signalHash, bool* found) const = 0;
	[[nodiscard]] virtual E::SignalType signalType(const QString& appSignalId, bool* found) const = 0;

	[[nodiscard]] virtual QString equipmentToAppSignalId(const QString& equipmentId) const = 0;

	// Setpoints
	//
	[[nodiscard]] virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const = 0;

	// Tags
	//
	[[nodiscard]] virtual QStringList tags() const = 0;
};
