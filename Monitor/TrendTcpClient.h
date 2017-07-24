#ifndef TRENDTCPCLIENT_H
#define TRENDTCPCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../Proto/network.pb.h"
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

//	void requestSignalParam(int startIndex);
//	void processSignalParam(const QByteArray& data);

//	void requestUnits();
//	void processUnits(const QByteArray& data);

//	void requestSignalState(int startIndex);
//	void processSignalState(const QByteArray& data);

public slots:
	void slot_requestData(QString appSignalId, TimeStamp hourToRequest);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

private:
	int m_timerId = 0;
	MonitorConfigController* m_cfgController = nullptr;

	struct RequestQueue
	{
		QString appSignalId;
		TimeStamp hourToRequest;
	};

	std::queue<RequestQueue> m_queue;

private:
	RequestQueue m_currentRequest;
	Hash m_currentSignalHash = 0;
	int m_currentRequestId = 0;

	Network::GetAppSignalStatesFromArchiveStartRequest m_startRequest;
	Network::GetAppSignalStatesFromArchiveStartReply m_startReply;

	Network::GetAppSignalStatesFromArchiveNextRequest m_nextRequest;
	Network::GetAppSignalStatesFromArchiveNextReply m_nextReply;
};

#endif // TRENDTCPCLIENT_H
