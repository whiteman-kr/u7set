#pragma once

#include <unordered_map>

#include "../OnlineLib/TcpFileTransfer.h"
#include "../OnlineLib/GrpcFileSrv.h"

// -------------------------------------------------------------------------------------
//
// GrpcCfgServer class declaration
//
// -------------------------------------------------------------------------------------

class GrpcCfgServer : public GrpcFileSrv, public CfgServerLoaderBase
{
public:
	GrpcCfgServer(const SoftwareInfo& softwareInfo,
				const SessionParams& sessionParams,
				const std::vector<ClientInfo>& clients,
				bool checkClientHostname,
				const HostAddressPort& listenIP,
				const QString& buildFolder,
				CircularLoggerShared logger);

	const OnlineLib::BuildInfo& buildInfo() const;

private:
	void readBuildXml(const QString& buildFolder);

	bool checkFile(const QString& pathFileName, const QByteArray& fileData) const override;
	virtual void getSessionParams(Network::SessionParams* params) const override;

private:
	SessionParams m_sessionParams;

	bool m_buildReadOK = false;

	OnlineLib::BuildInfo m_buildInfo;
	std::unordered_map<QString, OnlineLib::BuildFileInfo> m_buildFileInfo;		// fileName => buildFileInfo
};
