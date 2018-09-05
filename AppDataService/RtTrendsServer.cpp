#include "RtTrendsServer.h"

// -----------------------------------------------------------------------------------------------
//
// Class RtTrendsServer implementation
//
// -----------------------------------------------------------------------------------------------

std::atomic<int> RtTrendsServer::m_globalSessionID = 1;


RtTrendsServer::RtTrendsServer(const SoftwareInfo& sotwareInfo,
							   AppDataSourcesIP& appDataSourcesIP,
							   std::shared_ptr<CircularLogger> logger) :
	Tcp::Server(sotwareInfo),
	m_appDataSourcesIP(appDataSourcesIP),
	m_log(logger)
{
	m_session.ID = std::atomic_fetch_add(&m_globalSessionID, 1);
}

Tcp::Server* RtTrendsServer::getNewInstance()
{
	return new RtTrendsServer(localSoftwareInfo(), m_appDataSourcesIP, m_log);
}


void RtTrendsServer::onServerThreadStarted()
{
	LOG_MSG(m_log, QString("RtTrendsServer(%1) started, сlientID = %2").arg(m_session.ID).arg(connectedSoftwareInfo().equipmentID()));
}

void RtTrendsServer::onServerThreadFinished()
{
	LOG_MSG(m_log, QString("RtTrendsServer(%1) finished").arg(m_session.ID).arg(connectedSoftwareInfo().equipmentID()));
}

void RtTrendsServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
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

void RtTrendsServer::onRtTrendsManagementRequest(const char* requestData, quint32 requestDataSize)
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

void RtTrendsServer::setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod)
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

void RtTrendsServer::appendTrackedSignals(const Network::RtTrendsManagementRequest& request)
{
	int size = request.appendsignalhashes_size();

	for(int i = 0; i < size; i++)
	{
		Hash hashToAppend = request.appendsignalhashes(i);

		if (m_session.trackedSignals.contains(hashToAppend) == true)
		{
			continue;
		}

		m_session.trackedSignals.insert(hashToAppend, true);

		for(AppDataSourceShared source : m_appDataSourcesIP)
		{
			bool res = source->appendRtTrendsSignal(m_session.ID, hashToAppend, m_session.samplePeriod);

			if (res == true)
			{
				m_trackedSources.insert(source->lmAddress32(), source);
				break;
			}
		}
	}
}

void RtTrendsServer::deleteTrackedSignals(const Network::RtTrendsManagementRequest& request)
{
	int size = request.deletesignalhashes_size();

	for(int i = 0; i < size; i++)
	{
		Hash hashToDelete = request.appendsignalhashes(i);

		if (m_session.trackedSignals.contains(hashToDelete) == false)
		{
			continue;
		}

		m_session.trackedSignals.remove(hashToDelete);

		for(AppDataSourceShared source : m_trackedSources)
		{
			bool removeSourceFromTracked = source->deleteRtTrendsSignal(m_session.ID, hashToDelete, m_session.samplePeriod);

			if (removeSourceFromTracked == true)
			{
				m_trackedSources.remove(source->lmAddress32());
				break;
			}
		}
	}
}

void RtTrendsServer::onRtTrendsGetStateChangesRequest(const char* requestData, quint32 requestDataSize)
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
// Class RtTrendsServerThread implementation
//
// -----------------------------------------------------------------------------------------------

RtTrendsServerThread::RtTrendsServerThread(const SoftwareInfo& sotwareInfo,
					 const HostAddressPort& listenAddressPort,
					 AppDataSourcesIP& appDataSourcesIP,
					 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort,
					  new RtTrendsServer(sotwareInfo, appDataSourcesIP, logger),
					  logger)
{
}
