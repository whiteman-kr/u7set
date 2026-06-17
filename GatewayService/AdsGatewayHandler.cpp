#include <string>

#include "AdsGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::AdsGatewayHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	AdsGatewayHandler::AdsGatewayHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 AdsGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(gateway->gatewayID(), swInfo, settings, appSignals, log, logGatewayPackets),
		m_gateway(gateway)
	{
	}

	AdsGatewayHandler::~AdsGatewayHandler()
	{
	}

	void AdsGatewayHandler::run()
	{
		runAdsGatewayServer();
	}

	void AdsGatewayHandler::shutdown()
	{
		stopAdsGatewayServer();
		Handler::shutdown();
	}

	void AdsGatewayHandler::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getRequiredSignalsHashes(hashes);
	}

	void AdsGatewayHandler::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getEventSignalsHashes(hashes);
	}

	void AdsGatewayHandler::runAdsGatewayServer()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		Q_ASSERT(m_asyncAdsGatewayServer == nullptr);

		std::vector<HostAddressPort> listenAddresses;

		m_asyncAdsGatewayServer = std::make_unique<AsyncAdsGatewayServer>(
			m_swInfo,
			m_appSignals,
			std::vector<HostAddressPort>{m_gateway->clientRequestIP1()},
			std::vector<HostAddressPort>{m_settings.appDataService1.address, m_settings.appDataService2.address},
			2,
			m_log,
			"AsyncAdsGatewayServer");
		
		m_asyncAdsGatewayServer->start();
	}

	void AdsGatewayHandler::stopAdsGatewayServer()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_asyncAdsGatewayServer != nullptr)
		{
			m_asyncAdsGatewayServer->stop();
			m_asyncAdsGatewayServer.reset();
		}
	}
}
