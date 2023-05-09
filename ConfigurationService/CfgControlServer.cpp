#include "CfgControlServer.h"
#include "../Proto/network.pb.h"
#include "CfgChecker.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

CfgControlServer::CfgControlServer(const SoftwareInfo& softwareInfo,
								   E::SecurityLevel securityLevel,
								   const QString& autoloadBuildPath,
								   const QString& workDirectory,
								   const QString& buildPath,
								   const SessionParams& sessionParams,
								   const QList<CfgServiceSettings::ClientInfo>& clients,
								   bool checkClientHostname,
								   const CfgCheckerWorker& checkerWorker,
								   std::shared_ptr<CircularLogger> logger) :
	CfgServer(buildPath, softwareInfo, securityLevel, sessionParams, logger),
	m_checkerWorker(checkerWorker),
	m_equipmentID(softwareInfo.equipmentID()),
	m_autoloadBuildPath(autoloadBuildPath),
	m_workDirectory(workDirectory),
	m_sessionParams(sessionParams),
	m_knownClients(clients),
	m_checkClientHostname(checkClientHostname)
{
}

CfgControlServer* CfgControlServer::getNewInstance()
{
	return new CfgControlServer(localSoftwareInfo(), securityLevel(),
								m_autoloadBuildPath, m_workDirectory,
								m_rootFolder, m_sessionParams,
								m_knownClients, m_checkClientHostname,
								m_checkerWorker, log());
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

		case CFGS_GET_LOADED_BUILD_INFO:
			sendLoadedBuildInfo();
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

void CfgControlServer::sendLoadedBuildInfo()
{
	Network::BuildInfo message;

	const OnlineLib::BuildInfo& b = buildInfo();

	message.set_project(b.project.toStdString());
	message.set_id(b.id);
	message.set_date(b.date.toMSecsSinceEpoch());
	message.set_changeset(b.changeset);
	message.set_user(b.user.toStdString());
	message.set_workstation(b.workstation.toStdString());

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
