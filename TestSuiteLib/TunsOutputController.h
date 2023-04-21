#pragma once

#include "IOutputController.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../ClientLib/ITuningLog.h"
#include "../ClientLib/TuningConnection.h"


namespace TestSuite
{
	class TunsOutputController : public IOutputController
	{
	public:
		TunsOutputController(const SoftwareInfo& softwareInfo,
							 const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
							 const QByteArray& signalsFile,
							 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
							 ILogFile* logFile);

	public:
		virtual bool waitForConnection(qint64 timeoutMs) const override;
		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) override;

	private:
		TuningSignalManager m_signalManager;
		mutable HasLogFile m_appLog;

		ClientLib::TuningLogStub m_tuningLogStub;
		ClientLib::TuningConnection m_connection;
	};
}

