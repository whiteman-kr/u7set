#pragma once

#include "../OnlineLib/Tcp.h"
#include "../AppSignalLib/SimpleAppSignalState.h"

class AppDataServiceWorker;

class TcpArchiveClient : public Tcp::Client
{
public:
	TcpArchiveClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& archiveSrviceAddressPort,
					 AppDataServiceWorker& appDataService);

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	bool sendSignalStatesToArchiveRequest(bool sendNow);
	void onSaveAppSignalsStatesReply(const char* replyData, quint32 replyDataSize);

private slots:
	void onTimer();
	void onSignalStatesQueueIsNotEmpty();

private:
	AppDataServiceWorker& m_appDataService;
	CircularLoggerShared m_logger;

	SimpleAppSignalStatesQueueShared m_signalStatesQueue;

	QTimer m_timer;

	qint64 m_saveAppSignalsStateErrorReplyCount = 0;
};

class TcpArchiveClientThread : public SimpleThread
{
public:
	TcpArchiveClientThread(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort,
						   AppDataServiceWorker& appDataService);

	Tcp::ConnectionState getConnectionState();

private:
	TcpArchiveClient* m_tcpArchiveClient = nullptr;
	static Tcp::ConnectionState m_emptyState;
};
