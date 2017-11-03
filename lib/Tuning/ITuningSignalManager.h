#pragma once

#include "../lib/Tuning/TuningSignalState.h"
#include "../AppSignal.h"

class ITuningSignalManager
{
public:
	virtual bool signalExists(Hash hash) const = 0;
	virtual bool signalExists(const QString& appSignalId) const = 0;

	virtual AppSignalParam signalParam(Hash hash, bool* found) const = 0;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

	virtual bool signalParam(Hash hash, AppSignalParam* result) const = 0;
	virtual bool signalParam(const QString& appSignalId, AppSignalParam* result) const = 0;

	virtual TuningSignalState state(Hash hash, bool* found) const = 0;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const = 0;
};

