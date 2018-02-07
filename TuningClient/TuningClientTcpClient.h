#pragma once

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningFilter.h"

class TuningClientTcpClient : public TuningTcpClient
{
public:
	TuningClientTcpClient(const SoftwareInfo& softwareInfo,
						  TuningSignalManager* signalManager);

	bool tuningSourceCounters(const QString& equipmentID, TuningFilterCounters* result) const;// REWRITE THIS FUNCTION!!!

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

};
