#ifndef TCPSIGNALCLIENT_H
#define TCPSIGNALCLIENT_H

#include "../include/Tcp.h"


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
	void reset();

private:
	enum class State
	{
		Start,
		GetSignalList,
		GetSignalStates,
		GetStateChanges,
	};

	State m_state = State::Start;
	int m_startStateTimerId = -1;

};

#endif // TCPSIGNALCLIENT_H
