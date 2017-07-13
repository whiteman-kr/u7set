#pragma once

#include "../lib/Tcp.h"
#include "../lib/AppSignal.h"

class TcpArchiveClient : public Tcp::Client
{
public:
	TcpArchiveClient(int channel,
					 const HostAddressPort& serverAddressPort,
					 E::SoftwareType softwareType,
					 const QString equipmentID,
					 int majorVersion,
					 int minorVersion,
					 int commitNo,
					 CircularLoggerShared logger,
					 AppSignalStatesQueue& signalStatesQueue);

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	void sendSignalStatesToArchive();

private slots:
	void onTimer();
	void onSignalStatesQueueIsNotEmpty();

private:
	int m_channel = -1;

	CircularLoggerShared m_logger;

	AppSignalStatesQueue& m_signalStatesQueue;

	QTimer m_timer;
};

