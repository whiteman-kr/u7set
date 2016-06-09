#pragma once

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"
#include "../lib/OrderedHash.h"
#include "../lib/Signal.h"
#include "../Proto/network.pb.h"

class QTimer;

class TcpAppDataClient : public Tcp::Client
{
	Q_OBJECT

private:
	QHash<quint64, DataSource*> m_dataSources;		// id => DataSource

	QVector<Hash> m_signalHahes;
	QVector<Signal> m_signalParams;
	QVector<AppSignalState> m_states;
	QHash<Hash, int> m_hash2Index;
	UnitList m_unitList;

	QTimer* m_updateStatesTimer = nullptr;

	// reused protobuf messages
	//
	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;

	Network::GetDataSourcesStatesReply m_getDataSourcesStatesReply;

	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalParamRequest m_getSignalParamRequest;
	Network::GetAppSignalParamReply m_getSignalParamReply;

	Network::GetAppSignalStateRequest m_getSignalStateRequest;
	Network::GetAppSignalStateReply m_getSignalStateReply;

	Network::GetUnitsReply m_getUnitsReply;

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

	void onGetUnitsReply(const char* replyData, quint32 replyDataSize);

	//

	void clearDataSources();

private slots:
	void updateStates();

signals:
	void dataSourcesInfoLoaded();
	void appSignalListLoaded();

	void dataSoursesStateUpdated();
	void appSignalsStateUpdated();

	void disconnected();

public:
	TcpAppDataClient(const HostAddressPort& serverAddressPort);
	TcpAppDataClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpAppDataClient();

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	QList<DataSource*> dataSources() { return m_dataSources.values(); }
	const QVector<Signal>& signalParams() { return m_signalParams; }
	const QVector<AppSignalState>& signalStates() { return m_states; }
	QString unit(int id) { return m_unitList.contains(id) ? m_unitList.value(id) : tr("Unknown unit ID = %1").arg(id); }
};
