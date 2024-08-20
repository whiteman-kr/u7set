#pragma once

#include <span>
#include <vector>

#include "AppSignalParam.h"
#include "ISignalManager.h"
#include "TuningSignalState.h"

class ITuningSignalManager : public ISignalManager
{
public:
	virtual TuningSignalState state(Hash hash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const = 0;

	virtual TuningSignalState state(Hash hash, Hash tuningServiceHash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, Hash tuningServiceHash, bool* found) const = 0;

	virtual void state(std::span<const Hash> appSignalHashes, std::vector<TuningSignalState>* result, int* found) const = 0;
	virtual void state(std::span<const QString> appSignalIds, std::vector<TuningSignalState>* result, int* found) const = 0;

	virtual QStringList signalIdsByTag(const QString& tag) const = 0;
};
