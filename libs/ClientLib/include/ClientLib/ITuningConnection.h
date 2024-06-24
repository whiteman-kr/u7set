#pragma once

#include "../../AppSignalLib/TuningValue.h"

class ITuningConnection
{
public:
	virtual ~ITuningConnection() = default;

	virtual bool writeTuningSignal(const QString& appSignalId, const TuningValue& value) = 0;
	virtual bool writeTuningSignal(const QString& appSignalId, QVariant value) = 0;
	virtual void applyTuningSignals() = 0;
};
