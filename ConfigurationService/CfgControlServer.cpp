#include "CfgControlServer.h"
#include "CfgChecker.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

CfgControlServer::CfgControlServer(const SoftwareInfo& softwareInfo,
								   const QString& autoloadBuildPath,
								   const QString& workDirectory,
								   const QString& buildPath,
								   const SessionParams& sessionParams,
								   const std::list<CfgServiceSettings::ClientInfo> &clients,
								   bool checkClientHostname,
								   const CfgCheckerWorker& checkerWorker,
								   std::shared_ptr<CircularLogger> logger) :
	CfgServer(buildPath, softwareInfo, sessionParams, logger),
	m_checkerWorker(checkerWorker),
	m_equipmentID(softwareInfo.equipmentID()),
	m_autoloadBuildPath(autoloadBuildPath),
	m_workDirectory(workDirectory),
	m_knownClients(clients),
	m_sessionParams(sessionParams),
	m_checkClientHostname(checkClientHostname)
{
}

Tcp::Server* CfgControlServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
{
	CfgControlServer* newServer = new CfgControlServer(localSoftwareInfo(), m_autoloadBuildPath, m_workDirectory,
														m_rootFolder, m_sessionParams,
														m_knownClients, m_checkClientHostname,
														m_checkerWorker, log());
	newServer->setListenAddress(listenAddr);
	return newServer;
}

void CfgControlServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch (requestID)
	{
		case CFGS_GET_SERVICE_STATE:
			sendServiceState();
			break;

		case CFGS_GET_CLIENT_LIST:
			sendClientList();
			break;

		case CFGS_GET_SETTINGS:
			sendSettings();
			break;

		case CFGS_GET_LOG:
			sendServiceLog();
			break;

		default:
			CfgServer::processRequest(requestID, requestData, requestDataSize);
	}
}

Tcp::SetConnectionResult CfgControlServer::checkClient(const QString& clientEquipmentID, const QString& clientHostname) const
{
	for(const auto& ci : m_knownClients)
	{
		if (clientEquipmentID != ci.equipmentID)
		{
			continue;
		}

		if (m_checkClientHostname == true)
		{
			if (clientHostname != ci.hostname)
			{
				return Tcp::SetConnectionResult::WrongClientHostname;
			}
		}

		return Tcp::SetConnectionResult::Ok;
	}

	return Tcp::SetConnectionResult::UnknownClientID;
}

void CfgControlServer::sendServiceState()
{
	Network::ConfigurationServiceState message;

	message.set_currentbuilddirectory(m_rootFolder.toStdString());
	message.set_checkbuildattemptquantity(m_checkerWorker.checkNewBuildAttemptQuantity());
	message.set_buildcheckerstate(TO_INT(m_checkerWorker.checkNewBuildStage()));

	sendReply(message);
}

void CfgControlServer::sendSettings()
{
	Network::ConfigurationServiceSettings message;

	message.set_equipmentid(m_equipmentID.toStdString());
	message.set_autoloadbuildpath(m_autoloadBuildPath.toStdString());
	message.set_workdirectory(m_workDirectory.toStdString());

	sendReply(message);
}

void CfgControlServer::sendServiceLog()
{
}
