#pragma once

#include "../lib/Tcp.h"

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "../lib/TimeStamp.h"

#include "ArchRequestThread.h"


class TcpArchRequestsServer : public Tcp::Server
{
public:
	TcpArchRequestsServer(const SoftwareInfo& softwareInfo, ArchRequestThread& archRequestThread, CircularLoggerShared logger);

    virtual Tcp::Server* getNewInstance() override;
    virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	void onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize);
	void onGetSignalStatesFromArchiveNext(const char* requestData, quint32 requestDataSize);
	void onGetSignalStatesFromArchiveCancel(const char* requestData, quint32 requestDataSize);

	void finalizeCurrentRequest();

	quint32 getNewRequestID();

private:
	ArchRequestThread& m_archRequestThread;
	CircularLoggerShared m_logger;

	static std::atomic<quint32> m_nextRequestNo;

	ArchRequestContextShared m_currentRequest;
};



class TcpArchiveRequestsServerThread : public Tcp::ServerThread
{
public:
	TcpArchiveRequestsServerThread(const HostAddressPort& listenAddressPort,
				 Tcp::Server* server,
				 std::shared_ptr<CircularLogger> logger);
};
