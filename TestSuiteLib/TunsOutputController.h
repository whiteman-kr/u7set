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
							 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
							 ILogFile* logFile);

		virtual bool waitForConnection(qint64 timeoutMs) const override;

	private:
		TuningSignalManager m_signalManager;
		mutable HasLogFile m_appLog;

		ClientLib::TuningLogStub m_tuningLogStub;
		ClientLib::TuningConnection m_connection;
	};
}

