#include "SignalSocket.h"

#include <assert.h>

#include "Options.h"
#include "SignalBase.h"

#include "version.h"

// -------------------------------------------------------------------------------------------------------------------

SignalSocket::SignalSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalSocket::SignalSocket(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalSocket::~SignalSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onClientThreadStarted()
{
	qDebug() << "SignalSocket::onClientThreadStarted()";
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onClientThreadFinished()
{
	qDebug() << "SignalSocket::onClientThreadFinished()";
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onConnection()
{
	qDebug() << "SignalSocket::onConnection()";

	emit socketConnected();

	requestSignalState();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onDisconnection()
{
	qDebug() << "SignalSocket::onDisconnection";

	emit socketDisconnected();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	if(requestID == ADS_GET_APP_SIGNAL_STATE)
	{
		replySignalState(replyData, replyDataSize);
	}
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_APP_SIGNAL_STATE

void SignalSocket::requestSignalState()
{
	assert(isClearToSendRequest());

	// according to GOST MI-2002 in each point we need do twenty measurements per one second
	// 50 ms
	//
	QThread::msleep(SIGNAL_SOCKET_TIMEOUT_STATE);

	int hashCount = theSignalBase.hashForRequestStateCount();

	m_getSignalStateRequest.mutable_signalhashes()->Clear();
	m_getSignalStateRequest.mutable_signalhashes()->Reserve(SIGNAL_SOCKET_MAX_READ_SIGNAL);

	int startIndex = m_signalStateRequestIndex;

	for (int i = 0; SIGNAL_SOCKET_MAX_READ_SIGNAL; i++)
	{
		if (m_signalStateRequestIndex >= hashCount)
		{
			m_signalStateRequestIndex = 0;
			break;
		}

		Hash hash = theSignalBase.hashForRequestState(i + startIndex);
		if (hash == 0)
		{
			assert(hash != 0);
			continue;
		}

		m_getSignalStateRequest.add_signalhashes(hash);

		m_signalStateRequestIndex++;
	}

	sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replySignalState(const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		requestSignalState();
		return;
	}

	bool result = m_getSignalStateReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
	if (result == false)
	{
		qDebug() << "SignalSocket::replySignalState - error: ParseFromArray";
		assert(result);
		requestSignalState();
		return;
	}

	if (m_getSignalStateReply.error() != 0)
	{
		qDebug() << "SignalSocket::replySignalState - error: " << m_getSignalStateReply.error();
		assert(m_getSignalStateReply.error() != 0);
		requestSignalState();
		return;
	}

	for (int i = 0; i < m_getSignalStateReply.appsignalstates_size(); i++)
	{
		Hash hash = m_getSignalStateReply.appsignalstates(i).hash();
		if (hash == 0)
		{
			assert(hash != 0);
			continue;
		}

		AppSignalState appState;
		appState.load(m_getSignalStateReply.appsignalstates(i));

		//appState.m_flags.valid = 1;
		//appState.m_value = 2.5;

		Metrology::SignalState state(appState);
		theSignalBase.setSignalState(hash, state);
	}

	requestSignalState();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::configurationLoaded()
{
	HostAddressPort addr1 = theOptions.socket().client(SOCKET_TYPE_SIGNAL).address(SOCKET_SERVER_TYPE_PRIMARY);
	HostAddressPort addr2 = theOptions.socket().client(SOCKET_TYPE_SIGNAL).address(SOCKET_SERVER_TYPE_RESERVE);

	HostAddressPort currAddr1 = serverAddressPort(SOCKET_SERVER_TYPE_PRIMARY);
	HostAddressPort currAddr2 = serverAddressPort(SOCKET_SERVER_TYPE_RESERVE);

	if (	addr1.address32() == currAddr1.address32() && addr1.port() == currAddr1.port() &&
			addr2.address32() == currAddr2.address32() && addr2.port() == currAddr2.port())
	{
		return;
	}

	setServers(addr1, addr2, true);
}

// -------------------------------------------------------------------------------------------------------------------
