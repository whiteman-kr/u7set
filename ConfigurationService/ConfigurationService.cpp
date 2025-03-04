#include "ConfigurationService.h"
#include <CommonLib/ConstStrings.h>
#include "CfgChecker.h"
#include "CfgControlServer.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class implementation
//
// ------------------------------------------------------------------------------------

ConfigurationServiceWorker::ConfigurationServiceWorker(const SoftwareInfo& softwareInfo,
													   const QString& serviceName,
													   int argc, char** argv,
													   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}

ConfigurationServiceWorker::ConfigurationServiceWorker(const ConfigurationServiceWorker* worker) :
	ServiceWorker(worker)
{
}

ServiceWorker* ConfigurationServiceWorker::createInstance() const
{
	ConfigurationServiceWorker* newInstance = new ConfigurationServiceWorker(this);
	return newInstance;
}

void ConfigurationServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::ConfigurationService, m_cfgServiceSettings);

	serviceInfo.set_settingsxml(xmlString.toStdString());
}

void ConfigurationServiceWorker::onBuildPathChanged(QString newBuildPath)
{
	stopCfgServerThread();
	clearBuildInfo();

	emit renameWorkBuildToBackupExcept(newBuildPath);

	bool result = loadCfgServiceSettings(newBuildPath);

	if (result == false)
	{
		return;
	}

	startCfgServerThread(newBuildPath);
}

void ConfigurationServiceWorker::initServiceSpecificCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	addValueCmdLineArg(CmdLineArg::BUILD_PATH, SoftwareSetting::AUTOLOAD_BUILD_PATH, "Path to RPCT project's build for auto load.", "PathToBuild");
	addValueCmdLineArg(CmdLineArg::WORK_DIRECTORY, SoftwareSetting::WORK_DIRECTORY, "Work directory of Configuration Service.", "Path");
	addBoolCmdLineArg(CmdLineArg::CHECKHOSTNAME, SoftwareSetting::CHECK_HOSTNAME, "Check clients hostname.");
	addValueCmdLineArg(CmdLineArg::PROFILE, SoftwareSetting::CURRENT_PROFILE, "Current software settings profile.", "ProfileID");
	addValueCmdLineArg(CmdLineArg::MODE, SoftwareSetting::RUN_MODE, "Runs all software in simulation mode.", SoftwareSetting::SIMULATION);
}

void ConfigurationServiceWorker::loadServiceSpecificSettings()
{
	m_autoloadBuildPath = getSettingValue(SoftwareSetting::AUTOLOAD_BUILD_PATH);
	m_workDirectory = getSettingValue(SoftwareSetting::WORK_DIRECTORY);
	m_checkHostname = getBoolSettingValue(SoftwareSetting::CHECK_HOSTNAME);

	SessionParams sp;

	sp.currentSettingsProfile = getSettingValue(SoftwareSetting::CURRENT_PROFILE);

	if (sp.currentSettingsProfile.isEmpty() == true)
	{
		sp.currentSettingsProfile = SettingsProfile::DEFAULT;
	}

	sp.softwareRunMode = getSoftwareRunMode(getSettingValue(SoftwareSetting::RUN_MODE));

	setSessionParams(sp);

	DEBUG_LOG_MSG(logger(), QString());
	DEBUG_LOG_MSG(logger(), QString("Service settings:"));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::EQUIPMENT_ID, equipmentID()));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::AUTOLOAD_BUILD_PATH, m_autoloadBuildPath));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::WORK_DIRECTORY, m_workDirectory));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::CHECK_HOSTNAME, boolToString(m_checkHostname)));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::CURRENT_PROFILE, sessionParams().currentSettingsProfile));
	DEBUG_LOG_MSG(logger(), QString("%1 = %2").arg(SoftwareSetting::RUN_MODE, E::valueToString<E::SoftwareRunMode>(sessionParams().softwareRunMode)));
	DEBUG_LOG_MSG(logger(), QString());
}

