#pragma once

#include "../lib/CfgServerLoader.h"

// ------------------------------------------------------------------------------------
//
// CfgControlServer class declaration
//
// ------------------------------------------------------------------------------------

class CfgControlServer : public CfgServer
{
	Q_OBJECT

public:
	CfgControlServer(const QString& equipmentID, const QString& autoloadBuildPath, const QString& workDirectory, std::shared_ptr<CircularLogger> logger);

	virtual CfgControlServer* getNewInstance() override;

	void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) final;

private:
	void sendServiceState();
	void sendClientList();
	void sendLoadedBuildInfo();
	void sendSettings();
	void sendServiceLog();

	std::shared_ptr<CircularLogger> m_logger;
	QString m_equipmentID;
	QString m_autoloadBuildPath;
};
