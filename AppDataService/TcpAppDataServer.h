#pragma once

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"

#include "../lib/Tcp.h"

#include "AppDataSource.h"
#include "SignalStatesProcessingThread.h"


class TcpAppDataServerThread;
class AppDataServiceWorker;
class AppDataReceiverThread;

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
public:
	TcpAppDataServer(const SoftwareInfo& softwareInfo,
					 AppDataReceiverThread* appDataReceiverThread,
					 SignalStatesProcessingThread* signalStatesProcessingThread);

	virtual ~TcpAppDataServer() override;

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	void setThread(TcpAppDataServerThread* thread) { m_thread = thread; }

private:
	virtual Server* getNewInstance() override;

	// Request processing functions
	//
	void onGetState();

	void onGetAppSignalListStartRequest();
	void onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize);		// returns class AppSignalParam
	void onGetAppSignalRequest(const char* requestData, quint32 requestDataSize);			// returns class Signal

	void onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize);
	void onGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppDataSourcesInfoRequest();
	void onGetAppDataSourcesStatesRequest();

	void onGetSettings();

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& acquiredAppSignalIDs() const;
	const AppSignals& appSignals() const;
	const AppDataSourcesIP& appDataSources() const;

	bool getAppSignalStateState(Hash hash, AppSignalState& state);
	bool getDataSourceState(Hash hash, AppSignalState& state);

	void getServerTimes(qint64* utc, qint64* local);

private:
	TcpAppDataServerThread* m_thread = nullptr;

	AppDataReceiverThread* m_appDataReceiverThread = nullptr;
	SignalStatesProcessingThread* m_signalStatesProcessingThread = nullptr;

	SimpleAppSignalStatesQueueShared m_signalStatesQueue;

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
	TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
							TcpAppDataServer* server,
							const AppDataSourcesIP& appDataSources,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& appSignalStates,
							const AppDataServiceWorker& appDataServiceWorker,
							std::shared_ptr<CircularLogger> logger);

	const QVector<QString>& acquiredAppSignalIDs() const { return m_acquiredAppSignalIDs; }
	int acquiredAppSignalIDsCount() const { return m_acquiredAppSignalIDs.count(); }

	const AppSignals& appSignals() const { return m_appSignals; }
	const AppDataSourcesIP& appDataSources() const { return  m_appDataSources; }

	bool getAppSignalState(Hash hash, AppSignalState& state);

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port);
	bool isConnectedToArchiveService(quint32& ip, quint16& port);

	QString equipmentID() const;
	QString cfgServiceIP1Str() const;
	QString cfgServiceIP2Str() const;

private:
	void buildAppSignalIDs();

private:
	QVector<QString> m_acquiredAppSignalIDs;

	const AppDataSourcesIP& m_appDataSources;
	const AppSignals& m_appSignals;
	const DynamicAppSignalStates& m_appSignalStates;
	const AppDataServiceWorker& m_appDataServiceWorker;
};

