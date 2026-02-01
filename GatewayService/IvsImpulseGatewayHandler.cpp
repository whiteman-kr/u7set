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
		Q_UNUSED(request);
	}

	void IvsImpulseHandler::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();

		for(const AppSignalState& state : m_states)
		{
			hashes->emplace(state.hash());
		}
	}

	void IvsImpulseHandler::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();

		for(const AppSignalState& st : m_states)
		{
			if (st.isWorkable() == true && st.requestEvents() == true)
			{
				hashes->emplace(st.hash());
			}
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

	void IvsImpulseHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
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
				const AppSignal* s = m_appSignals.getSignalByID(id);

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
}
