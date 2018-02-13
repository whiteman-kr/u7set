#pragma once

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"
#include "../lib/OrderedHash.h"
#include "../lib/Signal.h"
#include "../lib/AppDataSource.h"
#include "../Proto/network.pb.h"


class QTimer;

class TcpAppDataClient : public Tcp::Client
{
	Q_OBJECT

public:
	TcpAppDataClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& serverAddressPort);

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

	QList<AppDataSource*> dataSources() { return m_appDataSources.values(); }
	const QVector<Signal>& signalParams() { return m_signalParams; }
	const QVector<AppSignalState>& signalStates() { return m_states; }

	const Network::ServiceClients& clients() { return m_serviceClientsMessage; }
	bool clientsIsReady() { return m_clientsIsReady; }
	bool stateIsReady() { return m_stateIsReady; }
	bool settingsIsReady() { return m_settingsIsReady; }

	QString equipmentID() { return m_equipmentID; }
	QString configIP1() {return m_configIP1; }
	QString configIP2() { return m_configIP2; }

	QString configServiceConnectionState();
	QString archiveServiceConnectionState();

signals:
	void clientsLoaded();

	void dataSourcesInfoLoaded();
	void appSignalListLoaded();

	void dataSoursesStateUpdated();
	void appSignalsStateUpdated();

	void settingsLoaded();

	void stateLoaded();

	void disconnected();

private:
	void init();
	void getNextItemsPart();
	void getNextParamPart();
	void getNextStatePart();

	void onGetClientList(const char* replyData, quint32 replyDataSize);

	void onGetDataSourcesInfoReply(const char* replyData, quint32 replyDataSize);
	void onGetDataSourcesStatesReply(const char* replyData, quint32 replyDataSize);

	void onGetAppSignalListStartReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalListNextReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalReply(const char* replyData, quint32 replyDataSize);
	void onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize);

	void onGetUnitsReply(const char* replyData, quint32 replyDataSize);

	void onGetServiceState(const char* replyData, quint32 replyDataSize);

	void onGetServiceSettings(const char* replyData, quint32 replyDataSize);

	//

	void sendNextRequest(quint32 processedRequestID);	// Describes all request sequence

	void clearDataSources();
	void startStateUpdating();

private slots:
	void updateStates();

private:
	QHash<quint64, AppDataSource*> m_appDataSources;		// id => AppDataSource

	QVector<Hash> m_signalHashes;
	QVector<Signal> m_signalParams;
	QVector<AppSignalState> m_states;
	QHash<Hash, int> m_hash2Index;

	QTimer* m_updateStatesTimer = nullptr;

	// reused protobuf messages
	//
	Network::ServiceClients m_serviceClientsMessage;

	Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;

	Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStatesReply;

	Network::GetSignalListStartReply m_getSignalListStartReply;

	Network::GetSignalListNextRequest m_getSignalListNextRequest;
	Network::GetSignalListNextReply m_getSignalListNextReply;

	Network::GetAppSignalRequest m_getSignalsRequest;
	Network::GetAppSignalReply m_getSignalsReply;

	Network::GetAppSignalStateRequest m_getSignalStateRequest;
	Network::GetAppSignalStateReply m_getSignalStateReply;

	Network::GetUnitsReply m_getUnitsReply;

	Network::AppDataServiceState m_getAppDataServiceState;

	Network::ServiceSettings m_getServiceSettings;

	QString m_equipmentID;
	QString m_configIP1;
	QString m_configIP2;

	//

	int	m_totalItemsCount = 0;
	int m_partCount = 0;
	int m_itemsPerPart = 0;

	int m_currentPart = 0;

	int m_getParamsCurrentPart = 0;
	int m_getStatesCurrentPart = 0;

	bool m_clientsIsReady = false;
	bool m_settingsIsReady = false;
	bool m_stateIsReady = false;
};
