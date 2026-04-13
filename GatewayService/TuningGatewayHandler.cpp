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

		Q_ASSERT(m_tunGatewayServer == nullptr);

		m_tunGatewayServer = std::make_unique<TuningGatewayServer>(m_swInfo,
																   m_gateway->clientRequestIP1(),
																   m_settings.tuningService1.address,
																   m_settings.tuningService2.address,
																   m_appSignals, m_log);
		m_tunGatewayServer->start();
	}

	void TuningGatewayHandler::stopTuningGatewayServer()
	{
		std::lock_guard lg(m_tunGatewayServerMutex);

		if (m_tunGatewayServer != nullptr)
		{
			m_tunGatewayServer->stop();
			m_tunGatewayServer.reset();
		}
	}
}
