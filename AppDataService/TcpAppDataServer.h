#pragma once

#include "../include/Tcp.h"
#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
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
	void onGetAppSignalListStartRequest();
	void onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize);
	void onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize);
	void onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize);

	// reused protobuf messages
	//
	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalParamRequest m_getAppSignalParamRequest;
	Network::GetAppSignalParamReply m_getAppSignalParamReply;

	Network::GetAppSignalStateRequest m_getAppSignalStateRequest;
	Network::GetAppSignalStateReply m_getAppSignalStateReply;

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& appSignalIDs() const;
	const AppSignals& appSignals() const;

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

	const QVector<QString>& appSignalIDs() const { return m_appSignalIDs; }
	int appSignalIDsCount() const { return m_appSignalIDs.count(); }
	const AppSignals& appSignals() const { return m_appSignals; }
};

