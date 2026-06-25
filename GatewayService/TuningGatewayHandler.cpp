#include <string>

#include "TuningGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::TuningGatewayHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	TuningGatewayHandler::TuningGatewayHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 TuningGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(gateway->gatewayID(), swInfo, settings, appSignals, log, logGatewayPackets),
		m_gateway(gateway)
	{
	}

	TuningGatewayHandler::~TuningGatewayHandler()
	{
	}

	void TuningGatewayHandler::run()
	{
		runTuningGatewayServer();
	}

	void TuningGatewayHandler::shutdown()
	{
		stopTuningGatewayServer();
		Handler::shutdown();
	}

	void TuningGatewayHandler::runTuningGatewayServer()
	{
		std::lock_guard lg(m_tunGatewayServerMutex);

		Q_ASSERT(m_asyncTunGatewayServer == nullptr);

		std::vector<HostAddressPort> listenAddresses;

		m_asyncTunGatewayServer = std::make_unique<AsyncTuningGatewayServer>(
											m_swInfo, m_appSignals,
											std::vector<HostAddressPort>{m_gateway->clientRequestIP1()},
											std::vector<HostAddressPort>{m_settings.tuningService1.address, m_settings.tuningService2.address},
											2, m_log, "AsyncTuningGatewayServer");
		m_asyncTunGatewayServer->start();

	}

	void TuningGatewayHandler::stopTuningGatewayServer()
	{
		std::lock_guard lg(m_tunGatewayServerMutex);

		if (m_asyncTunGatewayServer != nullptr)
		{
			m_asyncTunGatewayServer->stop();
			m_asyncTunGatewayServer.reset();
		}
	}
}
