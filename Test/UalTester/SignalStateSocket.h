#ifndef SIGNALSOCKET_H
#define SIGNALSOCKET_H


#include "../../lib/Tcp.h"
#include "../../lib/SocketIO.h"
#include "../../lib/Hash.h"
#include "../../lib/AppSignal.h"

#include "../../Proto/network.pb.h"

// ==============================================================================================

const int			SIGNAL_SOCKET_TIMEOUT_STATE	= 50;  // 50 ms

// ==============================================================================================

const int			SIGNAL_SOCKET_MAX_READ_SIGNAL = 100;

// ==============================================================================================

class SignalStateSocket : public Tcp::Client
{
	 Q_OBJECT

public:

	SignalStateSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort);

	SignalStateSocket(const SoftwareInfo& softwareInfo,
				 const HostAddressPort& serverAddressPort1,
				 const HostAddressPort& serverAddressPort2);

	virtual ~SignalStateSocket();

private:

	// protobuf messages
	//

	Network::GetAppSignalStateRequest	m_getSignalStateRequest;							// ADS_GET_APP_SIGNAL_STATE
	Network::GetAppSignalStateReply		m_getSignalStateReply;

	int									m_signalStateRequestIndex = 0;

	// functions: Request - Reply
	//
	void			requestSignalState();													// ADS_GET_APP_SIGNAL_STATE
	void			replySignalState(const char* replyData, quint32 replyDataSize);

public:

	virtual void	onClientThreadStarted() override;
	virtual void	onClientThreadFinished() override;

	virtual void	onConnection() override;
	virtual void	onDisconnection() override;

	virtual void	processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;		// for processing functions: Request - Reply

signals:

	void			socketConnected();
	void			socketDisconnected();
	void			signalStateReceived();

public slots:

};

// ==============================================================================================

#endif // SIGNALSOCKET_H
