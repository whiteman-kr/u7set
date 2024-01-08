#ifndef TUNINGCONNECTIONSTUB_H
#define TUNINGCONNECTIONSTUB_H
#include "../lib/Tuning/ITuningConnection.h"
#include "../lib/Tuning/ITuningAuthorization.h"

class TuningConnectionStub : public ITuningConnection
{
public:
	TuningConnectionStub() = default;

public:
	virtual bool writeTuningSignal(const QString& appSignalId, const TuningValue& value) override;
	virtual bool writeTuningSignal(const QString& appSignalId, QVariant value) override;
	virtual void applyTuningSignals() override;
};

#endif // TUNINGCONNECTIONSTUB_H
