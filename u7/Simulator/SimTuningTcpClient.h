#ifndef SIMTUNINGTCPCLIENT_H
#define SIMTUNINGTCPCLIENT_H
#include "../../lib/Tuning/ITuningTcpClient.h"

class SimTuningTcpClient : public ITuningTcpClient
{
public:
	SimTuningTcpClient();

public:
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) override;
	virtual void applyTuningSignals() override;
};

#endif // SIMTUNINGTCPCLIENT_H
