#ifndef TRENDTCPCLIENT_H
#define TRENDTCPCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../Proto/network.pb.h"
#include "../TrendView/TrendSignal.h"
#include "MonitorConfigController.h"

class TrendTcpClient : public Tcp::Client
{
	Q_OBJECT

public:
	TrendTcpClient(MonitorConfigController* configController);
	virtual ~TrendTcpClient();

protected:
	virtual void timerEvent(QTimerEvent* event) override;

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetRequestCycle();

	void requestStart();
	void processStart(const QByteArray& data);

	void requestNext();
	void processNext(const QByteArray& data);

public slots:
	void slot_requestData(QString appSignalId, TimeStamp hourToRequest, E::TimeType timeType);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void dataReady(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
	void requestError(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType);

private:
	int m_periodicTimerId = 0;

	MonitorConfigController* m_cfgController = nullptr;

	struct RequestQueue
	{
		QString appSignalId;
		TimeStamp hourToRequest;
		E::TimeType timeType;

		bool operator== (const RequestQueue& r) const
		{
			return	this->appSignalId == r.appSignalId &&
					this->hourToRequest == r.hourToRequest &&
					this->timeType == r.timeType;
		}
	};

	std::list<RequestQueue> m_queue;

private:
	bool requestInProgress = false;
	RequestQueue m_currentRequest;
	Hash m_currentSignalHash = 0;
	int m_currentRequestId = 0;

	std::shared_ptr<TrendLib::OneHourData> m_receivedData;

	Network::GetAppSignalStatesFromArchiveStartRequest m_startRequest;
	Network::GetAppSignalStatesFromArchiveStartReply m_startReply;

	Network::GetAppSignalStatesFromArchiveNextRequest m_nextRequest;
	Network::GetAppSignalStatesFromArchiveNextReply m_nextReply;

	QTime m_startRequestTime;

	// Statisctics and state variables
	//
public:
	QString m_statRequestDescription = 0;
	volatile int m_statRequestQueueSize = 0;
	volatile int m_statTcpRequestCount = 0;
	volatile int m_statTcpReplyCount = 0;
};

#endif // TRENDTCPCLIENT_H
