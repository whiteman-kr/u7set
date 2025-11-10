#pragma once

#include "../OnlineLib/TcpFileTransfer.h"
#include "../OnlineLib/CfgLoader.h"

// -------------------------------------------------------------------------------------
//
// CfgServer class declaration
//
// -------------------------------------------------------------------------------------

class CfgServer : public Tcp::FileServer, public CfgServerLoaderBase
{
	Q_OBJECT

public:
	CfgServer(const SoftwareInfo& softwareInfo,
			  const SessionParams& sessionParams,
			  const QString& buildFolder,
			  const std::vector<ClientInfo>& clients,
			  bool checkClientHostname,
			  CircularLoggerShared logger);

	virtual Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

	virtual void processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	const OnlineLib::BuildInfo& buildInfo() { return m_buildInfo; }

private:
	Tcp::SetConnectionResult checkClient(const QString& clientEquipmentID, const QString& clientHostname) const;

	void readBuildXml();

	bool checkFile(QString& pathFileName, QByteArray& fileData) override;

	void processGetSessionParamsRequest();

private:
	SessionParams m_sessionParams;
	std::vector<ClientInfo> m_knownClients;
	bool m_checkClientHostname = false;

	QString m_buildXmlPathFileName;

	OnlineLib::BuildInfo m_buildInfo;
	std::map<QString, OnlineLib::BuildFileInfo> m_buildFileInfo;		// fileName => buildFileInfo

	ErrorCode m_errorCode = ErrorCode::Ok;
};

