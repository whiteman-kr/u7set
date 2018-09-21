#include "AppDataService.h"

#include "RtTrendsServer.h"

namespace RtTrends
{

	// -----------------------------------------------------------------------------------------------
	//
	// Struct RtTrends::SignalStatesQueue implementation
	//
	// -----------------------------------------------------------------------------------------------

	SignalStatesQueue::SignalStatesQueue(Hash signalHash, int queueSize) :
		m_signalHash(signalHash),
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

	std::atomic<int> Session::m_globalID = 0;

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

		SignalStatesQueue* statesQueue = new SignalStatesQueue(signalHash, 1000);		// 5 sec queue for min samplePeriod

		m_trackedSignals.insert(signalHash, statesQueue);

		return true;
	}

	bool Session::deleteSignal(Hash signalHash)
	{
		SignalStatesQueue* queue = m_trackedSignals.value(signalHash, nullptr);

		m_trackedSignals.remove(signalHash);

		if (queue != nullptr)
		{
			delete queue;
		}
		else
		{
			assert(false);
		}

		return true;
	}

	void Session::pushSignalState(Hash signalHash, const SimpleAppSignalState& state)
	{
		SignalStatesQueue* queue = m_trackedSignals.value(signalHash, nullptr);

		TEST_PTR_RETURN(queue);

		qint64 archiveID = m_archiveID.fetch_add(1);

		queue->push(archiveID, state);
	}

	void Session::getTrackedSignalHashes(QVector<Hash>* hashes)
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();

		*hashes = QVector<Hash>::fromList(m_trackedSignals.uniqueKeys());
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
		DEBUG_LOG_MSG(m_log, QString("RtTrendsServer(%1) started").arg(m_session->id()));
	}

	void Server::onServerThreadFinished()
	{
		DEBUG_LOG_MSG(m_log, QString("RtTrendsServer(%1) finished").arg(m_session->id()));
	}

	void Server::onConnectedSoftwareInfoChanged()
	{
		DEBUG_LOG_MSG(m_log, QString("RtTrendsServer(%1) clientID = %2").arg(m_session->id()).arg(connectedSoftwareInfo().equipmentID()));
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

		m_rtTrendsManagementReply.set_sampleperiod(static_cast<int>(m_session->samplePeriod()));

		QVector<Hash> trakedSignals;

		m_session->getTrackedSignalHashes(&trakedSignals);

		for(Hash hash : trakedSignals)
		{
			m_rtTrendsManagementReply.add_trackedsignalhashes(hash);
		}

		sendReply(m_rtTrendsManagementReply);
	}

	void Server::setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod)
	{
		if (newSamplePeriod == m_session->samplePeriod())
		{
			return;
		}

		qDebug() << C_STR(QString("Session %1 NEW sample period %2").arg(m_session->id()).arg(static_cast<int>(newSamplePeriod)));

		m_session->setSamplePeriod(newSamplePeriod);

		QThread* thisThread = QThread::currentThread();

		QVector<Hash> trackedSignalHashes;

		m_session->getTrackedSignalHashes(&trackedSignalHashes);

		for(Hash signalHash : trackedSignalHashes)
		{
			AppDataSourceShared source = m_signalsToSources.value(signalHash, nullptr);

			TEST_PTR_CONTINUE(source);

			int samplePeriodCounter = getSamplePeriodCounter(newSamplePeriod, source->lmWorkcycle_ms());

			AppSignalStateEx* state = m_signalStates.getStateByHash(signalHash);

			TEST_PTR_CONTINUE(state);

			state->setRtSessionSamplePeriodCounter(signalHash, thisThread, m_session->id(), samplePeriodCounter);
		}
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

		if (state->signal() != nullptr)
		{
			DEBUG_LOG_MSG(m_log, QString("RtTrendsServer(%1) append signal %2").arg(m_session->id()).arg(state->signal()->appSignalID()));
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

		if (state->signal() != nullptr)
		{
			DEBUG_LOG_MSG(m_log, QString("RtTrendsServer(%1) remove signal %2").arg(m_session->id()).arg(state->signal()->appSignalID()));
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

/*		sendReply(m_rtTrendsGetStateChangesReply);
		return;*/

		const QHash<Hash, SignalStatesQueue*>& trackedSignals = m_session->trackedSignals();

		int states = 0;

		for(SignalStatesQueue* queue : trackedSignals)
		{
			TEST_PTR_CONTINUE(queue);

			Hash signalHash = queue->signalHash();

			LockFreeQueue<SignalState>& clientQueue = queue->clientQueue();

			int count = 0;

			SignalState ss;

			do
			{
				bool result = clientQueue.pop(&ss);

				if (result == false)
				{
					break;
				}

				Proto::AppSignalState* appSignalState = m_rtTrendsGetStateChangesReply.add_signalstates();

				if (appSignalState == nullptr)
				{
					assert(false);
					break;
				}

				appSignalState->set_hash(signalHash);
				appSignalState->set_value(ss.state.value);
				appSignalState->set_flags(ss.state.flags.all);

				appSignalState->set_systemtime(ss.state.time.system.timeStamp);
				appSignalState->set_localtime(ss.state.time.local.timeStamp);
				appSignalState->set_planttime(ss.state.time.plant.timeStamp);
				appSignalState->set_archiveid(ss.archiveID);

				count++;

				states++;

			} while(count < 1000);
		}

//		qDebug() << C_STR(QString("RtTrendsServer(%1) rt states = %2").arg(m_session->id()).arg(states));

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
