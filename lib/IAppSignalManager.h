#pragma once

#include <vector>
#include "../AppSignalLib/AppSignalParam.h"

class Comparator;


class IAppSignalManager
{
public:
	// AppSignals
	//
	virtual std::vector<AppSignalParam> signalList() const = 0;

	virtual bool signalExists(Hash hash) const = 0;
	virtual bool signalExists(const QString& appSignalId) const = 0;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const = 0;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const = 0;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const = 0;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const = 0;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const = 0;

	virtual QStringList signalTags(Hash signalHash) const = 0;
	virtual QStringList signalTags(const QString& appSignalId) const = 0;

	virtual bool signalHasTag(Hash signalHash, const QString& tag) const = 0;
	virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const = 0;

	virtual E::SignalType signalType(Hash signalHash, bool* found) const = 0;
	virtual E::SignalType signalType(const QString& appSignalId, bool* found) const = 0;

	virtual QString equipmentToAppSiganlId(const QString& equipmentId) const = 0;

	// Setpoints
	//
	virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const = 0;
};

