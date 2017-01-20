#include <QXmlStreamReader>
#include <QMetaProperty>

#include "ConfigurationService.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

ConfigurationServiceWorker::ConfigurationServiceWorker(const QString& serviceName,
													   int& argc, char** argv,
													   const VersionInfo& versionInfo) :
	ServiceWorker(ServiceType::ConfigurationService, serviceName, argc, argv, versionInfo)
{
}


ServiceWorker* ConfigurationServiceWorker::createInstance() const
{
	ConfigurationServiceWorker* newInstance = new ConfigurationServiceWorker(serviceName(), argc(), argv(), versionInfo());
	return newInstance;
}


void ConfigurationServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo)
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


void ConfigurationServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("b", "Path to RPCT project build.", "PathToBuild");
	cp.addSingleValueOption("ip", "Client request IP.", "IPv4");
}


void ConfigurationServiceWorker::processCmdLineSettings()
{
	CommandLineParser& cp = cmdLineParser();

	if (cp.optionIsSet("id") == true)
	{
		setStrSetting("EquipmentID", cp.optionValue("id"));
	}

	if (cp.optionIsSet("b") == true)
	{
		setStrSetting("BuildPath", cp.optionValue("b"));
	}

	if (cp.optionIsSet("ip") == true)
	{
		setStrSetting("ClientRequestIP", cp.optionValue("ip"));
	}
}


void ConfigurationServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting("EquipmentID");
	m_buildPath = getStrSetting("BuildPath");
	m_clientIPStr = getStrSetting("ClientRequestIP");

	m_clientIP = HostAddressPort(m_clientIPStr, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(QString(tr("Load settings:")));
	DEBUG_LOG_MSG(QString(tr("%1 = %2")).arg("EquipmentID").arg(m_equipmentID));
	DEBUG_LOG_MSG(QString(tr("%1 = %2")).arg("BuildPath").arg(m_buildPath));
	DEBUG_LOG_MSG(QString(tr("%1 = %2 (%3)")).arg("ClientRequestIP").arg(m_clientIPStr).arg(m_clientIP.addressPortStr()));
}


void ConfigurationServiceWorker::initialize()
{
	startCfgServerThread();

	DEBUG_LOG_MSG(QString(tr("ServiceWorker is initialized")));
}


void ConfigurationServiceWorker::shutdown()
{
	stopCfgServerThread();

	DEBUG_LOG_MSG(QString(tr("ServiceWorker is shutting down")));
}


void ConfigurationServiceWorker::startCfgServerThread()
{
	CfgServerListener* listener = new CfgServerListener(m_clientIP, new CfgServer(m_buildPath));

	m_cfgServerThread = new Tcp::ServerThread(listener);

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


// ------------------------------------------------------------------------------------
//
// CfgServerListener class implementation
//
// ------------------------------------------------------------------------------------

CfgServerListener::CfgServerListener(const HostAddressPort& listenAddressPort, Tcp::Server* server) :
	Tcp::Listener(listenAddressPort, server)
{
}


void CfgServerListener::onNewConnectionAccepted(const HostAddressPort& peerAddr, int connectionNo)
{
	DEBUG_LOG_MSG(QString(tr("CfgServer accept new connection #%1 from %2")).arg(connectionNo).arg(peerAddr.addressStr()));
}


void CfgServerListener::onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr)
{
	if (startOk)
	{
		DEBUG_LOG_MSG(QString("CfgServer start listening %1 OK").arg(addr.addressPortStr()));
	}
	else
	{
		DEBUG_LOG_ERR(QString("CfgServer error on start listening %1: %2").arg(addr.addressPortStr()).arg(errStr));
	}
}




