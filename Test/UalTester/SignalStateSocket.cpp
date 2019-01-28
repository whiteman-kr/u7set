#include "SignalStateSocket.h"

#include <assert.h>

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
//
// SignalStateSocket class implementation
//
// -------------------------------------------------------------------------------------------------------------------

SignalStateSocket::SignalStateSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort)
{
}

SignalStateSocket::SignalStateSocket(const SoftwareInfo& softwareInfo,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2)
{
}

SignalStateSocket::~SignalStateSocket()
{
}

void SignalStateSocket::onClientThreadStarted()
{
	 //qDebug() << "SignalStateSocket::onClientThreadStarted()";
}

void SignalStateSocket::onClientThreadFinished()
{
	 //qDebug() << "SignalStateSocket::onClientThreadFinished()";
}

void SignalStateSocket::onConnection()
{
	//qDebug() << "SignalStateSocket::onConnection()";

	emit socketConnected();

	requestSignalState();

	return;
}

void SignalStateSocket::onDisconnection()
{
	//qDebug() << "SignalStateSocket::onDisconnection";

	emit socketDisconnected();
}

void SignalStateSocket::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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

void SignalStateSocket::requestSignalState()
{
	assert(isClearToSendRequest());

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

void SignalStateSocket::replySignalState(const char* replyData, quint32 replyDataSize)
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
		//qDebug() << "SignalStateSocket::replySignalState - error: ParseFromArray";
		assert(result);
		requestSignalState();
		return;
	}

	if (m_getSignalStateReply.error() != 0)
	{
		//qDebug() << "SignalStateSocket::replySignalState - error: " << m_getSignalStateReply.error();
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

		 theSignalBase.setSignalState(hash, appState);
	}

	requestSignalState();
}
