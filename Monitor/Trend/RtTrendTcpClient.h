#ifndef RTTRENDTCPCLIENT_H
#define RTTRENDTCPCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../Proto/network.pb.h"
#include "../TrendView/TrendSignal.h"
#include "MonitorConfigController.h"
#include "../lib/TcpClientsStatistics.h"

class RtTrendTcpClient : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	RtTrendTcpClient(MonitorConfigController* configController);
	virtual ~RtTrendTcpClient();

	// Methods
	//
public:
	bool setData(E::RtTrendsSamplePeriod samplePeriod, const std::vector<TrendLib::TrendSignalParam> trendSignals);
	bool clearData();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void startRequestCycle();

	void requestTrendManagement();
	void processTrendManagement(const QByteArray& data);

	void requestTrendStateChanges();
	void processTrendStateChanges(const QByteArray& data);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void dataReady(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
	void requestError(QString text);

	// Staticstic
	//
public:
	struct Stat
	{
		QString text;
		int requestQueueSize = 0;
		int requestCount = 0;
		int replyCount = 0;
	};

	Stat stat() const;
	void setStat(const Stat& stat);

	void setStatText(const QString& text);
	void setStatRequestQueueSize(int value);

	void incStatRequestCount();
	void incStatReplyCount();

	// Data
	//
private:
	MonitorConfigController* m_cfgController = nullptr;

	mutable QMutex m_dataMutex;

	E::RtTrendsSamplePeriod m_samplePeriod;
	std::set<Hash> m_signalSet;

private:
	Network::RtTrendsManagementRequest m_managementRequest;
	Network::RtTrendsManagementReply m_managementReply;

	Network::RtTrendsGetStateChangesRequest m_stateChangesRequest;
	Network::RtTrendsGetStateChangesReply m_stateChangesReply;

	std::set<Hash> m_trackedSignals;		// Currently tracked signals by AppDataService

	// Statisctics and state variables
	//
	mutable QMutex m_statMutex;
	Stat m_stat;
};

#endif // RTTRENDTCPCLIENT_H
