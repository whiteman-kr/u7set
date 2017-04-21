#pragma once

#include "../lib/Tcp.h"
#include "../lib/AppDataSource.h"
#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "AppSignalStateEx.h"


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

	void onGetDataSourcesInfoRequest();
	void onGetDataSourcesStatesRequest();

	void onGetUnitsRequest();

	// reused protobuf messages
	//
	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalParamRequest m_getAppSignalParamRequest;
	Network::GetAppSignalParamReply m_getAppSignalParamReply;

	Network::GetAppSignalStateRequest m_getAppSignalStateRequest;
	Network::GetAppSignalStateReply m_getAppSignalStateReply;

	Network::GetUnitsReply m_getUnitsReply;

	//

	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStatesReply;

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& appSignalIDs() const;
	const AppSignals& appSignals() const;
	const AppDataSourcesIP& appDataSources() const;
	const UnitList& units() const;

	bool getAppSignalStateState(Hash hash, AppSignalState& state);
	bool getDataSourceState(Hash hash, AppSignalState& state);

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
	const AppDataSourcesIP& m_appDataSources;
	const AppSignals& m_appSignals;
	const AppSignalStates& m_appSignalStates;
	const UnitList& m_units;

	void buildAppSignalIDs();

public:
	TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
							TcpAppDataServer* server,
							const AppDataSourcesIP& appDataSources,
							const AppSignals& appSignals,
							const AppSignalStates& appSignalStates,
							const UnitList& units,
							std::shared_ptr<CircularLogger> logger);

	const QVector<QString>& appSignalIDs() const { return m_appSignalIDs; }
	int appSignalIDsCount() const { return m_appSignalIDs.count(); }

	const AppSignals& appSignals() const { return m_appSignals; }
	const AppDataSourcesIP& appDataSources() const { return  m_appDataSources; }
	const UnitList& units() const { return m_units; }

	bool getAppSignalState(Hash hash, AppSignalState& state);
};

