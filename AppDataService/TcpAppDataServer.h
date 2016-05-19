#pragma once

#include "../include/Tcp.h"

#include "AppSignalState.h"

class TcpAppDataServerThread;


// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
private:
	TcpAppDataServerThread* m_thread = nullptr;

	virtual Server* getNewInstance() { return new TcpAppDataServer(); }

public:
	TcpAppDataServer();
	virtual ~TcpAppDataServer();

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	void setThread(TcpAppDataServerThread* thread) { m_thread = thread; }
};


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServerThread : public Tcp::ServerThread
{
private:
	QVector<QString> m_appSignalIDs;
	const AppSignals& m_appSignals;

	void buildAppSignalIDs();

public:
	TcpAppDataServerThread(	const HostAddressPort& listenAddressPort,
							TcpAppDataServer* server,
							const AppSignals& appSignals);

	const QVector<QString>& appSignalIDs() { return m_appSignalIDs; }
};

