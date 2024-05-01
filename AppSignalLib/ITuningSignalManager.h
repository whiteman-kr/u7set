#pragma once

#include "ISignalManager.h"
#include "TuningSignalState.h"
#include "AppSignalParam.h"

class ITuningSignalManager : public ISignalManager
{
public:
	virtual TuningSignalState state(Hash hash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const = 0;

	virtual TuningSignalState state(Hash hash, Hash tuningServiceHash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, Hash tuningServiceHash, bool* found) const = 0;

	virtual void state(const std::vector<Hash>& appSignalHashes, std::vector<TuningSignalState>* result, int* found) const = 0;
	virtual void state(const std::vector<QString>& appSignalIds, std::vector<TuningSignalState>* result, int* found) const = 0;

	virtual QStringList signalIdsByTag(const QString& tag) const = 0;
};

