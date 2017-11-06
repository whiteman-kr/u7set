#pragma once

#include "../lib/Tuning/TuningTcpClient.h"

class TuningClientTcpClient : public TuningTcpClient
{
public:
	TuningClientTcpClient(QString equipmentID,
						  int majorVersion,
						  int minorVersion,
						  int commitNo,
						  TuningSignalManager* signalManager);

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

};
