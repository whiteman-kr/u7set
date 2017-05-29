#include "CfgControlServer.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

CfgControlServer::CfgControlServer(const QString& buildFolder, std::shared_ptr<CircularLogger> logger) :
	CfgServer(buildFolder, logger),
	m_logger(logger)
{

}

CfgControlServer* CfgControlServer::getNewInstance()
{
	return new CfgControlServer(m_rootFolder, m_logger);
}

void CfgControlServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch (requestID)
	{
		case RQID_GET_CONFIGURATION_SERVICE_STATE:
			sendServiceState();
			break;

		case RQID_GET_CONFIGURATION_SERVICE_CLIENT_LIST:
			sendClientList();
			break;

		case RQID_GET_CONFIGURATION_SERVICE_LOADED_BUILD_INFO:
			sendLoadedBuildInfo();
			break;

		case RQID_GET_CONFIGURATION_SERVICE_SETTINGS:
			sendSettings();
			break;

		case RQID_GET_CONFIGURATION_SERVICE_LOG:
			sendServiceLog();
			break;

		default:
			CfgServer::processRequest(requestID, requestData, requestDataSize);
	}
}

void CfgControlServer::sendServiceState()
{

}

void CfgControlServer::sendClientList()
{

}

void CfgControlServer::sendLoadedBuildInfo()
{
	QByteArray data;
	ConfigurationServiceBuildInfo info(buildInfo());

	info.writeToJson(data);

	sendReply(data);
}

void CfgControlServer::sendSettings()
{

}

void CfgControlServer::sendServiceLog()
{

}
