#include <ServiceLib/TcpSrvInfoServer.h>
#include <ServiceLib/Service.h>

TcpSrvInfoServer::TcpSrvInfoServer(const SoftwareInfo& sotwareInfo,
	const QString& serverDescription,
	Service& service) :
	Tcp::Server(sotwareInfo, serverDescription),
	m_service(service)
{
}

TcpSrvInfoServer::~TcpSrvInfoServer()
{
}

Tcp::Server* TcpSrvInfoServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
{
	Tcp::Server* newServer = new TcpSrvInfoServer(m_localSoftwareInfo, m_socketDescription, m_service);
	newServer->setListenAddress(listenAddr);
	return newServer;
}

void TcpSrvInfoServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	Q_UNUSED(requestData);
	Q_UNUSED(requestDataSize);

	switch(requestID)
	{
	case RQID_SERVICE_GET_INFO:
		{
			Network::GetServiceInfoRequest rq;

			if (rq.ParseFromArray(requestData, requestDataSize) == true)
			{
				m_service.processGetServiceInfoRequest(rq);
			}

			Network::ServiceInfo srvInfo;
			m_service.getServiceInfo(srvInfo, false);
			sendReply(srvInfo);
		}
		break;

	case RQID_SERVICE_START:
		LOG_MSG(m_service.logger(), QString("Service START request from SCM (%1).").arg(peerAddr().addressPortStr()));
		m_service.startServiceWorkerThread();
		sendReply();
		break;

	case RQID_SERVICE_STOP:
		LOG_MSG(m_service.logger(), QString("Service STOP request from SCM (%1).").arg(peerAddr().addressPortStr()));
		m_service.stopServiceWorkerThread();
		sendReply();
		break;

	case RQID_SERVICE_RESTART:
		LOG_MSG(m_service.logger(), QString("Service RESTART request from SCM (%1).").arg(peerAddr().addressPortStr()));
		m_service.stopServiceWorkerThread();
		m_service.startServiceWorkerThread();
		sendReply();
		break;

	default:
		Q_ASSERT(false);
	}
}

void TcpSrvInfoServer::onServiceGetInfo()
{
}
