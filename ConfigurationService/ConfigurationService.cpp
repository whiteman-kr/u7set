#include <QXmlStreamReader>
#include <QMetaProperty>

#include "ConfigurationService.h"
#include "CfgChecker.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

const char* const ConfigurationServiceWorker::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const ConfigurationServiceWorker::SETTING_AUTOLOAD_BUILD_PATH = "AutoloadBuildPath";
const char* const ConfigurationServiceWorker::SETTING_CLIENT_REQUEST_IP = "ClientRequestIP";
const char* const ConfigurationServiceWorker::SETTING_WORK_DIRECTORY = "WorkDirectory";


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

	newInstance->init();

	return newInstance;
}


void ConfigurationServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	serviceInfo.set_clientrequestip(m_clientIP.address32());
	serviceInfo.set_clientrequestport(m_clientIP.port());
}


void ConfigurationServiceWorker::onBuildPathChanged(QString newBuildPath)
{
	stopCfgServerThread();

	emit renameWorkBuildToBackupExcept(newBuildPath);

	startCfgServerThread(newBuildPath);
}



void ConfigurationServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("b", SETTING_AUTOLOAD_BUILD_PATH, "Path to RPCT project's build  for auto load.", "PathToBuild");
	cp.addSingleValueOption("ip", SETTING_CLIENT_REQUEST_IP, "Client request IP.", "IPv4");
	cp.addSingleValueOption("w", SETTING_WORK_DIRECTORY, "Work directory of Configuration Service.", "Path");
}

void ConfigurationServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting(SETTING_EQUIPMENT_ID);
	m_autoloadBuildPath = getStrSetting(SETTING_AUTOLOAD_BUILD_PATH);
	m_clientIPStr = getStrSetting(SETTING_CLIENT_REQUEST_IP);
	m_workDirectory = getStrSetting(SETTING_WORK_DIRECTORY);

	m_clientIP = HostAddressPort(m_clientIPStr, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString("Load settings:"));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SETTING_EQUIPMENT_ID).arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SETTING_AUTOLOAD_BUILD_PATH).arg(m_autoloadBuildPath));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SETTING_CLIENT_REQUEST_IP).arg(m_clientIP.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SETTING_WORK_DIRECTORY).arg(m_workDirectory));
}


void ConfigurationServiceWorker::initialize()
{
	startCfgCheckerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is initialized")));
}


void ConfigurationServiceWorker::shutdown()
{
	stopCfgCheckerThread();

	stopCfgServerThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ServiceWorker is shutting down")));
}


void ConfigurationServiceWorker::startCfgServerThread(const QString& buildPath)
{
	CfgControlServer* cfgControlServer = new CfgControlServer(m_equipmentID, m_autoloadBuildPath, m_workDirectory, buildPath, *m_cfgCheckerWorker, m_logger);

	Tcp::Listener* listener = new Tcp::Listener(m_clientIP, cfgControlServer, m_logger);

	connect(listener, &Tcp::Listener::connectedClientsDataChanged, cfgControlServer, &CfgControlServer::updateClientsInfo);

	m_cfgServerThread = new Tcp::ServerThread(listener);

	m_cfgServerThread->start();
}


void ConfigurationServiceWorker::stopCfgServerThread()
{
	if (m_cfgServerThread != nullptr)
	{
		m_cfgServerThread->quit();

		delete m_cfgServerThread;

		m_cfgServerThread = nullptr;
	}
}


void ConfigurationServiceWorker::startCfgCheckerThread()
{
	m_cfgCheckerWorker = new CfgCheckerWorker(m_workDirectory, m_autoloadBuildPath, 3 * 1000, m_logger);

	m_cfgCheckerThread = new SimpleThread(m_cfgCheckerWorker);

	m_cfgCheckerThread->start();

	connect(m_cfgCheckerWorker, &CfgCheckerWorker::buildPathChanged, this, &ConfigurationServiceWorker::onBuildPathChanged);
	connect(this, &ConfigurationServiceWorker::renameWorkBuildToBackupExcept, m_cfgCheckerWorker, &CfgCheckerWorker::renameWorkToBackup);
}


void ConfigurationServiceWorker::stopCfgCheckerThread()
{
	assert(m_cfgCheckerThread != nullptr);

	m_cfgCheckerThread->quit();
	delete m_cfgCheckerThread;

	m_cfgCheckerWorker = nullptr;
}


void ConfigurationServiceWorker::startUdpThreads()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_CONFIGURATION_SERVICE_INFO, m_logger);

	m_infoSocketThread = new UdpSocketThread(serverSocket);

	m_infoSocketThread->start();
}


void ConfigurationServiceWorker::stopUdpThreads()
{
	m_infoSocketThread->quitAndWait();

	delete m_infoSocketThread;
}
