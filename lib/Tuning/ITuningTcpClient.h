#pragma once

#include "TuningSignalState.h"

class ITuningTcpClient
{
public:
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) = 0;
	virtual void applyTuningSignals() = 0;
};


