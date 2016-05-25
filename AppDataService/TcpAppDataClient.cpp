#include "TcpAppDataClient.h"


TcpAppDataClient::TcpAppDataClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
}


TcpAppDataClient::~TcpAppDataClient()
{
}


void TcpAppDataClient::onClientThreadStarted()
{

}


void TcpAppDataClient::onClientThreadFinished()
{

}


void TcpAppDataClient::onConnection()
{
	init();

	sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}


void TcpAppDataClient::onDisconnection()
{

}


void TcpAppDataClient::onReplyTimeout()
{

}


void TcpAppDataClient::init()
{
	m_signalHahes.clear();

	m_totalItemsCount = 0;
	m_partCount = 0;
	m_itemsPerPart = 0;
	m_currentPart = 0;
}


void TcpAppDataClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case ADS_GET_APP_SIGNAL_LIST_START:
		onGetAppSignalListStartReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		onGetAppSignalListNextReply(replyData, replyDataSize);
		break;


	default:
		assert(false);
	}
}


void TcpAppDataClient::onGetAppSignalListStartReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalListStartReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_totalItemsCount = m_getSignalListStartReply.totalitemcount();
	m_partCount = m_getSignalListStartReply.partcount();
	m_itemsPerPart = m_getSignalListStartReply.itemsperpart();

	if (m_partCount > 0)
	{
		m_currentPart = 0;
		getNextItemsPart();
	}
}


void TcpAppDataClient::getNextItemsPart()
{
	if (m_currentPart >= m_partCount)
	{
		assert(false);
		return;
	}

	m_getSignalListNextRequest.Clear();

	m_getSignalListNextRequest.set_part(m_currentPart);

	sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, m_getSignalListNextRequest);
}


void TcpAppDataClient::onGetAppSignalListNextReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalListNextReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	int stringCount = m_getSignalListNextReply.appsignalids_size();

	for(int i = 0; i < stringCount; i++)
	{
		Hash hash = calcHash(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));

		m_signalHahes.append(hash);
	}

	m_currentPart++;

	getNextItemsPart();
}
