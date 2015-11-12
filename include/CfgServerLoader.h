#pragma once

#include "../include/TcpFileTransfer.h"


// -------------------------------------------------------------------------------------
//
// CfgServer class declaration
//
// -------------------------------------------------------------------------------------

class CfgServer : public Tcp::FileServer
{
private:
	QString m_buildXmlPathFileName;

	enum ErrorCode
	{
		Ok,
		BuildNotFound,
		BuildCantRead
	};

	ErrorCode m_errorCode = ErrorCode::Ok;

	void onRootFolderChange();

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

class CfgLoader: public Tcp::FileClient
{
private:
public:
	CfgLoader(const QString& appStrID, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	virtual void onClientThreadStarted() override;
};
