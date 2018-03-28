#pragma once

#include "../lib/Tcp.h"
#include "../lib/AppSignal.h"
#include "../Proto/network.pb.h"

#include "SignalStatesProcessingThread.h"

class TcpArchiveClient : public Tcp::Client
{
public:
	TcpArchiveClient(const SoftwareInfo& softwareInfo,
					 const HostAddressPort& serverAddressPort,
					 SignalStatesProcessingThread* signalStatesProcessingThread,
					 CircularLoggerShared logger);

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;

	bool sendSignalStatesToArchiveRequest(bool sendNow);
	void onSaveAppSignalsStatesReply(const char* replyData, quint32 replyDataSize);

private slots:
	void onTimer();
	void onSignalStatesQueueIsNotEmpty();

private:
	SignalStatesProcessingThread* m_signalStatesProcessingThread = nullptr;
	CircularLoggerShared m_logger;

	AppSignalStatesQueueShared m_signalStatesQueue;

	QTimer m_timer;

	int m_connectionKeepAliveCounter = 0;

	qint64 m_saveAppSignalsStateErrorReplyCount = 0;
};


class TcpArchiveClientThread : public SimpleThread
{
public:
	TcpArchiveClientThread(TcpArchiveClient* tcpArchiveClient);

	Tcp::ConnectionState getConnectionState();

private:

	virtual void beforeQuit() override;

private:
	TcpArchiveClient* m_tcpArchiveClient = nullptr;
	Tcp::ConnectionState m_dummyState;
};
