#ifndef SIMTUNINGCONNECTION_H
#define SIMTUNINGCONNECTION_H
#include "../../lib/Tuning/ITuningConnection.h"

class SimTuningConnection : public ITuningConnection
{
public:
	SimTuningConnection() = default;

public:
	virtual bool writeTuningSignal(const QString& appSignalId, const TuningValue& value) override;
	virtual bool writeTuningSignal(const QString& appSignalId, QVariant value) override;
	virtual void applyTuningSignals() override;
};

#endif // SIMTUNINGCONNECTION_H
