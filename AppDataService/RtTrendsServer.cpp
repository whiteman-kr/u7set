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
	m_sessionID = std::atomic_fetch_add(&m_globalSessionID, 1);
}

Tcp::Server* RtTrendsServer::getNewInstance()
{
	return new RtTrendsServer(localSoftwareInfo(), m_appDataSourcesIP, m_log);
}


void RtTrendsServer::onServerThreadStarted()
{
	LOG_MSG(m_log, QString("RtTrendsServer(%1) started, сlientID = %2").arg(m_sessionID).arg(connectedSoftwareInfo().equipmentID()));
}

void RtTrendsServer::onServerThreadFinished()
{
	LOG_MSG(m_log, QString("RtTrendsServer(%1) finished").arg(m_sessionID).arg(connectedSoftwareInfo().equipmentID()));
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

	sendReply(m_rtTrendsManagementReply);
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
