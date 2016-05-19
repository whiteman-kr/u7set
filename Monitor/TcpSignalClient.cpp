#include "TcpSignalClient.h"
#include "Settings.h"

TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
	qDebug() << "TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)";

	m_startStateTimerId = startTimer(theSettings.requestTimeInterval());

	reset();
}

TcpSignalClient::~TcpSignalClient()
{
	qDebug() << "TcpSignalClient::~TcpSignalClient()";
}

void TcpSignalClient::timerEvent(QTimerEvent* event)
{
	assert(event);
	return;
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

	assert(isClearToSendRequest() == true);

	requestSignalListStart();

	return;
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

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	case ADS_GET_SIGNAL_LIST_START:
		processSignalListStart(data);
		break;

	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpSignalClient::processReply()";

		reset();
	}

	return;
}

void TcpSignalClient::reset()
{
	m_state = State::Start;
}

void TcpSignalClient::requestSignalListStart()
{
	sendRequest(ADS_GET_SIGNAL_LIST_START);
}

void TcpSignalClient::processSignalListStart(const QByteArray& data)
{
	qDebug() << data.size();
}

void TcpSignalClient::requestSignalListNext()
{

}

void TcpSignalClient::replySignalListNext()
{

}
