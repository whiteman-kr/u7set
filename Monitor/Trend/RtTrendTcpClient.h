#ifndef RTTRENDTCPCLIENT_H
#define RTTRENDTCPCLIENT_H

#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "../Proto/network.pb.h"
#include "../TrendView/TrendSignalSet.h"
#include "MonitorConfigController.h"


//     onConnection()
//            |
//	 startRequestCycle()    <-------------------------------------------+
//	          |															|
//   requestTrendManagement() - RT_TRENDS_MANAGEMENT					|
//   processTrendManagement() - RT_TRENDS_MANAGEMENT					|
//            |															|
//   requestTrendStateChanges() - RT_TRENDS_GET_STATE_CHANGES			|
//   processTrendStateChanges() - RT_TRENDS_GET_STATE_CHANGES			|
//            |															|
//	 emit dataReady(...)												|
//            |															|
//            +---------------------------------------------------------+
//
class RtTrendTcpClient : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	RtTrendTcpClient(const MonitorConfigController* configController, ILogFile* logFile);
	virtual ~RtTrendTcpClient();

	// Methods
	//
public:
	bool addSignals(const QStringList& appSignalIds);
	bool setData(const QStringList& trendSignals);
	bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);

	void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);
	E::RtTrendsSamplePeriod samplePeriod() const;

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
	void connectionLost();

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
	const MonitorConfigController* m_cfgController = nullptr;
	HasLogFile m_logFile;

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
