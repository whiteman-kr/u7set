#pragma once

#include "../lib/Tcp.h"

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "../lib/TimeStamp.h"

#include "ArchRequestThread.h"


struct ArchRequestContext
{
	TimeType timeType = TimeType::System;

	qint64 startTime = 0;
	qint64 endTime = 0;

	int hashesCount = 0;

	Hash signalHashes[ARCH_REQUEST_MAX_SIGNALS];

	quint32 requestID = 0;

	int totalStates = 0;
	int sentStatesStates = 0;
	int statesInLastPart = 0;
};


typedef std::shared_ptr<ArchRequestContext> ArchRequestContextShared;


class TcpArchiveRequestsServer : public Tcp::Server
{
public:
    TcpArchiveRequestsServer(CircularLoggerShared logger);

    virtual Tcp::Server* getNewInstance() override;
    virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	void onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize);

	//void

private:
    CircularLoggerShared m_logger;

	int m_nextRequestNo = 1;

	Network::GetAppSignalStatesFromArchiveStartRequest m_getSignalStatesRequest;
	Network::GetAppSignalStatesFromArchiveStartReply m_getSignalStatesReply;
};



class TcpArchiveRequestsServerThread : public Tcp::ServerThread
{
public:
	TcpArchiveRequestsServerThread(const HostAddressPort& listenAddressPort,
				 Tcp::Server* server,
				 std::shared_ptr<CircularLogger> logger);
};
