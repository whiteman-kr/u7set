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
		prepareRequests();
		runAppDataSrvClient();
		runAdsGatewayServer();
	}

	void AdsGatewayHandler::shutdown()
	{
		stopAdsGatewayServer();
		stopAppDataSrvClient();
		Handler::shutdown();
	}

	void AdsGatewayHandler::onAppDataSrvConnected()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->setConnectedToAppDataSrv(true);
		}

		m_requestIndex = 0;
		m_changesRequestCount = 0;
		m_hasPendingChanges = false;
	}

	void AdsGatewayHandler::onAppDataSrvDisconnected()
	{
		std::lock_guard lg(m_adsGatewayServerMutex);

		if (m_adsGatewayServer)
		{
			m_adsGatewayServer->setConnectedToAppDataSrv(false);
		}
	}

	void AdsGatewayHandler::planNextPreparedRequest(PreparedRequest& request)
	{
		request.clear();

		if (m_requests.empty())
		{
			return;
		}

		if (m_requestIndex == 0)
		{
			m_changesRequestCount = 0;
		}

		if (m_requestIndex < m_requests.size())
		{
			request.setRequest(m_requests[m_requestIndex], 0);
			m_requestIndex++;
			return;
		}

		if (m_hasPendingChanges && m_changesRequestIndex.has_value())
		{
			if (m_changesRequestCount < 10)
			{
				// retry ADS_GET_APP_SIGNAL_STATE_CHANGES
				//
				request.setRequest(m_requests[m_changesRequestIndex.value()], 0);
				m_changesRequestCount++;
				return;
			}

			m_changesRequestCount = 0;
			request.setDelay(10);
			return;
		}

		m_requestIndex = 0;
		request.setDelay(200);
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

	void AdsGatewayHandler::processStateChanges(const Network::GetAppSignalStateChangesReply& getStateChangesReply)
	{
		{
			std::lock_guard lg(m_adsGatewayServerMutex);

			if (m_adsGatewayServer)
			{
				m_adsGatewayServer->processStateChanges(getStateChangesReply);
			}
		}

		m_hasPendingChanges = getStateChangesReply.pendingstatescount() > 0;
	}

	void AdsGatewayHandler::prepareRequests()
	{
		m_requests.clear();
		m_requestIndex = 0;
		m_changesRequestCount = 0;
		m_hasPendingChanges = false;
		m_changesRequestIndex.reset();

		//

		size_t signalsCount = m_appSignals.count();
		size_t partCount = signalsCount / ADS_GET_APP_SIGNAL_STATE_MAX +
						   ((signalsCount % ADS_GET_APP_SIGNAL_STATE_MAX) ? 1 : 0);

		m_requests.reserve(partCount + 1);

		{
			Network::GetAppSignalStateRequest rq;

			for(size_t p = 0; p < partCount; p++)
			{
				rq.Clear();

				for(size_t i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
				{
					size_t index = p * ADS_GET_APP_SIGNAL_STATE_MAX + i;

					if (index >= signalsCount)
					{
						break;
					}

					const AppSignal* appSignal = m_appSignals.getSignalByIndex(index);

					TEST_PTR_CONTINUE(appSignal);

					Hash hash = appSignal->hash();

					rq.add_signalhashes(hash);
				}

				size_t requestSize = rq.ByteSizeLong();

				if (requestSize > Tcp::TCP_MAX_DATA_SIZE)
				{
					Q_ASSERT(false);
					continue;
				}

				PreparedRequest& stateRequest = m_requests.emplace_back(PreparedRequest{});

				stateRequest.ID = ADS_GET_APP_SIGNAL_STATE;

				stateRequest.data.resize(requestSize);

				rq.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(stateRequest.data.data()));
			}
		}

		// last request in m_requests is ADS_GET_APP_SIGNAL_STATE_CHANGES
		//
		{
			Network::GetAppSignalStateChangesRequest rq;

			size_t requestSize = rq.ByteSizeLong();

			PreparedRequest& stateChangesRequest = m_requests.emplace_back(PreparedRequest{});
			m_changesRequestIndex = m_requests.size() - 1;

			stateChangesRequest.ID = ADS_GET_APP_SIGNAL_STATE_CHANGES;

			stateChangesRequest.data.resize(requestSize);

			rq.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(stateChangesRequest.data.data()));
		}
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
