#include "CfgControlServer.h"
#include "../Proto/network.pb.h"
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
								   const CfgCheckerWorker& checkerWorker,
								   std::shared_ptr<CircularLogger> logger) :
	CfgServer(softwareInfo, buildPath, logger),
	m_logger(logger),
	m_checkerWorker(checkerWorker),
	m_equipmentID(softwareInfo.equipmentID()),
	m_autoloadBuildPath(autoloadBuildPath),
	m_workDirectory(workDirectory)
{
}

CfgControlServer* CfgControlServer::getNewInstance()
{
	return new CfgControlServer(localSoftwareInfo(), m_autoloadBuildPath, m_workDirectory, m_rootFolder, m_checkerWorker, m_logger);
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

void CfgControlServer::updateClientsInfo(const std::list<Tcp::ConnectionState>& connectionStates)
{
	m_statesMutex.lock();

	m_connectionStates = connectionStates;

	m_statesMutex.unlock();
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

	m_statesMutex.lock();

	for(const Tcp::ConnectionState& state : m_connectionStates)
	{
		const SoftwareInfo& si = state.connectedSoftwareInfo;

		if (E::containes<E::SoftwareType>(TO_INT(si.softwareType())) == false)
		{
			continue;
		}

		Network::ConfigurationServiceClientInfo* i = message.add_clients();

		i->set_softwaretype(TO_INT(si.softwareType()));

		i->set_equipmentid(si.equipmentID().toStdString());

		i->set_majorversion(si.majorVersion());
		i->set_minorversion(si.minorVersion());
		i->set_commitno(si.commitNo());

		i->set_ip(state.peerAddr.address32());

		i->set_uptime(QDateTime::currentMSecsSinceEpoch() - state.startTime);
		i->set_isactual(state.isActual);
		i->set_replyquantity(state.replyCount);
	}

	m_statesMutex.unlock();

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
