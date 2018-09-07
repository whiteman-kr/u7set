#include "AppDataService.h"

#include "RtTrendsServer.h"

namespace RtTrends
{

	// -----------------------------------------------------------------------------------------------
	//
	// Struct RtTrends::SignalStatesQueue implementation
	//
	// -----------------------------------------------------------------------------------------------

	SignalStatesQueue::SignalStatesQueue(int queueSize) :
		m_clientQueue(queueSize)
		//m_dbQueue(queueSize)
	{
	}

	void SignalStatesQueue::push(qint64 archiveID, const SimpleAppSignalState& state)
	{
		SignalState ss;

		ss.state = state;
		ss.archiveID = archiveID;

		m_clientQueue.push(ss);
		//m_dbQueue.push(&ss);
	}

	// -----------------------------------------------------------------------------------------------
	//
	// Class RtTrends::Session implementation
	//
	// -----------------------------------------------------------------------------------------------

	std::atomic<int> Session::m_globalID = 1;

	Session::Session(AppDataServiceWorker& service) :
		m_id(m_globalID.fetch_add(1)),
		m_signalStates(service.signalStates()),
		m_signalToSources(service.signalsToSources())
	{
	}

	Session::~Session()
	{
		for(SignalStatesQueue* queue : m_trackedSignals)
		{
			delete queue;
		}
	}

	bool Session::containsSignal(Hash signalHash)
	{
		return m_trackedSignals.contains(signalHash);
	}

	bool Session::appendSignal(Hash signalHash)
	{
		if (m_trackedSignals.contains(signalHash) == true)
		{
			assert(false);
			return false;
		}

		SignalStatesQueue* statesQueue = new SignalStatesQueue(1000);		// 5 sec queue for min samplePeriod

		m_trackedSignals.insert(signalHash, statesQueue);

		return true;
	}

	bool Session::deleteSignal(Hash signalHash)
	{
		SignalStatesQueue* queue = m_trackedSignals.value(signalHash, nullptr);

		if (queue == nullptr)
		{
			assert(false);
			return false;
		}

		m_trackedSignals.remove(signalHash);

		delete queue;

		return true;
	}

	void Session::pushSignalState(Hash signalHash, const SimpleAppSignalState& state)
	{
		SignalStatesQueue* queue = m_trackedSignals.value(signalHash, nullptr);

		if (queue == nullptr)
		{
			assert(false);
			return;
		}

		qint64 archiveID = m_archiveID.fetch_add(1);

		queue->push(archiveID, state);
	}

	// -----------------------------------------------------------------------------------------------
	//
	// Class RtTrends::Server implementation
	//
	// -----------------------------------------------------------------------------------------------


	Server::Server(AppDataServiceWorker& appDataService) :
		Tcp::Server(appDataService.softwareInfo()),
		m_appDataService(appDataService),
		m_signalsToSources(appDataService.signalsToSources()),
		m_signalStates(appDataService.signalStates()),
		m_log(appDataService.logger()),
		m_session(std::make_shared<Session>(appDataService))
	{
	}

	Tcp::Server* Server::getNewInstance()
	{
		return new Server(m_appDataService);
	}


	void Server::onServerThreadStarted()
	{
		LOG_MSG(m_log, QString("RtTrendsServer(%1) started, сlientID = %2").arg(m_session->id()).arg(connectedSoftwareInfo().equipmentID()));
	}

	void Server::onServerThreadFinished()
	{
		LOG_MSG(m_log, QString("RtTrendsServer(%1) finished").arg(m_session->id()).arg(connectedSoftwareInfo().equipmentID()));
	}

