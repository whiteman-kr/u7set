#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include <CommonLib/Times.h>
#include <TrendView/TrendArchiveServer.h>
#include <TrendView/TrendSignalState.h>


class ArchiveTrendTcpClient : public Tcp::Client,
							  public TcpClientStatistics
{
	Q_OBJECT

public:
	ArchiveTrendTcpClient(const SoftwareInfo& softwareInfo, const SoftwareEndpoint::ArchiveService& server, ILogFile* logFile);
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
	void slot_requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId, TimeStamp hourToRequest, E::TimeType timeType);

signals:
	void dataReady(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId,
				   TimeStamp requestedHour,
				   E::TimeType timeType,
				   std::shared_ptr<TrendLib::OneHourData> data);
	void requestError(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType);

	// Staticstic
	//
public:
	struct Stat
	{
		QString text;
		int requestQueueSize = 0;
		int requestCount = 0;
		int replyCount = 0;
		int isConnected = 0; // do not make it bool please, it is convenient to propogate the summ of statistics later
	};

	Stat stat() const;
	void setStat(const Stat& stat);

	void setStatText(const QString& text);
	void setStatRequestQueueSize(int value);

	void incStatRequestCount();
	void incStatReplyCount();


private:
	int m_periodicTimerId = 0;

	const SoftwareEndpoint::ArchiveService m_server;
	HasLogFile m_logFile;

	struct RequestQueue
	{
		TrendLib::TrendSignalPlusServerId signalPlusServerId;
		TimeStamp hourToRequest;
		E::TimeType timeType{E::TimeType::Plant};

		bool operator==(const RequestQueue& r) const
		{
			return this->signalPlusServerId == r.signalPlusServerId && this->hourToRequest == r.hourToRequest &&
				   this->timeType == r.timeType;
		}

		QString toString()
		{
			return QString{"RequestQueue{'%1', TimeType %2, HourToRequest %3}"}.arg(signalPlusServerId.appSignalId,
																					E::valueToString(timeType),
																					hourToRequest.toDateTime().toString());
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
