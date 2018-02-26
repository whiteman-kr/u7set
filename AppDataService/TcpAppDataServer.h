#pragma once

#include "../lib/Tcp.h"
#include "../lib/AppDataSource.h"
#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "AppSignalStateEx.h"


class TcpAppDataServerThread;
class AppDataServiceWorker;

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
	void onGetState();

	void onGetAppSignalListStartRequest();
	void onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize);

	void onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize);		// returns class AppSignalParam
	void onGetAppSignalRequest(const char* requestData, quint32 requestDataSize);			// returns class Signal

	void onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize);

	void onGetDataSourcesInfoRequest();
	void onGetDataSourcesStatesRequest();

	void onGetUnitsRequest();

	void onGetSettings();

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

	Network::GetUnitsReply m_getUnitsReply;

	//

	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStatesReply;

	Network::ServiceSettings m_getServiceSettings;

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& appSignalIDs() const;
	const AppSignals& appSignals() const;
	const AppDataSourcesIP& appDataSources() const;

	bool getAppSignalStateState(Hash hash, AppSignalState& state);
	bool getDataSourceState(Hash hash, AppSignalState& state);

public:
	TcpAppDataServer(const SoftwareInfo& softwareInfo);
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
	const AppDataSourcesIP& m_appDataSources;
	const AppSignals& m_appSignals;
	const AppSignalStates& m_appSignalStates;
	const AppDataServiceWorker& m_appDataServiceWorker;

	void buildAppSignalIDs();

public:
	TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
							TcpAppDataServer* server,
							const AppDataSourcesIP& appDataSources,
							const AppSignals& appSignals,
							const AppSignalStates& appSignalStates,
							const AppDataServiceWorker& appDataServiceWorker,
							std::shared_ptr<CircularLogger> logger);

	const QVector<QString>& appSignalIDs() const { return m_appSignalIDs; }
	int appSignalIDsCount() const { return m_appSignalIDs.count(); }

	const AppSignals& appSignals() const { return m_appSignals; }
	const AppDataSourcesIP& appDataSources() const { return  m_appDataSources; }

	bool getAppSignalState(Hash hash, AppSignalState& state);

	bool isConnectedToConfigurationService(quint32 &ip, quint16 &port);
	bool isConnectedToArchiveService(quint32& ip, quint16& port);

	QString equipmentID() const;
	QString cfgServiceIP1Str() const;
	QString cfgServiceIP2Str() const;
};

