#include "RtTrendsServer.h"

namespace RtTrends
{

	// -----------------------------------------------------------------------------------------------
	//
	// Class RtTrends::Session implementation
	//
	// -----------------------------------------------------------------------------------------------

	Session::Session(int id) :
		m_id(id)
	{

	}

	// -----------------------------------------------------------------------------------------------
	//
	// Class RtTrends::Server implementation
	//
	// -----------------------------------------------------------------------------------------------

	std::atomic<int> Server::m_globalSessionID = 1;


	Server::Server(	const SoftwareInfo& sotwareInfo,
					const SignalsToSources& signalsToSources,
					std::shared_ptr<CircularLogger> logger) :
		Tcp::Server(sotwareInfo),
		m_signalsToSources(signalsToSources),
		m_log(logger)
	{
		m_session.ID = std::atomic_fetch_add(&m_globalSessionID, 1);
	}

	Tcp::Server* Server::getNewInstance()
	{
		return new Server(localSoftwareInfo(), m_appDataSourcesIP, m_log);
	}


	void Server::onServerThreadStarted()
	{
		LOG_MSG(m_log, QString("RtTrendsServer(%1) started, сlientID = %2").arg(m_session.ID).arg(connectedSoftwareInfo().equipmentID()));
	}

	void Server::onServerThreadFinished()
	{
		LOG_MSG(m_log, QString("RtTrendsServer(%1) finished").arg(m_session.ID).arg(connectedSoftwareInfo().equipmentID()));
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

		deleteTrackedSignals(m_rtTrendsManagementRequest);

		sendReply(m_rtTrendsManagementReply);
	}

	void Server::setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod)
	{
		if (newSamplePeriod == m_session.samplePeriod)
		{
			return;
		}

		m_session.samplePeriod = newSamplePeriod;

		for(AppDataSourceShared source : m_trackedSources)
		{
			source->setRtTrendsSamplePeriod(m_session.ID, m_session.samplePeriod);
		}
	}

	void Server::appendTrackedSignals(const Network::RtTrendsManagementRequest& request)
	{
		int size = request.appendsignalhashes_size();

		for(int i = 0; i < size; i++)
		{
			Hash hashToAppend = request.appendsignalhashes(i);

			if (m_session.trackedSignals.contains(hashToAppend) == true)
			{
				continue;
			}

			AppDataSourceShared source = m_signalsToSources.value(hashToAppend, nullptr);

			if (source == nullptr)
			{
				assert(false);
				continue;
			}

			bool res = source->appendRtTrendsSignal(m_session.ID, hashToAppend, m_session.samplePeriod);

			if (res == false)
			{
				assert(false);
				continue;
			}

			m_trackedSources.insert(source->lmAddress32(), source);
			m_session.trackedSignals.insert(hashToAppend, true);
		}
	}

	void Server::deleteTrackedSignals(const Network::RtTrendsManagementRequest& request)
	{
		int size = request.deletesignalhashes_size();

		for(int i = 0; i < size; i++)
		{
			Hash hashToDelete = request.appendsignalhashes(i);

			if (m_session.trackedSignals.contains(hashToDelete) == false)
			{
				continue;
			}

			AppDataSourceShared source = m_signalsToSources.value(hashToDelete, nullptr);

			if (source == nullptr)
			{
				assert(false);
				continue;
			}

			m_session.trackedSignals.remove(hashToDelete);

			bool removeSourceFromTracked = source->deleteRtTrendsSignal(m_session.ID, hashToDelete, m_session.samplePeriod);

			if (removeSourceFromTracked == true)
			{
				m_trackedSources.remove(source->lmAddress32());
			}
		}
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

	ServerThread::ServerThread(	const SoftwareInfo& sotwareInfo,
								const HostAddressPort& listenAddressPort,
								const SignalsToSources& signalsToSources,
								std::shared_ptr<CircularLogger> logger) :
		Tcp::ServerThread(listenAddressPort,
						  new Server(sotwareInfo, signalsToSources, logger),
						  logger)
	{
	}

}
