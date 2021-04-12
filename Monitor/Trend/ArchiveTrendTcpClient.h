#ifndef ARCHIVETRENDTCPCLIENT_H
#define ARCHIVETRENDTCPCLIENT_H

#include "../OnlineLib/Tcp.h"
#include "../UtilsLib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../Proto/network.pb.h"
#include "../TrendView/TrendSignal.h"
#include "MonitorConfigController.h"
#include "../lib/TcpClientsStatistics.h"

class ArchiveTrendTcpClient : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	ArchiveTrendTcpClient(MonitorConfigController* configController);
	virtual ~ArchiveTrendTcpClient();

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
	quint32 m_currentRequestId = 0;

	std::shared_ptr<TrendLib::OneHourData> m_receivedData;

	Network::GetAppSignalStatesFromArchiveStartRequest m_startRequest;
	Network::GetAppSignalStatesFromArchiveStartReply m_startReply;

	Network::GetAppSignalStatesFromArchiveNextRequest m_nextRequest;
	Network::GetAppSignalStatesFromArchiveNextReply m_nextReply;

	QElapsedTimer m_startRequestTime;

	// Statisctics and state variables
	//
	mutable QMutex m_statMutex;
	Stat m_stat;
};

#endif // ARCHIVETRENDTCPCLIENT_H
