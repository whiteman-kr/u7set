#pragma once
#include <vector>

#include "../AppSignalLib/TuningSignalState.h"


class ITuningSignalUpdater
{
public:
	virtual ~ITuningSignalUpdater() = default;

	// Reset all signal params and states.
	//
	virtual void reset() = 0;

	// Get list of tuning signal hashes for LM.
	//
	virtual std::vector<Hash> signalHashes(const std::vector<Hash> lmEquipmentIdHashes) const = 0;

	// Invalidate all signal states by Tuning Service tuningServiceHash.
	//
	virtual void invalidateSignalStates(Hash tuningServiceHash) = 0;

	// Set signal states by sources.
	//
	virtual void setState(const TuningSignalState& state, Hash tuningServiceHash) = 0;
	virtual void setStates(const std::vector<TuningSignalState>& states, Hash tuningServiceHash) = 0;

protected:
	// This function must notify (emit signal?) that signal params where updated.
	// This signal is out of scope of this interface, it is up to implemenation which signal to emit and how to use it.
	//
	virtual void notifySignalParamsUpdated() = 0;
};
