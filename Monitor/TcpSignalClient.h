#ifndef TCPSIGNALCLIENT_H
#define TCPSIGNALCLIENT_H

#include "../include/Tcp.h"


class TcpSignalClient : public Tcp::Client
{
public:
	TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpSignalClient();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;
};

#endif // TCPSIGNALCLIENT_H
