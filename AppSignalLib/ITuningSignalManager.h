#pragma once

#include <span>
#include <vector>

#include "AppSignalParam.h"
#include "TuningSignalState.h"

class ITuningSignalManager
{
public:
	virtual bool signalExists(Hash hash) const = 0;
	virtual bool signalExists(const QString& appSignalId) const = 0;
	virtual bool signalsExist(const QStringList& signalIds) const = 0;

	virtual AppSignalParam signalParam(Hash hash, bool* found) const = 0;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

	virtual bool signalParam(Hash hash, AppSignalParam* result) const = 0;
	virtual bool signalParam(const QString& appSignalId, AppSignalParam* result) const = 0;

	virtual TuningSignalState state(Hash hash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const = 0;

	virtual TuningSignalState state(Hash hash, Hash tuningServiceHash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, Hash tuningServiceHash, bool* found) const = 0;

	virtual void state(std::span<const Hash> appSignalHashes, std::vector<TuningSignalState>* result, int* found) const = 0;
	virtual void state(std::span<const QString> appSignalIds, std::vector<TuningSignalState>* result, int* found) const = 0;

	virtual QStringList signalIdsByTag(const QString& tag) const = 0;
};
