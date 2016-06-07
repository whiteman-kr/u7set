#pragma once

#include "../include/Tcp.h"
#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "AppSignalStateEx.h"
#include "AppDataSource.h"


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
	void onGetDataSourcesStatesRequest(const char* requestData, quint32 requestDataSize);

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

	// helper functions
	//
	int getSignalListPartCount(int signalCount);

	const QVector<QString>& appSignalIDs() const;
	const AppSignals& appSignals() const;
	const AppDataSources& appDataSources() const;
	const UnitList& units() const;

	bool getConnectionState(Hash hash, AppSignalState& state);

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
	const AppDataSources& m_appDataSources;
	const AppSignals& m_appSignals;
	const AppSignalStates& m_appSignalStates;
	const UnitList& m_units;

	void buildAppSignalIDs();

public:
	TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
							TcpAppDataServer* server,
							const AppDataSources& appDataSources,
							const AppSignals& appSignals,
							const AppSignalStates& appSignalStates,
							const UnitList& units);

	const QVector<QString>& appSignalIDs() const { return m_appSignalIDs; }
	int appSignalIDsCount() const { return m_appSignalIDs.count(); }

	const AppSignals& appSignals() const { return m_appSignals; }
	const AppDataSources& appDataSources() const { return  m_appDataSources; }
	const UnitList& units() const { return m_units; }

	bool getState(Hash hash, AppSignalState& state);
};

