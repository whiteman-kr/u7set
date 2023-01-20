#ifndef ARCHIVETCPCLIENT_H
#define ARCHIVETCPCLIENT_H
/*
#include "../OnlineLib/Tcp.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "../Proto/network.pb.h"
#include "MonitorConfigController.h"
#include "ArchiveData.h"
#include "../OnlineLib/TcpClientStatistics.h"

namespace AAA
{


struct ArchiveRequest
{
	TimeStamp startTime;
	TimeStamp endTime;
	E::TimeType timeType;
	bool removePrioodicRecords;
	std::map<Hash, QString> appSignals;
};

struct ArchiveRequestResult
{

};

// Getting data from archive
// Single use thread, it can procces only one request
// Subscribe to signals:
//
class ArchiveTcpClient : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	ArchiveTcpClient(const ArchiveSource& request,
					 QPromise<ArchiveRequestResult>&& promise,
					 const SoftwareInfo& softwareInfo,
					 const MonitorSettings::ArchiveService& archiveService,
					 ILogFile* logFile);
	virtual ~ArchiveTcpClient();

	// Methods
	//
private:
	bool setRequestData(const ArchiveSource& request);

public:
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

	virtual void onTryConnectToServer(const HostAddressPort& serverAddr) override;
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
	//void signal_connectionEstablished();
	//void signal_startRequest();
	void signal_cancelRequest();

	void dataReady(std::shared_ptr<ArchiveChunk> chunk);

	void requestError(QString errorMessage);
	void statusUpdate(QString status, int statesReceived, int requestCount, int repliesCount);
	void requestIsFinished();

public slots:
//	void slot_requestData(QString appSignalId, TimeStamp hourToRequest, TimeType timeType);

protected slots:
	//void slot_startRequest();
	void slot_cancelRequest();

	// Data
	//
private:
	HasLogFile m_logFile;
	int m_periodicTimerId = 0;

	// State
	//
private:
	std::atomic<bool> m_requestInProgress{false};
	quint32 m_currentRequestId = 0;
	ArchiveRequest m_requestData;
	bool m_needCancelRequest = false;
	int m_tryToConnectCounter = 5;	// Try to connect to server 5 times, if connection was not established, then
									// report error and stop any attempts to connect

	QElapsedTimer m_startRequestTime;

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
	QString m_statRequestDescription;
	int m_statStateReceived = 0;
	int m_statTcpRequestCount = 0;
	int m_statTcpReplyCount = 0;
};


}
*/
#endif // ARCHIVETCPCLIENT_H
