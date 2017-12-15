#pragma once

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"
#include "../lib/OrderedHash.h"
#include "../lib/Signal.h"
#include "../lib/AppSignal.h"
#include "../Proto/network.pb.h"

class DataSource;

class TcpAppDataClient : public Tcp::Client
{
private:
	QHash<quint64, DataSource*> m_dataSources;		// id => DataSource

	QVector<Hash> m_signalHahes;
	QVector<Signal> m_signalParams;
	QVector<AppSignalState> m_states;
	QHash<Hash, int> m_hash2Index;

	// reused protobuf messages
	//
	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;

	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalRequest m_getSignalRequest;
	Network::GetAppSignalReply m_getSignalReply;

	Network::GetAppSignalStateRequest m_getSignalStateRequest;
	Network::GetAppSignalStateReply m_getSignalStateReply;

	//

	int	m_totalItemsCount = 0;
	int m_partCount = 0;
	int m_itemsPerPart = 0;

	int m_currentPart = 0;

	int m_getParamsCurrentPart = 0;
	int m_getStatesCurrentPart = 0;

	//

	void init();
	void getNextItemsPart();
	void getNextParamPart();
	void getNextStatePart();

	void onGetDataSourcesInfoReply(const char* replyData, quint32 replyDataSize);
	void onGetDataSourcesStatesReply(const char* replyData, quint32 replyDataSize);

	void onGetAppSignalListStartReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalListNextReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalParamReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize);

	//

	void clearDataSources();

public:
	TcpAppDataClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& serverAddressPort1,
					 const HostAddressPort& serverAddressPort2);

	virtual ~TcpAppDataClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

};
