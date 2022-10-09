#pragma once
#include <future>
#include "../../lib/SoftwareSettings.h"
#include "../../CommonLib/Times.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "ArchiveData.h"

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
	std::deque<AppSignalState> states;
};

// Request data from archive service
//	Any object state and control via QFuture
//	Note: One time use object
//
class ArchiveTcpClient2 : public Tcp::Client, public TcpClientStatistics
{
public:
	ArchiveTcpClient2(const ArchiveSource& request,
					  const SoftwareInfo& softwareInfo,
					  const MonitorSettings::ArchiveService& archiveService,
					  ILogFile* logFile);

	virtual ~ArchiveTcpClient2();

public:
	// Take result or exception via this future object.
	//
	[[nodiscard]] std::future<ArchiveRequestResult> future();
	void cancelRequest();

private:
	void setRequestData(const ArchiveSource& request);

	// Tcp::Client implementation
	//
private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onTryConnectToServer(const HostAddressPort& serverAddr) override;
	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	void finish(QString error = QString{});

private:
	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void requestStart();
	void processStart(const QByteArray& data);

	void requestNext();
	void processNext(const QByteArray& data);

	void requestCancel();
	void processCancel(const QByteArray& data);

	// Data
	//
private:
	HasLogFile m_logFile;
	MonitorSettings::ArchiveService m_serverSettings;

	std::promise<ArchiveRequestResult> m_promise;
	std::atomic<bool> m_needCancelRequest{false};

	// State
	//
	ArchiveRequest m_requestData;
	ArchiveRequestResult m_result;

	quint32 m_currentRequestId = 0;
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
