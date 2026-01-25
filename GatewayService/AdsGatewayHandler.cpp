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
		m_softwareInfo(swInfo),
		m_appDataService1(settings.appDataService1.address),
		m_appDataService2(settings.appDataService2.address),
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
		m_adsGatewayServer->updateSignalStates(getStatesReply);
	}

	void AdsGatewayHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
		Q_UNUSED(getStateChangesReply);
		// int statesCount = getStateChangesReply.appsignalstates_size();

		// m_hashesToUpdate.clear();

		// for(int i = 0; i < statesCount; i++)
		// {
		// 	const Proto::AppSignalState& state = getStateChangesReply.appsignalstates(i).curstate();

		// 	Hash hash = state.hash();

		// 	auto it = m_signalsStates.find(hash);

		// 	if (it == m_signalsStates.end())
		// 	{
		// 		continue;
		// 	}

		// 	m_hashesToUpdate.emplace(hash);

		// 	std::list<SignalState>& states = it->second;

		// 	for(SignalState& st : states)
		// 	{
		// 		st.setValue(state.value());
		// 	}
		// }

		// updateRegisters(m_hashesToUpdate);
	}

	bool AdsGatewayHandler::init()
	{
		std::set<Hash> stateHashes;
		std::set<Hash> eventHashes;

		for(const AppSignal* appSignal : m_appSignals)
		{
			TEST_PTR_CONTINUE(appSignal);

			stateHashes.insert(appSignal->hash());
			eventHashes.insert(appSignal->hash());
		}

		m_gateway->setRequiredSignalHashes(stateHashes, eventHashes);

		return true;
	}

	void AdsGatewayHandler::runAppDataSrvClient()
	{
		m_appDataSrvClientThread =
			std::make_unique<AppDataServiceClientThread>( m_softwareInfo,
										   m_appDataService1,
										   m_appDataService2,
										   QString("GatewayService %1").arg(m_softwareInfo.equipmentID()),
										   this, m_log);
		m_appDataSrvClientThread->start();
	}

	void AdsGatewayHandler::stopAppDataSrvClient()
	{
		if (m_appDataSrvClientThread != nullptr)
		{
			m_appDataSrvClientThread->quitAndWait();
			m_appDataSrvClientThread.reset();
		}
	}

	void AdsGatewayHandler::runAdsGatewayServer()
	{
		Q_ASSERT(m_adsGatewayServer == nullptr);

		m_adsGatewayServer = std::make_unique<AdsGatewayServer>(m_gateway->clientRequestIP1(), m_appSignals, m_log);

		m_adsGatewayServer->run();
	}

	void AdsGatewayHandler::stopAdsGatewayServer()
	{
		if (m_adsGatewayServer != nullptr)
		{
			m_adsGatewayServer->stop();
			m_adsGatewayServer.reset();
		}
	}
}
