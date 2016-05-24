#include <QXmlStreamReader>
#include <QMetaProperty>

#include "ConfigurationService.h"


// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

ConfigurationServiceWorker::ConfigurationServiceWorker(const QString &serviceStrID, const QString& buildFolder, const QString& ipStr) :
	ServiceWorker(ServiceType::ConfigurationService, serviceStrID, "", ""),
	m_buildFolder(buildFolder),
	m_clientIPStr(ipStr)
{
}


void ConfigurationServiceWorker::startCfgServerThread()
{
	m_cfgServerThread = new Tcp::ServerThread(HostAddressPort(m_clientIPStr, PORT_CONFIGURATION_SERVICE_REQUEST),
											  new CfgServer(m_buildFolder));

	m_cfgServerThread->start();
}


void ConfigurationServiceWorker::stopCfgServerThread()
{
	m_cfgServerThread->quit();

	delete m_cfgServerThread;
}


void ConfigurationServiceWorker::startUdpThreads()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_CONFIGURATION_SERVICE_INFO);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &ConfigurationServiceWorker::onInformationRequest);
	connect(this, &ConfigurationServiceWorker::ackInformationRequest, serverSocket, &UdpServerSocket::sendAck);

	m_infoSocketThread = new UdpSocketThread(serverSocket);

	m_infoSocketThread->start();
}


void ConfigurationServiceWorker::stopUdpThreads()
{
	m_infoSocketThread->quitAndWait();

	delete m_infoSocketThread;
}


void ConfigurationServiceWorker::initialize()
{
	startCfgServerThread();
	//startUdpThreads();

	qDebug() << "ConfigurationServiceWorker initialized";
}


void ConfigurationServiceWorker::shutdown()
{
	//stopUdpThreads();
	stopCfgServerThread();

	qDebug() << "ConfigurationServiceWorker stoped";
}


void ConfigurationServiceWorker::onInformationRequest(UdpRequest request)
{
	switch(request.ID())
	{
	case RQID_GET_CONFIGURATION_SERVICE_INFO:
		onGetInfo(request);
		break;

	case RQID_SET_CONFIGURATION_SERVICE_SETTINGS:
		onSetSettings(request);
		break;

	case RQID_GET_CONFIGURATION_SERVICE_SETTINGS:
		onGetSettings(request);
		break;

	default:
		assert(false);
	}
}


void ConfigurationServiceWorker::onGetInfo(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}


void ConfigurationServiceWorker::onSetSettings(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}


void ConfigurationServiceWorker::onGetSettings(UdpRequest& request)
{
	UdpRequest ack;

	ack.initAck(request);

	ackInformationRequest(ack);
}





