#include "AppDataServiceClient.h"
#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	AppDataServiceClient::AppDataServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2,
											   const QString& clientDescription,
											   IvsImpulseHandler& handler,
											   CircularLoggerShared logger) :
		Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, clientDescription),
		m_lists(handler.m_lists),
		m_states(handler.m_states),
		m_hashToLists(handler.m_hashToLists),
		m_signalStatesUpdated(handler.m_signalStatesUpdated),
		m_timer(this)
	{
		setLogger(logger);
	}

	void AppDataServiceClient::onClientThreadStarted()
	{
		m_getStatesRequest.mutable_signalhashes()->Reserve(TO_INT(m_states.size()));

		for(const AppSignalState& st : m_states)
		{
			m_getStatesRequest.add_signalhashes(st.hash());
		}

		m_timer.setTimerType(Qt::PreciseTimer);
		m_timer.setInterval(200);
		m_timer.setSingleShot(false);

		connect(&m_timer, &QTimer::timeout, this, &AppDataServiceClient::onTimer);
	}

	void AppDataServiceClient::onClientThreadFinished()
	{
		m_timer.stop();
	}

	void AppDataServiceClient::onConnection()
	{
		Tcp::Client::onConnection();

		m_timer.start();

		//

		Network::GatewayGetAppSignalStateChangesRequest initialRequest;

		std::set<Hash> eventHashes;

		for(const AppSignalState& st : m_states)
		{
			if (st.isWorkable() == true && st.requestEvents() == true)
			{
				eventHashes.insert(st.hash());
			}
		}

		initialRequest.mutable_signalshashes()->Reserve(TO_INT(eventHashes.size()));

		for(Hash h : eventHashes)
		{
			initialRequest.add_signalshashes(h);
		}

		sendRequest(ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES, initialRequest);
	}

	void AppDataServiceClient::onDisconnection()
	{
		Tcp::Client::onDisconnection();

		m_timer.stop();
	}

	void AppDataServiceClient::onTimer()
	{
		if (isClearToSendRequest() == false)
		{
			return;
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE_CONST_SIZE, m_getStatesRequest);
	}

	void AppDataServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case ADS_GET_APP_SIGNAL_STATE_CONST_SIZE:
			onGetAppSignalStateReply(replyData, replyDataSize);
			break;

		case ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES:
			onGatewayGetAppSignalStateChangesReply(replyData, replyDataSize);
			break;

		default:
			Q_ASSERT(false);
		}
	}

	void AppDataServiceClient::onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize)
	{
		bool result = m_getStatesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		int replyStatesSize = m_getStatesReply.appsignalstates_size();

		if (replyStatesSize != TO_INT(m_states.size()))
		{
			Q_ASSERT(false);
			return;
		}

		for(int i = 0; i < replyStatesSize; i++)
		{
			m_states[i].updateState(m_getStatesReply.appsignalstates(i));
		}

		m_signalStatesUpdated = true;

		if (m_getStatesReply.gatewaystatechangesqueuesize() != 0)
		{
			sendRequest(ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES, m_gwGetStateChangesRequest);
		}
	}

	void AppDataServiceClient::onGatewayGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize)
	{
		auto& reply = m_gwGetStateChangesReply;

		bool result = reply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		int statesCount = reply.appsignalstates_size();

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
			const ::Network::GatewayAppSignalState& protoState = reply.appsignalstates(i);

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

		QThread* thread = QThread::currentThread();

		for(IvsImpulseListInfoShared& list : m_lists)
		{
			list->stateChangesMutex.lock(thread);

			list->stateChangesToWrite.swap(list->stateChangesToRead);

			list->stateChangesMutex.unlock(thread);
		}

		emit sendStateChanges();
	}
}
