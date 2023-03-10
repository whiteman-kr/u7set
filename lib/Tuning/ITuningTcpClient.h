#pragma once

#include "../../AppSignalLib/TuningValue.h"

class ITuningTcpClient
{
public:
	virtual bool hasTuningSignal(QString appSignalId) const = 0;
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) = 0;
	virtual void applyTuningSignals() = 0;
};
