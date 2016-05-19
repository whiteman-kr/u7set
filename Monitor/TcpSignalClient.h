#ifndef TCPSIGNALCLIENT_H
#define TCPSIGNALCLIENT_H

#include <QStringList>
#include "../include/Tcp.h"
#include "../include/Hash.h"
#include "../Proto/network.pb.h"


class TcpSignalClient : public Tcp::Client
{
public:
	TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpSignalClient();

protected:
	void timerEvent(QTimerEvent *event);

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetSignalList();
	void resetToGetState();

	void requestSignalListStart();
	void processSignalListStart(const QByteArray& data);

	void requestSignalListNext(int part);
	void processSignalListNext(const QByteArray& data);

private:
	int m_startStateTimerId = -1;

private:
	// Cache protobug messages
	//
	::Network::GetSignalListStartReply m_getSignalListStartReply;

	::Network::GetSignalListNextRequest m_getSignalListNextRequest;
	::Network::GetSignalListNextReply m_getSignalListNextReply;

	QStringList m_signalList;
};

#endif // TCPSIGNALCLIENT_H
