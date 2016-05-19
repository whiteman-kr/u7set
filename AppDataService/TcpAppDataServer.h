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

	// precalculated variables
	//
	int m_signalCount = 0;
	int m_signalListPartCount = 0;

	virtual Server* getNewInstance();

	// Request processing functions
	//
	static const int GET_SIGNAL_LIST_ITEMS_PER_PART = 10;		// 5000
	static const int ERROR_OK = 0;
	static const int ERROR_BAD_PART_NO = 1;

	void onGetSignalListStartRequest();
	void onGetSignalListNextRequest(const char* requestData, quint32 requestDataSize);

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& appSignalIDs();

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
	int appSignalIDsCount() { return m_appSignalIDs.count(); }
};

