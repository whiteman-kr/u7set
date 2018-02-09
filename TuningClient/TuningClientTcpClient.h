#pragma once

#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningFilter.h"
#include "../lib/LogFile.h"
#include "../lib/Tuning/TuningLog.h"
#include "UserManager.h"

class TuningClientTcpClient : public TuningTcpClient
{
public:
	TuningClientTcpClient(const SoftwareInfo& softwareInfo,
						  TuningSignalManager* signalManager,
						  Log::LogFile* log,
						  TuningLog::TuningLog* tuningLog,
						  UserManager* userManager);

	bool tuningSourceCounters(const Hash& equipmentHash, TuningFilterCounters* result) const;// REWRITE THIS FUNCTION!!!

	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

	virtual void writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue) override;

private:

	Log::LogFile* m_log = nullptr;

	TuningLog::TuningLog *m_tuningLog = nullptr;

	UserManager* m_userManager = nullptr;

};
