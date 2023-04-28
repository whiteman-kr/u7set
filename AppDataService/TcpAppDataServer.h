#pragma once

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"

#include "../OnlineLib/Tcp.h"

#include "AppDataSource.h"
#include "SignalStatesProcessingThread.h"

class TcpAppDataServerThread;
class AppDataServiceWorker;
class AppDataReceiver;

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
public:
	TcpAppDataServer(const SoftwareInfo& softwareInfo,
					 E::SecurityLevel securityLevel,
					 AppDataServiceWorker& appDataService);

	virtual ~TcpAppDataServer() override;

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	virtual Server* getNewInstance() override;

	// Request processing functions
	//
	void onGetState();

	void onGetAppSignalListStartRequest();
	void onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize);		// returns class AppSignalParam
	void onGetAppSignalRequest(const char* requestData, quint32 requestDataSize);			// returns class Signal

	void onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize, bool constSize);
	void onGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize);
	void onGetAppSignalStateChangesForGatewayRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppDataSourcesInfoRequest();
	void onGetAppDataSourcesStatesRequest();

	void onGetSettings();

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	void getServerTimes(qint64* utc, qint64* local);

private:
	AppDataServiceWorker& m_appDataService;

	SimpleAppSignalStatesQueueShared m_signalStatesQueue;
	GatewaytAppSignalStatesQueueShared m_gatewaySignalStatesQueue;

	// precalculated variables
	//
	int m_acquiredSignalCount = 0;
	int m_acquiredSignalListPartCount = 0;

	// reused protobuf messages
	//
	Network::AppDataServiceState m_getAppDataServiceState;

	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalParamRequest m_getAppSignalParamRequest;
	Network::GetAppSignalParamReply m_getAppSignalParamReply;

	Network::GetAppSignalRequest m_getAppSignalRequest;
	Network::GetAppSignalReply m_getAppSignalReply;

	Network::GetAppSignalStateRequest m_getAppSignalStateRequest;
	Network::GetAppSignalStateReply m_getAppSignalStateReply;

	Network::GetAppSignalStateChangesRequest m_getAppSignalStateChangesRequest;
	Network::GetAppSignalStateChangesReply m_getAppSignalStateChangesReply;

	Network::GetAppSignalStateChangesForGatewayRequest m_getAppSignalStateChangesForGatewayRequest;
	Network::GetAppSignalStateChangesForGatewayReply m_getAppSignalStateChangesForGatewayReply;

	//

	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStatesReply;

	Network::ServiceSettings m_getServiceSettings;
};


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServerThread : public Tcp::ServerThread
{
public:
	TcpAppDataServerThread(const SoftwareInfo& softwareInfo,
							const HostAddressPort& listenAddressPort,
							E::SecurityLevel securityLevel,
							AppDataServiceWorker &appDataServiceWorker);
};

