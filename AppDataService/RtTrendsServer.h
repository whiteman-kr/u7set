#pragma once

#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include "AppDataSource.h"

class RtTrendsServer : public Tcp::Server
{
public:
	RtTrendsServer(const SoftwareInfo& sotwareInfo,
				   AppDataSourcesIP& appDataSourcesIP,
				   std::shared_ptr<CircularLogger> logger);

	Tcp::Server* getNewInstance() override;

	void onServerThreadStarted() override;
	void onServerThreadFinished() override;

	void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	void onRtTrendsManagementRequest(const char* requestData, quint32 requestDataSize);
	void onRtTrendsGetStateChangesRequest(const char* requestData, quint32 requestDataSize);

private:
	AppDataSourcesIP& m_appDataSourcesIP;
	std::shared_ptr<CircularLogger> m_log;
	int m_sessionID = 0;

	static std::atomic<int> m_globalSessionID;

	//

	Network::RtTrendsManagementRequest m_rtTrendsManagementRequest;
	Network::RtTrendsManagementReply m_rtTrendsManagementReply;

	Network::RtTrendsGetStateChangesRequest m_rtTrendsGetStateChangesRequest;
	Network::RtTrendsGetStateChangesReply m_rtTrendsGetStateChangesReply;
};

//

class RtTrendsServerThread : public Tcp::ServerThread
{
public:
	RtTrendsServerThread(const SoftwareInfo& sotwareInfo,
						 const HostAddressPort& listenAddressPort,
						 AppDataSourcesIP &appDataSourcesIP,
						 std::shared_ptr<CircularLogger> logger);
};
