#ifndef TcpSourcesStateClient_H
#define TcpSourcesStateClient_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "../lib/AppSignalManager.h"
#include "MonitorConfigController.h"
#include "../lib/TcpClientsStatistics.h"

class AppDataSourceState
{
public:
	AppDataSourceState();

	Hash id() const;
	QString equipmentId() const;

	void setNewState(const ::Network::AppDataSourceState& newState);

	int getErrorsCount() const;

	bool valid() const;
	void invalidate();

	const ::Network::AppDataSourceState& previousState() const;

public:
	::Network::DataSourceInfo info;
	::Network::AppDataSourceState state;

private:
	qint64 m_previousStateUpdatePeriod = 5;

	bool m_valid = true;

	::Network::AppDataSourceState m_previousState;	// Previous state is updated every 5 seconds

	QDateTime m_perviousStateLastUpdateTime;
};

//
//		ADS_GET_APP_DATA_SOURCES_INFO
//				|
//		ADS_GET_APP_DATA_SOURCES_STATES <------+
//				|						|			Repeat it
//				+------------------------
//

class TcpAppSourcesState : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	TcpAppSourcesState(MonitorConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpAppSourcesState();

	std::vector<Hash> appDataSourceHashes();
	AppDataSourceState appDataSourceState(Hash id, bool* ok);

	int sourceErrorCount();
public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetAppDataSourcesInfo();
	void resetToGetAppDataSourcesState();

	void requestAppDataSourcesInfo();
	void processAppDataSourcesInfo(const QByteArray& data);

	void requestAppDataSourcesState();
	void processAppDataSourcesState(const QByteArray& data);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void connectionReset();

private:
	MonitorConfigController* m_cfgController = nullptr;

private:
	int m_requestPeriod = 100;

	QMutex m_appDataSourceStatesMutex;
	std::map<Hash, AppDataSourceState> m_appDataSourceStates;

	// Cache protobuf messages
	//
	::Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
	::Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStateReply;


};

#endif // TcpSourcesStateClient_H
