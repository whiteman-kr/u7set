#include "ScmServiceClient.h"
#include "../OnlineLib/SocketIO.h"


ScmServiceClient::ScmServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort, "TcpConfigServiceClient")
{
}

ScmServiceClient::ScmServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, "TcpConfigServiceClient")
{
}

ScmServiceClient::~ScmServiceClient()
{
}

void ScmServiceClient::onClientThreadStarted()
{

}

void ScmServiceClient::onClientThreadFinished()
{

}

void ScmServiceClient::onConnection()
{
	if (m_timer == nullptr)
	{
		m_timer = new QTimer(this);
		connect(m_timer, &QTimer::timeout, this, &ScmServiceClient::updateSrvInfo);
	}

	m_timer->start(500);

	updateSrvInfo();
}

void ScmServiceClient::onDisconnection()
{
	if (m_timer != nullptr)
	{
		m_timer->stop();
	}

	emit socketDisconnected();
}

void ScmServiceClient::onReplyTimeout()
{
}

void ScmServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case RQID_SERVICE_GET_INFO:
		onGetServiceInfo(replyData, replyDataSize);
		break;

	case RQID_SERVICE_START:
	case RQID_SERVICE_STOP:
	case RQID_SERVICE_RESTART:
		break;

	default:
		assert(false);
	}
}

void ScmServiceClient::onGetServiceInfo(const char* replyData, quint32 replyDataSize)
{
	bool result = m_srvInfo.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	emit serviceInfoUpdated(m_srvInfo);
}

void ScmServiceClient::enqueueRequest(int requestID)
{
	m_requestQueueMutex.lock();

	m_requestQueue.push(requestID);

	m_requestQueueMutex.unlock();
}

void ScmServiceClient::updateSrvInfo()
{
	if (isClearToSendRequest())
	{
		sendRequest(RQID_SERVICE_GET_INFO);
	}
}
