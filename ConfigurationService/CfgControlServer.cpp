#include "CfgControlServer.h"
#include "../Proto/network.pb.h"
#include "CfgChecker.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

std::list<Tcp::ConnectionState> CfgControlServer::m_connectionStates;

CfgControlServer::CfgControlServer(const QString& equipmentID, const QString& autoloadBuildPath, const QString& workDirectory, const QString& buildPath, const CfgCheckerWorker& checkerWorker, std::shared_ptr<CircularLogger> logger) :
	CfgServer(buildPath, logger),
	m_logger(logger),
	m_checkerWorker(checkerWorker),
	m_equipmentID(equipmentID),
	m_autoloadBuildPath(autoloadBuildPath),
	m_workDirectory(workDirectory)
{

}

CfgControlServer* CfgControlServer::getNewInstance()
{
	return new CfgControlServer(m_equipmentID, m_autoloadBuildPath, m_workDirectory, m_rootFolder, m_checkerWorker, m_logger);
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
	Network::ConfigurationServiceState message;

	message.set_currentbuilddirectory(m_rootFolder.toStdString());
	message.set_checkbuildattemptquantity(m_checkerWorker.checkNewBuildAttemptQuantity());
	message.set_buildcheckerstate(TO_INT(m_checkerWorker.checkNewBuildStage()));

	sendReply(message);
}

void CfgControlServer::sendClientList()
{
	Network::ConfigurationServiceClients message;

	for(const Tcp::ConnectionState& state : m_connectionStates)
	{
		Network::ConfigurationServiceClientInfo* i = message.add_clients();

		i->set_softwaretype(0);
		i->set_equipmentid("???");
		i->set_ip(state.peerAddr.address32());
		i->set_uptime(QDateTime::currentMSecsSinceEpoch() - state.startTime);
		i->set_isactual(false);
		i->set_replyquantity(state.replyCount);
	}

	sendReply(message);
}

void CfgControlServer::sendLoadedBuildInfo()
{
	Network::BuildInfo message;

	const Builder::BuildInfo& b = buildInfo();

	message.set_project(b.project.toStdString());
	message.set_id(b.id);
	message.set_release(b.release);
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
