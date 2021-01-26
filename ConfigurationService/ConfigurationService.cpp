#include <QXmlStreamReader>
#include <QMetaProperty>

#include "ConfigurationService.h"
#include "CfgChecker.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

ConfigurationServiceWorker::ConfigurationServiceWorker(const SoftwareInfo& softwareInfo,
													   const QString& serviceName,
													   int& argc, char** argv,
													   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
	m_logger(logger)
{
}


ServiceWorker* ConfigurationServiceWorker::createInstance() const
{
	ConfigurationServiceWorker* newInstance = new ConfigurationServiceWorker(softwareInfo(),
																			 serviceName(),
																			 argc(), argv(), m_logger);

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

	bool result = loadCfgServiceSettings(newBuildPath);

	if (result == false)
	{
		return;
	}

	startCfgServerThread(newBuildPath);
}

void ConfigurationServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("b", SoftwareSetting::AUTOLOAD_BUILD_PATH, "Path to RPCT project's build  for auto load.", "PathToBuild");
	cp.addSingleValueOption("ip", SoftwareSetting::CLIENT_REQUEST_IP, "Client request IP.", "IPv4");
	cp.addSingleValueOption("w", SoftwareSetting::WORK_DIRECTORY, "Work directory of Configuration Service.", "Path");
	cp.addSingleValueOption("profile", SoftwareSetting::CURRENT_PROFILE, "Current software settings profile.", "ProfileID");
	cp.addSingleValueOption("mode", SoftwareSetting::RUN_MODE, "Runs all software in simulation mode.", SoftwareSetting::SIMULATION);
}

void ConfigurationServiceWorker::loadSettings()
{
	m_autoloadBuildPath = getStrSetting(SoftwareSetting::AUTOLOAD_BUILD_PATH);
	m_clientIPStr = getStrSetting(SoftwareSetting::CLIENT_REQUEST_IP);
	m_workDirectory = getStrSetting(SoftwareSetting::WORK_DIRECTORY);

	SessionParams sp;

	sp.currentSettingsProfile = getStrSetting(SoftwareSetting::CURRENT_PROFILE);

	if (sp.currentSettingsProfile.isEmpty() == true)
	{
		sp.currentSettingsProfile = SettingsProfile::DEFAULT;
	}

	sp.softwareRunMode = getSoftwareRunMode(getStrSetting(SoftwareSetting::RUN_MODE));

	setSessionParams(sp);

	DEBUG_LOG_MSG(m_logger, QString("Settings from command line or registry:"));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::AUTOLOAD_BUILD_PATH).arg(m_autoloadBuildPath));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::CLIENT_REQUEST_IP).arg(m_clientIPStr));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::WORK_DIRECTORY).arg(m_workDirectory));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::CURRENT_PROFILE).arg(sessionParams().currentSettingsProfile));
	DEBUG_LOG_MSG(m_logger, QString("%1 = %2").arg(SoftwareSetting::RUN_MODE).arg(E::valueToString<E::SoftwareRunMode>(sessionParams().softwareRunMode)));
	DEBUG_LOG_MSG(m_logger, QString());
}

bool ConfigurationServiceWorker::loadCfgServiceSettings(const QString& buildPath)
{
	QString cfgXmlPath = QString("%1/%2/%3").arg(buildPath).arg(equipmentID()).arg(File::CONFIGURATION_XML);

	QFile cfgXmlFile(cfgXmlPath);

	if (cfgXmlFile.open(QIODevice::ReadOnly) == false)
	{
		DEBUG_LOG_ERR(m_logger, QString("Error opening: %1").arg(cfgXmlPath));
		return false;
	}

	QByteArray cfgXmlData = cfgXmlFile.readAll();

	cfgXmlFile.close();

	bool res = softwareSettingsSet().readFromXml(cfgXmlData);

	QString curProfile = sessionParams().currentSettingsProfile;

	m_cfgServiceSettings = softwareSettingsSet().getSettingsProfile<CfgServiceSettings>(curProfile);

	if (m_cfgServiceSettings == nullptr)
	{
		DEBUG_LOG_ERR(m_logger, QString("Error loading settings for profile: %1").arg(curProfile));
		return false;
	}

	DEBUG_LOG_MSG(m_logger, QString());
	DEBUG_LOG_MSG(m_logger, QString("Loading settings for profile: %1 - Ok").arg(curProfile));
	DEBUG_LOG_MSG(m_logger, QString());

	if (m_clientIPStr.isEmpty() == true)
	{
		m_clientIP = m_cfgServiceSettings->clientRequestIP;
	}
	else
	{
		m_clientIP.setAddressPortStr(m_clientIPStr, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	}

	DEBUG_LOG_MSG(m_logger, QString("%1 is set to %2").arg(SoftwareSetting::CLIENT_REQUEST_IP).arg(m_clientIP.addressPortStr()));

	return res;
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

	CfgControlServer* cfgControlServer = new CfgControlServer(softwareInfo(),
															  m_autoloadBuildPath,
															  m_workDirectory,
															  buildPath,
															  sessionParams(),
															  m_cfgServiceSettings->knownClients(),
															  *m_cfgCheckerWorker,
															  m_logger);

	Tcp::Listener* listener = new Tcp::Listener(m_clientIP, cfgControlServer, m_logger);

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

E::SoftwareRunMode ConfigurationServiceWorker::getSoftwareRunMode(QString runModeStr)
{
	if (runModeStr.trimmed().toLower() == SoftwareSetting::SIMULATION.trimmed().toLower())
	{
		return E::SoftwareRunMode::Simulation;
	}

	return E::SoftwareRunMode::Normal;
}

