#pragma once

#include "../../AppSignalLib/TuningValue.h"

class ITuningConnection
{
public:
	virtual ~ITuningConnection() = default;

	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) = 0;
	virtual void applyTuningSignals() = 0;
};
