#pragma once

#include "../lib/CfgServerLoader.h"

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
	CfgControlServer(const QString& equipmentID,
					 const QString& autoloadBuildPath,
					 const QString& workDirectory,
					 const QString& buildPath,
					 const CfgCheckerWorker& checkerWorker,
					 std::shared_ptr<CircularLogger> logger);

	virtual CfgControlServer* getNewInstance() override;

	void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) final;

public slots:
	void updateClientsInfo(std::list<std::shared_ptr<const Tcp::ConnectionState>> connectionStates) { m_connectionStates = connectionStates; }

private:
	void sendServiceState();
	void sendClientList();
	void sendLoadedBuildInfo();
	void sendSettings();
	void sendServiceLog();

	std::shared_ptr<CircularLogger> m_logger;
	const CfgCheckerWorker& m_checkerWorker;
	static std::list<std::shared_ptr<const Tcp::ConnectionState>> m_connectionStates;
	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_workDirectory;
};
