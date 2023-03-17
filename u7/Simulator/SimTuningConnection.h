#ifndef SIMTUNINGCONNECTION_H
#define SIMTUNINGCONNECTION_H
#include "../../lib/Tuning/ITuningConnection.h"

class SimTuningConnection : public ITuningConnection
{
public:
	SimTuningConnection();

public:
	virtual bool hasTuningSignal(QString appSignalId) const override;
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) override;
	virtual void applyTuningSignals() override;
};

#endif // SIMTUNINGCONNECTION_H
