#include "CfgControlServer.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

CfgControlServer::CfgControlServer(const QString& equipmentID, const QString& autoloadBuildPath, const QString& workDirectory, std::shared_ptr<CircularLogger> logger) :
	CfgServer(workDirectory, logger),
	m_logger(logger),
	m_equipmentID(equipmentID),
	m_autoloadBuildPath(autoloadBuildPath)
{

}

CfgControlServer* CfgControlServer::getNewInstance()
{
	return new CfgControlServer(m_equipmentID, m_autoloadBuildPath, m_rootFolder, m_logger);
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
	QByteArray data;
	ConfigurationServiceSettings settings;

	settings.setEquipmentID(m_equipmentID);
	settings.setAutoloadBuildPath(m_autoloadBuildPath);
	settings.setWorkDirectory(m_rootFolder);

	settings.writeToJson(data);

	sendReply(data);
}

void CfgControlServer::sendServiceLog()
{

}
