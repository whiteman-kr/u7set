#pragma once

#include "../lib/Tcp.h"

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
					 CircularLoggerShared logger);

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

private:
	int m_channel = -1;

	CircularLoggerShared m_logger;
};

