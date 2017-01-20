#include "CfgLoaderWithLog.h"


CfgLoaderWithLog::CfgLoaderWithLog(	const QString& appEquipmentID,
									int appInstance,
									const HostAddressPort& serverAddressPort1,
									const HostAddressPort& serverAddressPort2,
									bool enableDownloadCfg) :
	CfgLoader(appEquipmentID, appInstance, serverAddressPort1, serverAddressPort2, enableDownloadCfg)
{
}


void CfgLoaderWithLog::onTryConnectToServer(const HostAddressPort& serverAddr)
{
	DEBUG_LOG_MSG(QString(tr("Try connect to server %1").arg(serverAddr.addressPortStr())));
}


void CfgLoaderWithLog::onConnection()
{
	DEBUG_LOG_MSG(QString(tr("CfgLoader connected to server %1").arg(peerAddr().addressStr())));
}


void CfgLoaderWithLog::onDisconnection()
{
	DEBUG_LOG_MSG(QString(tr("CfgLoader disconnected from server %1")).arg(peerAddr().addressStr()));
}


void CfgLoaderWithLog::onStartDownload(const QString& fileName)
{
	DEBUG_LOG_MSG(QString(tr("Start download: %1")).arg(fileName));
}


void CfgLoaderWithLog::onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode)
{
	if (errorCode != Tcp::FileTransferResult::Ok)
	{
		DEBUG_LOG_ERR(QString("File %1 download error - %2").arg(fileName).arg(getErrorStr(errorCode)));
	}
	else
	{
		DEBUG_LOG_MSG(QString("File %1 download Ok").arg(fileName));
	}
}
