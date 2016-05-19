#include "TcpSignalClient.h"

TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
	qDebug() << "TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)";
}

TcpSignalClient::~TcpSignalClient()
{
	qDebug() << "TcpSignalClient::~TcpSignalClient()";
}

void TcpSignalClient::onClientThreadStarted()
{
	qDebug() << "TcpSignalClient::onClientThreadStarted()";
}

void TcpSignalClient::onClientThreadFinished()
{
	qDebug() << "TcpSignalClient::onClientThreadFinished()";
}

void TcpSignalClient::onConnection()
{
	qDebug() << "TcpSignalClient::onConnection()";
}

void TcpSignalClient::onDisconnection()
{
	qDebug() << "TcpSignalClient::onDisconnection";
}

void TcpSignalClient::onReplyTimeout()
{
	qDebug() << "TcpSignalClient::onReplyTimeout()";
}

void TcpSignalClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	switch (requestID)
	{
	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpSignalClient::processReply()";
	}

	return;
}
