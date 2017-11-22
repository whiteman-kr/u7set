#ifndef ARCHIVETCPCLIENT_H
#define ARCHIVETCPCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../lib/AppSignal.h"
#include "../Proto/network.pb.h"
#include "MonitorConfigController.h"
#include "ArchiveData.h"


struct ArchiveRequest
{
	TimeStamp startTime;
	TimeStamp endTime;
	E::TimeType timeType;
	bool removePrioodicRecords;
	std::map<Hash, QString> appSignals;
};


// Getting data freom archive, can process only one request in a time
//
class ArchiveTcpClient : public Tcp::Client
{
	Q_OBJECT

public:
	ArchiveTcpClient(MonitorConfigController* configController);
	virtual ~ArchiveTcpClient();

	// Methods
	//
public:
	bool requestData(TimeStamp startTime,
					 TimeStamp endTime,
					 E::TimeType timeType,
					 bool removePeriodicRecords,
					 const std::vector<AppSignalParam>& appSignals);
	bool cancelRequest();
	bool isRequestInProgress() const;

	// Internals
	//
protected:
	virtual void timerEvent(QTimerEvent* event) override;

	void emitErrorResetState(QString errorMessage);
	void resetState();

protected:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void requestStart();
	void processStart(const QByteArray& data);

	void requestNext();
	void processNext(const QByteArray& data);

	void requestCancel();
	void processCancel(const QByteArray& data);

signals:
	void signal_startRequest();
	void signal_cancelRequest();

	void dataReady(std::shared_ptr<ArchiveChunk> chunk);

	void requestError(QString errorMessage);
	void statusUpdate(QString status, int statesReceived, int requestCount, int repliesCount);
	void requestIsFinished();

public slots:
//	void slot_requestData(QString appSignalId, TimeStamp hourToRequest, TimeType timeType);

protected slots:
	void slot_startRequest();
	void slot_cancelRequest();

	void slot_configurationArrived(ConfigSettings configuration);

	// Data
	//
private:
	MonitorConfigController* m_cfgController = nullptr;
	int m_periodicTimerId = 0;

	// State
	//
private:
	volatile bool m_requestInProgress = false;
	int m_currentRequestId = 0;
	ArchiveRequest m_requestData;
	bool m_needCancelRequest = false;

	QTime m_startRequestTime;

	// Protobufer messages
	//
private:
	Network::GetAppSignalStatesFromArchiveStartRequest m_startRequest;
	Network::GetAppSignalStatesFromArchiveStartReply m_startReply;

	Network::GetAppSignalStatesFromArchiveNextRequest m_nextRequest;
	Network::GetAppSignalStatesFromArchiveNextReply m_nextReply;

	Network::GetAppSignalStatesFromArchiveCancelRequest m_cancelRequest;
	Network::GetAppSignalStatesFromArchiveCancelReply m_cancelReply;

	// Statisctics
	//
private:
	QString m_statRequestDescription = 0;
	int m_statStateReceived = 0;
	int m_statTcpRequestCount = 0;
	int m_statTcpReplyCount = 0;
};

#endif // ARCHIVETCPCLIENT_H
