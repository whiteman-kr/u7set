#pragma once

#include <TestSuiteLib/IInputController.h>

#include <ClientLib/AdsConnection.h>
#include <ClientLib/AppSignalManager.h>


namespace TestSuite
{
	class AdsInputController : public IInputController
	{
	public:
		explicit AdsInputController(ClientLib::AppSignalManager& signalManager, ILogFile* logFile);

		// IInputController implementation
		//
	public:
		virtual bool init(qint64 timeoutMs) override;
		virtual bool shutdown() override;

		virtual bool signalExists(const QString& signalId) const override;
		virtual std::optional<AppSignalParam> signalParam(const QString& appSignalId) const override;
		virtual std::optional<AppSignalState> signalState(const QString& appSignalId) const override;

		virtual bool expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance = 0) const override;

		virtual tl::expected<std::unique_ptr<ITestObserver>, QString> createTestObserver() override;

		// End of IInputController
		//

	public:
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

	private:
		ClientLib::AppSignalManager& m_signalManager;
		::SoftwareInfo m_softwareInfo;
		std::vector<SoftwareEndpoint::AppDataService> m_appDataServices;

		mutable HasLogFile m_appLog;
		ClientLib::AdsConnection m_connection;
	};
} // namespace TestSuite
