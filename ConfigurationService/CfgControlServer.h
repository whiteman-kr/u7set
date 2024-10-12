#pragma once

#include "../OnlineLib/CfgServerLoader.h"

// ------------------------------------------------------------------------------------
//
// CfgControlServer class declaration
//
// ------------------------------------------------------------------------------------

class CfgCheckerWorker;

class CfgControlServer : public CfgServer
{
	Q_OBJECT

public:
	CfgControlServer(const SoftwareInfo& softwareInfo,
					 const QString& autoloadBuildPath,
					 const QString& workDirectory,
					 const QString& buildPath,
					 const SessionParams& sessionParams,
					 const std::list<CfgServiceSettings::ClientInfo>& clients,
					 bool checkClientHostname,
					 const CfgCheckerWorker& checkerWorker,
					 std::shared_ptr<CircularLogger> logger);

	virtual Tcp::Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

	void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override final;

private:
	Tcp::SetConnectionResult checkClient(const QString& clientEquipmentID, const QString& clientHostname) const override;

	void sendServiceState();
	void sendLoadedBuildInfo();
	void sendSettings();
	void sendServiceLog();

	const CfgCheckerWorker& m_checkerWorker;
	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_workDirectory;
	std::list<CfgServiceSettings::ClientInfo> m_knownClients;
	bool m_checkClientHostname = false;

	SessionParams m_sessionParams;
};