	void Server::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		switch(requestID)
		{
		case RT_TRENDS_MANAGEMENT:
			onRtTrendsManagementRequest(requestData, requestDataSize);
			break;

		case RT_TRENDS_GET_STATE_CHANGES:
			onRtTrendsGetStateChangesRequest(requestData, requestDataSize);
			break;

		default:
			assert(false);
		}

	}

	void Server::onRtTrendsManagementRequest(const char* requestData, quint32 requestDataSize)
	{
		bool result = m_rtTrendsManagementRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

		m_rtTrendsManagementReply.Clear();

		if (result == false)
		{
			m_rtTrendsManagementReply.set_error(TO_INT(NetworkError::ParseRequestError));
			m_rtTrendsManagementReply.set_errorstring("Network::RtTrendsManagementRequest parsing error");
			sendReply(m_rtTrendsManagementReply);
			return;
		}

		setSamplePeriod(static_cast<E::RtTrendsSamplePeriod>(m_rtTrendsManagementRequest.sampleperiod()));

		appendTrackedSignals(m_rtTrendsManagementRequest);

		removeTrackedSignals(m_rtTrendsManagementRequest);

		sendReply(m_rtTrendsManagementReply);
	}

	void Server::setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod)
	{
		if (newSamplePeriod == m_session->samplePeriod())
		{
			return;
		}

		m_session->setSamplePeriod(newSamplePeriod);
	}

	void Server::appendTrackedSignals(const Network::RtTrendsManagementRequest& request)
	{
		int size = request.appendsignalhashes_size();

		E::RtTrendsSamplePeriod samplePeriod = static_cast<E::RtTrendsSamplePeriod>(m_rtTrendsManagementRequest.sampleperiod());

		for(int i = 0; i < size; i++)
		{
			Hash hashToAppend = request.appendsignalhashes(i);

			appendTrackedSignal(hashToAppend, samplePeriod);
		}
	}

	bool Server::appendTrackedSignal(Hash signalHash, E::RtTrendsSamplePeriod samplePeriod)
	{
		if (m_session->containsSignal(signalHash) == true)
		{
			return true;
		}

		AppDataSourceShared source = m_signalsToSources.value(signalHash, nullptr);

		if (source == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		int lmWorkcycle_ms = source->lmWorkcycle_ms();

		int samplePeriodCounter = getSamplePeriodCounter(samplePeriod, lmWorkcycle_ms);

		AppSignalStateEx* state = m_signalStates.getStateByHash(signalHash);

		if (state == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		m_session->appendSignal(signalHash);

		state->appendRtSession(signalHash, QThread::currentThread(), m_session, samplePeriodCounter);

		return true;
	}

	void Server::removeTrackedSignals(const Network::RtTrendsManagementRequest& request)
	{
		int size = request.deletesignalhashes_size();

		for(int i = 0; i < size; i++)
		{
			Hash hashToDelete = request.deletesignalhashes(i);

			removeTrackedSignal(hashToDelete);
		}
	}

	bool Server::removeTrackedSignal(Hash signalHash)
	{
		if (m_session->containsSignal(signalHash) == false)
		{
			return false;
		}

		AppSignalStateEx* state = m_signalStates.getStateByHash(signalHash);

		if (state == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		state->removeRtSession(signalHash, QThread::currentThread(), m_session);

		m_session->deleteSignal(signalHash);

		return true;
	}

	void Server::onRtTrendsGetStateChangesRequest(const char* requestData, quint32 requestDataSize)
	{
		bool result = m_rtTrendsGetStateChangesRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

		m_rtTrendsGetStateChangesReply.Clear();

		if (result == false)
		{
			m_rtTrendsGetStateChangesReply.set_error(TO_INT(NetworkError::ParseRequestError));
			m_rtTrendsGetStateChangesReply.set_errorstring("Network::RtTrendsGetStateChangesRequest parsing error");
			sendReply(m_rtTrendsGetStateChangesReply);
			return;
		}

		assert(false);	// do real work!

		sendReply(m_rtTrendsGetStateChangesReply);
	}


	// -----------------------------------------------------------------------------------------------
	//
	// Class RtTrends::ServerThread implementation
	//
	// -----------------------------------------------------------------------------------------------

	ServerThread::ServerThread(	const HostAddressPort& listenAddressPort,
								AppDataServiceWorker& appDataService) :
		Tcp::ServerThread(listenAddressPort,
						  new Server(appDataService),
						  appDataService.logger())
	{
	}
}
