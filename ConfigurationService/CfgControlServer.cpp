#include "CfgControlServer.h"
#include "CfgChecker.h"

// -------------------------------------------------------------------------------------
//
// CfgControlServer class implementation
//
// -------------------------------------------------------------------------------------

// CfgControlServer::CfgControlServer(const SoftwareInfo& softwareInfo,
// 								   const SessionParams& sessionParams,
// 								   const QString& buildPath,
// 								   const std::list<CfgServiceSettings::ClientInfo> &clients,
// 								   bool checkClientHostname,
// 								   CircularLoggerShared logger) :
// 	CfgServer(softwareInfo, sessionParams, buildPath, logger),
// 	m_knownClients(clients),
// 	m_checkClientHostname(checkClientHostname)
// {
// }

// Tcp::Server* CfgControlServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
// {
// 	CfgControlServer* newServer = new CfgControlServer(localSoftwareInfo(), m_autoloadBuildPath, m_workDirectory,
// 														m_rootFolder, m_sessionParams,
// 														m_knownClients, m_checkClientHostname,
// 														m_checkerWorker, log());
// 	newServer->setListenAddress(listenAddr);
// 	return newServer;
// }

