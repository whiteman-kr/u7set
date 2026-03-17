#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::IvsImpulseHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	IvsImpulseHandler::IvsImpulseHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 IvsImpulseGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(gateway->gatewayID(), swInfo, settings, appSignals, log, logGatewayPackets),
		m_gateway(gateway)
	{
	}

	IvsImpulseHandler::~IvsImpulseHandler()
	{
	}

	void IvsImpulseHandler::run()
	{
		init();

		prepareRequests();

		runAppDataSrvClient();

		m_ivsImpulseCommThread = new IvsImpulseCommThread(*this);

		m_ivsImpulseCommThread->connect(appDataServiceClient());

		m_ivsImpulseCommThread->start();
	}

	void IvsImpulseHandler::shutdown()
	{
		if (m_ivsImpulseCommThread != nullptr)
		{
			m_ivsImpulseCommThread->quitAndWait();
			delete m_ivsImpulseCommThread;
			m_ivsImpulseCommThread = nullptr;
		}

		stopAppDataSrvClient();

		Handler::shutdown();
	}

	void IvsImpulseHandler::onAppDataSrvConnected()
	{
	}

	void IvsImpulseHandler::onAppDataSrvDisconnected()
	{
	}

	void IvsImpulseHandler::planNextPreparedRequest(PreparedRequest& request)
	{
		request.clear();

		if (m_requests.empty())
		{
			request.setDelay(500);
			return;
		}

		if (m_requestIndex >= m_requests.size())
		{
			m_requestIndex = 0;
			request.setDelay(200);
			return;
		}

		if (m_requestIndex < m_requests.size())
		{
			request.setRequest(m_requests[m_requestIndex], 0);
			m_requestIndex++;
		}
	}

	void IvsImpulseHandler::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
	{
		int replyStatesSize = getStatesReply.appsignalstates_size();

		for(int i = 0; i < replyStatesSize; i++)
		{
			const Proto::AppSignalState& appSignalState = getStatesReply.appsignalstates(i);

			auto it = m_hashToStatesIndexes.find(appSignalState.hash());

			if (it == m_hashToStatesIndexes.end())
			{
				Q_ASSERT(false);
				continue;
			}

			const std::vector<int>& statesIndexes = it->second;

			int statesCount = TO_INT(statesIndexes.size());

			for(int si = 0; si < statesCount; si++)
			{
				m_states[statesIndexes[si]].updateState(appSignalState);
			}
		}

		m_signalStatesUpdated = true;
	}

	void IvsImpulseHandler::processGatewayStateChanges(const Network::GetGatewayAppSignalStateChangesReply& getStateChangesReply)
	{
		int statesCount = getStateChangesReply.appsignalstates_size();

		if (statesCount == 0)
		{
			return;
		}

		for(auto& list : m_lists)
		{
			list->stateChangesToWrite.clear();
		}

		GatewayAppSignalState state;

		for(int i = 0; i < statesCount; i++)
		{
			const ::Network::GatewayAppSignalState& protoState = getStateChangesReply.appsignalstates(i);

			state.loadFromProto(protoState);

			Q_ASSERT(state.prevState.hash == state.curState.hash);

			auto it = m_hashToLists.find(state.prevState.hash);

			if (it == m_hashToLists.end())
			{
				Q_ASSERT(false);
				continue;
			}

			const std::set<IvsImpulseListInfoShared>& lists = it->second;

			for(const IvsImpulseListInfoShared& list : lists)
			{
				list->stateChangesToWrite.push_back(state);
			}
		}

		for(IvsImpulseListInfoShared& list : m_lists)
		{
			list->stateChangesMutex.lock();

			list->stateChangesToWrite.swap(list->stateChangesToRead);

			list->stateChangesMutex.unlock();
		}
	}

	bool IvsImpulseHandler::init()
	{
		m_lists.clear();
		m_states.clear();

		int signalsCount = m_gateway->signalsCount();

		m_states.reserve(signalsCount);

		const SignalLists& lists = m_gateway->signalLists();

		int signalStateIndex = 0;

		for(SignalListShared sl : lists)
		{
			TEST_PTR_CONTINUE(sl);

			IvsImpulseSignalListShared ivsList = std::dynamic_pointer_cast<IvsImpulseSignalList>(sl);

			TEST_PTR_CONTINUE(ivsList);

			IvsImpulseListInfoShared li = std::make_shared<IvsImpulseListInfo>();

			li->info = ivsList;
			li->startIndex = signalStateIndex;

			const auto& ids = ivsList->signalIDs();

			// signal index in list, numbered from 1
			//
			int listIndex = 1;

			for(const QString& id : ids)
			{
				const AppSignal* s = m_appSignals.getByAppSignalID(id);

				if (s != nullptr)
				{
					Hash h = calcHash(id);

					//

					int stateIndex = TO_INT(m_states.size());

					auto map_it = m_hashToStatesIndexes.find(h);

					if (map_it == m_hashToStatesIndexes.end())
					{
						auto [new_it, b] = m_hashToStatesIndexes.emplace(h, std::vector<int>{});
						map_it = new_it;
					}

					std::vector<int>& statesIndexes = map_it->second;

					statesIndexes.push_back(stateIndex);

					//

					AppSignalState& newState = m_states.emplace_back(h, ivsList->sendEvents());

					newState.setListIndex(listIndex);

					auto it = li->hashToListIndexes.find(h);

					if (it == li->hashToListIndexes.end())
					{
						li->hashToListIndexes.insert({h, {listIndex}});
					}
					else
					{
						it->second.push_back(listIndex);
					}
				}
				else
				{
					m_states.emplace_back(0, false);		// init as NOT workable
				}

				listIndex++;

				signalStateIndex++;
			}

			li->size = signalStateIndex - li->startIndex;

			m_lists.push_back(li);
		}

		//

		for(IvsImpulseListInfoShared& list : m_lists)
		{
			const auto& ids = list->info->signalIDs();

			for(const QString& id : ids)
			{
				Hash h = calcHash(id);

				auto it = m_hashToLists.find(h);

				if (it == m_hashToLists.end())
				{
					auto p = m_hashToLists.emplace(h, std::set<IvsImpulseListInfoShared>());

					it = p.first;
				}

				it->second.insert(list);
			}
		}

		return true;
	}

	void IvsImpulseHandler::prepareRequests()
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

		m_requests.reserve(partCount * 2);

		{
			Network::GetAppSignalStateRequest rq;
			Network::GetGatewayAppSignalStateChangesRequest chRq;

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
					chRq.add_signalshashes(hash);
				}

				{
					size_t requestSize = rq.ByteSizeLong();

					if (requestSize > Tcp::TCP_MAX_DATA_SIZE)
					{
						Q_ASSERT(false);
						continue;
					}

					PreparedRequest& stateRequest = m_requests.emplace_back(PreparedRequest{});

					stateRequest.ID = ADS_GET_APP_SIGNAL_STATE_CONST_SIZE;

					stateRequest.data.resize(requestSize);

					rq.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(stateRequest.data.data()));
				}

				{
					size_t requestSize = chRq.ByteSizeLong();

					if (requestSize > Tcp::TCP_MAX_DATA_SIZE)
					{
						Q_ASSERT(false);
						continue;
					}

					PreparedRequest& stateChangesRequest = m_requests.emplace_back(PreparedRequest{});

					stateChangesRequest.ID = ADS_GET_GATEWAY_APP_SIGNAL_STATE_CHANGES;

					stateChangesRequest.data.resize(requestSize);

					chRq.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(stateChangesRequest.data.data()));
				}
			}
		}
	}
}
