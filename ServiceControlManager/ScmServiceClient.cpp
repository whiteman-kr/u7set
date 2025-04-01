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
	connect(this, &ScmServiceClient::requestEnqueued, this, &ScmServiceClient::sendSrvGetInfoRequest);

	m_timer = new QTimer;
	connect(m_timer, &QTimer::timeout, this, &ScmServiceClient::sendSrvGetInfoRequest);
	m_timer->start(500);
}

void ScmServiceClient::onClientThreadFinished()
{
	delete m_timer;
}

void ScmServiceClient::onConnection()
{
	sendSrvGetInfoRequest();
}

void ScmServiceClient::onDisconnection()
{
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
		checkRequestQueue();
		break;

	case RQID_SERVICE_START:
	case RQID_SERVICE_STOP:
	case RQID_SERVICE_RESTART:
		sendSrvGetInfoRequest();
		break;

	default:
		assert(false);
	}
}

void ScmServiceClient::onGetServiceInfo(const char* replyData, quint32 replyDataSize)
{
	emit serviceInfoUpdated(QByteArray(replyData, replyDataSize));
}

void ScmServiceClient::enqueueRequest(int requestID)
{
	m_requestQueueMutex.lock();

	m_requestQueue.push(requestID);

	m_requestQueueMutex.unlock();

	emit requestEnqueued();
}

void ScmServiceClient::checkRequestQueue()
{
	bool queueEmpty = false;

	m_requestQueueMutex.lock();

	queueEmpty = m_requestQueue.empty();

	m_requestQueueMutex.unlock();

	if (queueEmpty == false)
	{
		sendSrvGetInfoRequest();
	}
}

void ScmServiceClient::sendSrvGetInfoRequest()
{
	if (isClearToSendRequest())
	{
		int request = RQID_SERVICE_GET_INFO;

		m_requestQueueMutex.lock();

		if (m_requestQueue.empty() == false)
		{
			request = m_requestQueue.front();
			m_requestQueue.pop();
		}

		m_requestQueueMutex.unlock();

		sendRequest(request);
	}
}
