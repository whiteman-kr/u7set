#pragma once

#include "CfgServerLoader.h"
#include "CircularLogger.h"


class CfgLoaderWithLog : public CfgLoader
{
public:
	CfgLoaderWithLog(	const QString& appEquipmentID,
						int appInstance,
						const HostAddressPort& serverAddressPort1,
						const HostAddressPort& serverAddressPort2,
						bool enableDownloadCfg);

	virtual void onTryConnectToServer(const HostAddressPort& serverAddr) override;
	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onStartDownload(const QString& fileName) override;
	virtual void onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode) override;
};
