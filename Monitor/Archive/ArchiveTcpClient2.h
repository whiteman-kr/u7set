#pragma once
#include <CommonLib/Times.h>
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../OnlineLib/Tcp.h"
#include "../../OnlineLib/TcpClientStatistics.h"
#include "ArchiveData.h"

struct ArchiveRequest
{
	TimeStamp startTime;
	TimeStamp endTime;
	E::TimeType timeType;
	bool removePrioodicRecords;
	std::map<Hash, QString> appSignals;		// Key is signal hash, value is appSignalId
};


// Request data from archive service
//		Note: One time use object
//
class ArchiveTcpClient2 : public Tcp::Client, public TcpClientStatistics
{
	Q_OBJECT

public:
	ArchiveTcpClient2(const ArchiveSource& request,
					  const SoftwareInfo& softwareInfo,
					  const SoftwareEndpoint::ArchiveService& archiveService,
					  ILogFile* logFile);

	virtual ~ArchiveTcpClient2();

public:
	/// Call to cancel request.
	///
	void cancelRequest();

signals:
	/// Report connection statistics.
	void statistics(QString archServiceShortId, QString state, int requests, int replies, int states);

	/// Reports if data ready or error occured.
	void dataReady(std::shared_ptr<ArchiveRequestResult> result, QString error);

private:
	void setRequestData(const ArchiveSource& request);

	void finish(QString error = QString{});

	// Tcp::Client implementation
	//
private:
	virtual void onTryConnectToServer(const HostAddressPort& serverAddr) override;
	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

private:
	void requestStart();
	void processStart(const QByteArray& data);

	void requestNext();
	void processNext(const QByteArray& data);

	void requestCancel();
	void processCancel(const QByteArray& data);

private:
	void updateStatistics(QString state);
	void updateStatistics(int incRequests, int incReplies, int incStates);

	// Data
	//
private:
	HasLogFile m_logFile;
	SoftwareEndpoint::ArchiveService m_serverSettings;

	// State
	//
	std::atomic<bool> m_needCancelRequest{false};

	ArchiveRequest m_requestData;
	ArchiveRequestResult m_result;

	quint32 m_currentRequestId = 0;
	int m_tryToConnectCounter = 3;	// Try to connect to server 3 times, if connection was not established, then
									// report error and stop any attempts to connect
	QElapsedTimer m_startRequestTime;

	// Protobufer messages for use and reuse
	//
private:
	Network::GetAppSignalStatesFromArchiveNextRequest m_nextRequest;
	Network::GetAppSignalStatesFromArchiveNextReply m_nextReply;

	// Statisctics
	//
private:
	QMutex m_statMutex;
	QString m_statState;
	int m_statStateReceived = 0;
	int m_statTcpRequestCount = 0;
	int m_statTcpReplyCount = 0;
};
