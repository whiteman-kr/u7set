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

		m_appDataServiceClientThread =
				new AppDataServiceClientThread( m_softwareInfo,
												m_appDataService1,
												m_appDataService2,
												QString("GatewayService %1").arg(m_softwareInfo.equipmentID()),
												this, m_log);
		m_appDataServiceClientThread->start();

		runAdsGatewayServer();
	}

	void AdsGatewayHandler::shutdown()
	{
		stopAdsGatewayServer();

		if (m_appDataServiceClientThread != nullptr)
		{
			m_appDataServiceClientThread->quitAndWait();
			delete m_appDataServiceClientThread;
			m_appDataServiceClientThread = nullptr;
		}

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
		// int statesCount = getStatesReply.appsignalstates_size();

		// for(int i = 0; i < statesCount; i++)
		// {
		// 	const Proto::AppSignalState& state = getStatesReply.appsignalstates(i);

		// 	Hash hash = state.hash();

		// 	auto it = m_signalsStates.find(hash);

		// 	if (it == m_signalsStates.end())
		// 	{
		// 		continue;
		// 	}

		// 	std::list<SignalState>& states = it->second;

		// 	for(SignalState& st : states)
		// 	{
		// 		st.setValue(state.value());
		// 	}
		// }

		// updateAllRegisters();
	}

	void AdsGatewayHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
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
		// const std::map<Address16, ModbusSlaveGateway::ModbusSignal>& mbSignals = m_gateway->modbusSignals();

		// auto itLast = mbSignals.rbegin();

		// if (itLast == mbSignals.rend())
		// {
		// 	return true;
		// }

		// Address16 maxAddr = itLast->first;
		// const ModbusSlaveGateway::ModbusSignal& mbs = itLast->second;

		// const ModbusFormat lastSignalFormat = mbs.format;

		// maxAddr.addWord(lastSignalFormat.registersCount());			// maxAddr here is +1 to real registers count, it is ok

		// m_regsMutex.lock();

		// m_registers.clear();
		// m_registers.resize(maxAddr.offset(), 0);

		// m_regsMutex.unlock();

		// m_signalsStates.clear();

		// for(const auto& [addr16, mbSignal] : mbSignals)
		// {
		// 	Hash hash = calcHash(mbSignal.signalID);

		// 	auto it = m_signalsStates.find(hash);

		// 	if (it == m_signalsStates.end())
		// 	{
		// 		auto [newIt, b] = m_signalsStates.emplace(hash, std::list<SignalState>{});

		// 		it = newIt;
		// 	}

		// 	it->second.emplace_back(mbSignal.format, mbSignal.addr, mbSignal.isConst, mbSignal.constValue);
		// }

		// updateAllRegisters();		// to init registers values

		return true;
	}

	void AdsGatewayHandler::runAdsGatewayServer()
	{
		Q_ASSERT(m_adsGatewayServer == nullptr);

		m_adsGatewayServer = new AdsGatewayServer(m_gateway->clientRequestIP1());

		m_adsGatewayServer->run();
	}

	void AdsGatewayHandler::stopAdsGatewayServer()
	{
		if (m_adsGatewayServer != nullptr)
		{
			m_adsGatewayServer->stop();
			delete m_adsGatewayServer;
			m_adsGatewayServer = nullptr;
		}
	}

}