bool ConfigurationServiceWorker::loadCfgServiceSettings(const QString& buildPath)
{
	QString cfgXmlPath = QString("%1/%2/%3").arg(buildPath).arg(equipmentID()).arg(File::CONFIGURATION_XML);

	QFile cfgXmlFile(cfgXmlPath);

	if (cfgXmlFile.open(QIODevice::ReadOnly) == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error opening: %1").arg(cfgXmlPath));
		return false;
	}

	QByteArray cfgXmlData = cfgXmlFile.readAll();

	cfgXmlFile.close();

	bool res = readBuildInfo(cfgXmlData);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading BuildInfo from file: %1").arg(File::CONFIGURATION_XML));
		return false;
	}

	res = softwareSettingsSet().readFromXml(cfgXmlData);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger(), QString("Error reading file: %1").arg(File::CONFIGURATION_XML));
		return false;
	}

	QString curProfile = sessionParams().currentSettingsProfile;

	auto ptr = softwareSettingsSet().getSettingsProfile<CfgServiceSettings>(curProfile);

	if (ptr == nullptr)
	{
		if (softwareSettingsSet().settingsProfileIsExists(curProfile) == false)
		{
			DEBUG_LOG_ERR(logger(), QString("------------ Settings profile '%1' is not exists ------------").arg(curProfile));
		}
		else
		{
			DEBUG_LOG_ERR(logger(), QString("------------ Error loading settings for profile: %1 ------------").arg(curProfile));
		}

		if (serviceRunMode() == E::ServiceRunMode::ConsoleApp)
		{
			QCoreApplication::exit(0);
		}

		return false;
	}

	m_cfgServiceSettings = *ptr.get();

	DEBUG_LOG_MSG(logger(), QString());
	DEBUG_LOG_MSG(logger(), QString("Loading settings for profile: %1 - Ok").arg(curProfile));
	DEBUG_LOG_MSG(logger(), QString());

	if (cmdLineArgIsSet(CmdLineArg::CHECKHOSTNAME) == true)
	{
		m_cfgServiceSettings.checkHostname = m_checkHostname;

		DEBUG_LOG_MSG(logger(), QString("CheckHostname is set to %1").
					  arg(m_cfgServiceSettings.checkHostname == true ? "On" : "Off"));
	}

	return res;
}

void ConfigurationServiceWorker::initialize()
{
	startCfgCheckerThread();

	DEBUG_LOG_MSG(logger(), QString(tr("ConfigurationServiceWorker is initialized")));
}

void ConfigurationServiceWorker::shutdown()
{
	stopCfgCheckerThread();
	stopCfgServerThread();

	DEBUG_LOG_MSG(logger(), QString(tr("ConfigurationServiceWorker is shutting down")));
}

void ConfigurationServiceWorker::startCfgServerThread(const QString& buildPath)
{

	CfgControlServer* cfgControlServer = new CfgControlServer(softwareInfo(),
															  m_autoloadBuildPath,
															  m_workDirectory,
															  buildPath,
															  sessionParams(),
															  m_cfgServiceSettings.clients,
															  m_cfgServiceSettings.checkHostname,
															  *m_cfgCheckerWorker,
															  logger());
	std::vector<Tcp::ListenAddress> listenAddrs;

	for(const RqCtrlSettings& rcs: m_cfgServiceSettings.rcSettings)
	{
		if (rcs.enable())
		{
			listenAddrs.emplace_back(rcs.equipmentID(), rcs.clientRequestIP(), rcs.securityLevel());
		}
	}

	m_cfgServerThread = new Tcp::ListenerThread(listenAddrs, cfgControlServer, logger());

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
	m_cfgCheckerWorker = new CfgCheckerWorker(equipmentID(), m_workDirectory, m_autoloadBuildPath, 3 * 1000, logger());

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
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_CONFIGURATION_SERVICE_INFO, logger());

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
