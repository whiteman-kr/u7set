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
		Handler(gateway->gatewayID(), swInfo, settings, log, logGatewayPackets),
		m_gateway(gateway),
		m_appSignals(appSignals)
	{
	}

	AdsGatewayHandler::~AdsGatewayHandler()
	{
	}

	void AdsGatewayHandler::run()
	{
		init();
		runAppDataSrvClient();
		runAdsGatewayServer();
	}

	void AdsGatewayHandler::shutdown()
	{
		stopAdsGatewayServer();
		stopAppDataSrvClient();
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

	void AdsGatewayHandler::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->updateSignalStates(getStatesReply);
		}
	}

	void AdsGatewayHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->processStateChanges(getStateChangesReply);
		}
	}

	void AdsGatewayHandler::onAppDataSrvConnected()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->setConnectedToAppDataSrv(true);
		}
	}

	void AdsGatewayHandler::onAppDataSrvDisconnected()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->setConnectedToAppDataSrv(false);
		}
	}

	bool AdsGatewayHandler::init()
	{
		std::set<Hash> hashes;

		for(const AppSignal* appSignal : m_appSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

			hashes.insert(appSignal->hash());
		}

		m_gateway->setRequiredSignalHashes(hashes);

		return true;
	}

	void AdsGatewayHandler::runAdsGatewayServer()
	{
		Q_ASSERT(m_adsGatewayServer == nullptr);

		std::lock_guard lg(m_adsGatewayServerMutex);

		m_adsGatewayServer = std::make_unique<AdsGatewayServer>(m_gateway->clientRequestIP1(), m_appSignals, m_log);
		m_adsGatewayServer->run();
	}

	void AdsGatewayHandler::stopAdsGatewayServer()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer != nullptr)
		{
			m_adsGatewayServer->stop();
			m_adsGatewayServer.reset();
		}
	}
}
