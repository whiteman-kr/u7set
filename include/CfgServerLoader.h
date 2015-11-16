#pragma once

#include "../include/TcpFileTransfer.h"


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class declaration
//
// -------------------------------------------------------------------------------------

class CfgServerLoaderBase
{
protected:

	enum ErrorCode
	{
		Ok,
		BuildNotFound,
		BuildCantRead
	};
};


// -------------------------------------------------------------------------------------
//
// CfgServer class declaration
//
// -------------------------------------------------------------------------------------

class CfgServer : public Tcp::FileServer, public CfgServerLoaderBase
{
private:
	QString m_buildXmlPathFileName;

	ErrorCode m_errorCode = ErrorCode::Ok;

	void onRootFolderChange();

	void readBuildXml();

public:
	CfgServer(const QString& rootFolder);

	virtual CfgServer* getNewInstance() override { return new CfgServer(m_rootFolder); }

	virtual void onServerThreadStarted() override;
};


// -------------------------------------------------------------------------------------
//
// CfgLoader class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoader: public Tcp::FileClient, public CfgServerLoaderBase
{
private:
public:
	CfgLoader(const QString& appStrID, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	virtual void onClientThreadStarted() override;
};
