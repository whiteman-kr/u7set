#ifndef ITUNINGTCPCLIENT_H
#define ITUNINGTCPCLIENT_H
#include "TuningSignalState.h"

class ITuningTcpClient
{
public:
	virtual bool writeTuningSignal(QString appSignalId, TuningValue value) = 0;
};


#endif // ITUNINGTCPCLIENT_H
