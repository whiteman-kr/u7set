#pragma once

#include "../OnlineLib/Tcp.h"
#include "../AppSignalLib/SimpleAppSignalState.h"


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
					 AppDataServiceWorker& appDataService);

	virtual ~TcpAppDataServer() override;

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	virtual Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

	// Request processing functions
	//
	void onGetState();

	void onGetAppSignalListStartRequest();
	void onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize);		// returns class AppSignalParam
	void onGetAppSignalRequest(const char* requestData, quint32 requestDataSize);			// returns class Signal

	void onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize, bool constSize);
	void onGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize);
	void onGatewayGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize);

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
	GatewayAppSignalStatesQueueShared m_gatewaySignalStatesQueue;

	// precalculated variables
	//
	int m_acquiredSignalCount = 0;
	int m_acquiredSignalListPartCount = 0;
};


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServerThread : public Tcp::ListenerThread
{
public:
	TcpAppDataServerThread(const SoftwareInfo& softwareInfo,
						   const std::vector<Tcp::ListenAddress>& listenAddresses,
							AppDataServiceWorker &appDataServiceWorker);
};

