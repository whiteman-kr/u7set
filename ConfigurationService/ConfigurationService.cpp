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
													   const VersionInfo& versionInfo,
													   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(ServiceType::ConfigurationService, serviceName, argc, argv, versionInfo, logger),
	m_logger(logger)
{
}


ServiceWorker* ConfigurationServiceWorker::createInstance() const
{
	ConfigurationServiceWorker* newInstance = new ConfigurationServiceWorker(serviceName(), argc(), argv(), versionInfo(), m_logger);
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
	cp.addSingleValueOption("b", "Path to RPCT project's build  for auto load.", "PathToBuild");
	cp.addSingleValueOption("ip", "Client request IP.", "IPv4");
	cp.addSingleValueOption("w", "Work directory of Configuration Service.", "Path");
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
		setStrSetting("AutoloadBuildPath", cp.optionValue("b"));
	}

	if (cp.optionIsSet("ip") == true)
	{
		setStrSetting("ClientRequestIP", cp.optionValue("ip"));
	}

	if (cp.optionIsSet("w") == true)
	{
		setStrSetting("WorkDirectory", cp.optionValue("w"));
	}
}


void ConfigurationServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting("EquipmentID");
	m_autoloadBuildPath = getStrSetting("AutoloadBuildPath");
	m_clientIPStr = getStrSetting("ClientRequestIP");
	m_workDirectory = getStrSetting("WorkDirectory");

	m_clientIP = HostAddressPort(m_clientIPStr, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString("Load settings:"));
	DEBUG_LOG_MSG(m_logger, QString("EquipmentID = %1").arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString("AutoloadBuildPath = %1").arg(m_autoloadBuildPath));
	DEBUG_LOG_MSG(m_logger, QString("ClientRequestIP = %1 (%2)").arg(m_clientIPStr).arg(m_clientIP.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString("WorkDirectory = %1").arg(m_workDirectory));
}


void ConfigurationServiceWorker::initialize()
{
	startCfgServerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is initialized")));
}


void ConfigurationServiceWorker::shutdown()
{
	stopCfgServerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is shutting down")));
}


void ConfigurationServiceWorker::startCfgServerThread()
{
	CfgServer* cfgServer = new CfgServer(m_autoloadBuildPath, m_logger);

	Tcp::Listener* listener = new Tcp::Listener(m_clientIP, cfgServer, m_logger);

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
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_CONFIGURATION_SERVICE_INFO, m_logger);

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


