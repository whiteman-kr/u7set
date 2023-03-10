#pragma once

#include "../lib/Tuning/TuningFilter.h"
#include "../lib/Tuning/TuningLog.h"
#include "../UtilsLib/LogFile.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../ClientLib/TuningUserManager.h"


class TuningClientTcpClient : public ClientLib::TuningTcpClient, public TcpClientStatistics
{
	Q_OBJECT
public:
	TuningClientTcpClient(const SoftwareInfo& softwareInfo,
						  const QString& tuningServiceId,
						  int singleLmControlMode,
						  TuningSignalManager& signalManager,
						  Log::LogFile* log,
						  TuningLog::TuningLog* tuningLog,
						  ClientLib::TuningUserManager& userManager);

	virtual void writeLogAlert(const QString& message) override;
	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

	virtual void writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue) override;
	virtual void writeLogSignalChange(const QString& message) override;

	int sourceErrorCount() const;
	int sourceErrorCount(Hash equipmentHash) const;

	int sourceSorCount(bool* sorActive, bool* sorValid) const;
	int sourceSorCount(Hash equipmentHash, bool* sorActive, bool* sorValid) const;

	QString getStateToolTip() const;

	std::vector<Hash> getProcessedHashes(const std::vector<Hash>& hashes);	// Returns hashes that are processed by this client among specified hashes

private:
	Log::LogFile* m_log = nullptr;

	TuningLog::TuningLog* m_tuningLog = nullptr;

	ClientLib::TuningUserManager& m_userManager;
};
