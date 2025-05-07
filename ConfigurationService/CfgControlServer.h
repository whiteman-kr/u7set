#pragma once

#include "../OnlineLib/CfgLoader.h"

// ------------------------------------------------------------------------------------
//
// CfgControlServer class declaration
//
// ------------------------------------------------------------------------------------

// class CfgCheckerWorker;

// class CfgControlServer : public CfgServer
// {
// 	Q_OBJECT

// public:
// 	CfgControlServer(const SoftwareInfo& softwareInfo,
// 					 const SessionParams& sessionParams,
// 					 const QString& buildPath,
// 					 const std::list<CfgServiceSettings::ClientInfo>& clients,
// 					 bool checkClientHostname,
// 					 std::shared_ptr<CircularLogger> logger);

// 	//virtual Tcp::Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

// //	void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override final;

// private:
// 	Tcp::SetConnectionResult checkClient(const QString& clientEquipmentID, const QString& clientHostname) const override;

// };
