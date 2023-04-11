#pragma once

#include "IInputController.h"
#include "../ClientLib/AppSignalManager.h"
#include "../ClientLib/AdsConnection.h"


namespace TestSuite
{
	class AdsInputController : public IInputController
	{
	public:
		AdsInputController(ClientLib::AppSignalManager& signalManager,
						   const SoftwareInfo& softwareInfo,
						   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
						   ILogFile* logFile);

		virtual bool waitForConnection(qint64 timeoutMs) const override;

	private:
		ClientLib::AppSignalManager& m_signalManager;
		mutable HasLogFile m_appLog;
		ClientLib::AdsConnection m_connection;
	};
}

