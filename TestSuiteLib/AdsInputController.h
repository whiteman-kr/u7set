#pragma once

#include "IInputController.h"

#include <ClientLib/AppSignalManager.h>
#include <ClientLib/AdsConnection.h>


namespace TestSuite
{
	class AdsInputController : public IInputController
	{
	public:
		AdsInputController(ClientLib::AppSignalManager& signalManager,
						   const SoftwareInfo& softwareInfo,
						   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
						   ILogFile* logFile);

		// IInputController implementation
		//
	public:
		virtual bool waitForConnection(qint64 timeoutMs) const override;

		virtual bool signalExists(const QString& signalId) const override;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;
		virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;

		virtual bool expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance = 0) const override;

		// End of IInputController
		//

	private:
		ClientLib::AppSignalManager& m_signalManager;
		mutable HasLogFile m_appLog;
		ClientLib::AdsConnection m_connection;
	};
}

