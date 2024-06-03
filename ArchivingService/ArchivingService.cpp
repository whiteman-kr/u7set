#include "ArchivingService.h"
#include "TcpAppDataServer.h"
#include "TcpArchRequestsServer.h"


// -------------------------------------------------------------------------------
//
// AppDataService class implementation
//
// -------------------------------------------------------------------------------

ArchivingService::ArchivingService(const SoftwareInfo& softwareInfo,
											   const QString& serviceName,
											   int argc,
											   char** argv,
											   CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}

ArchivingService::ArchivingService(const ArchivingService* worker) :
	ServiceWorker(worker)
{
}

ArchivingService::~ArchivingService()
{
}

ServiceWorker* ArchivingService::createInstance() const
{
	ArchivingService* archServiceWorker = new ArchivingService(this);
	return archServiceWorker;
}

void ArchivingService::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::ArchiveService, m_serviceSettings);

	serviceInfo.set_settingsxml(xmlString.toStdString());
}

bool ArchivingService::isReadOnlyArchive() const
{
	return !m_readOnlyArchivePath.isEmpty();
}

void ArchivingService::initServiceSpecificCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	addValueCmdLineArg(CmdLineArg::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	addValueCmdLineArg(CmdLineArg::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
	addValueCmdLineArg("location",
						SoftwareSetting::ARCHIVE_LOCATION,
						"Path to archive location (overwrite ArchiveLocation from project settings)", "D:\\Archives");
	addValueCmdLineArg("mq",
							SoftwareSetting::MIN_QUEUE_SIZE_FOR_FLUSHING,
							QString("Minimum size of signal states queue for flushing to disk (default = %1 states).").
								arg(Archive::DEFAULT_QUEUE_SIZE_FOR_FLUSHING), "");
	addValueCmdLineArg(CmdLineArg::READ_ONLY, SoftwareSetting::READ_ONLY_ARCHIVE_PATH, "Path to read only archive.", "D:\\Archive\\ProjectID");
}

void ArchivingService::loadServiceSpecificSettings()
{
	m_overwriteArchiveLocation = getSettingValue(SoftwareSetting::ARCHIVE_LOCATION);

	QString sizeStr = getSettingValue(SoftwareSetting::MIN_QUEUE_SIZE_FOR_FLUSHING);

	bool ok = false;

	m_minQueueSizeForFlushing = sizeStr.toInt(&ok);

	if (sizeStr.isEmpty() == true || ok == false)
	{
		m_minQueueSizeForFlushing = Archive::DEFAULT_QUEUE_SIZE_FOR_FLUSHING;
	}

	m_readOnlyArchivePath = getSettingValue(SoftwareSetting::READ_ONLY_ARCHIVE_PATH);

	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).
						arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).
						arg(cfgServiceIP2().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::ARCHIVE_LOCATION).arg(m_overwriteArchiveLocation));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::MIN_QUEUE_SIZE_FOR_FLUSHING).arg(m_minQueueSizeForFlushing));
	DEBUG_LOG_MSG(logger(), "");
}

void ArchivingService::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();
	DEBUG_LOG_MSG(logger(), QString(tr("ArchivingServiceWorker initialized")));
}

void ArchivingService::shutdown()
{
	// Service Main Function deinitialization
	//
	stopAllThreads();
	stopCfgLoaderThread();
}

void ArchivingService::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ArchivingService::onConfigurationReady);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();
}

void ArchivingService::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread != nullptr)
	{
		m_cfgLoaderThread->quit();

		delete m_cfgLoaderThread;
	}
}

void ArchivingService::startAllThreads()
{
	startArchive();

	if (m_archive->isWorkable() == false)
	{
		return;
	}

	if (m_archive->isReadOnly() == false)
	{
		startTcpAppDataServerThread();
	}

	startTcpArchRequestsServerThread();
}

void ArchivingService::stopAllThreads()
{
	stopTcpAppDataServerThread();
	stopTcpArchiveRequestsServerThread();

	stopArchive();
}

void ArchivingService::startArchive()
{
	Q_ASSERT(m_archive == nullptr);

	if (isReadOnlyArchive() == true)
	{
		m_archive = new Archive(m_buildInfo.project,
								equipmentID(),
								m_readOnlyArchivePath,
								m_archInfoFileData,
								logger());
	}
	else
	{
		Q_ASSERT(m_archInfoFileData.isEmpty() == false);

		m_archive = new Archive(m_buildInfo.project,
								equipmentID(),
								m_serviceSettings.archiveLocation,
								m_archInfoFileData,
								m_serviceSettings.shortTermArchivePeriod,
								m_serviceSettings.longTermArchivePeriod,
								Archive::DEFAULT_MAINTENANCE_DELAY_MINUTES,
								m_minQueueSizeForFlushing,
								logger());
	}

	m_archive->start();

	if (m_archive->isWorkable() == true)
	{
		DEBUG_LOG_MSG(logger(), QString("Archive is workable. Directory: %1").arg(m_archive->archFullPath()));
	}
	else
	{
		DEBUG_LOG_ERR(logger(), QString("Archive is NOT WORKABLE!"));
	}
}

void ArchivingService::stopArchive()
{
	if (m_archive != nullptr)
	{
		m_archive->stop();
		delete m_archive;
		m_archive = nullptr;
	}
}

void ArchivingService::startTcpAppDataServerThread()
{
	assert(m_tcpAppDataServerThread == nullptr);
	assert(m_archive != nullptr);

	TcpAppDataServer* server = new TcpAppDataServer(softwareInfo(), m_archive);

	m_tcpAppDataServerThread = new Tcp::ListenerThread(m_serviceSettings.appDataReceivingIP,
													   m_serviceSettings.securityLevel,
													   server, logger());
	m_tcpAppDataServerThread->start();
}

void ArchivingService::stopTcpAppDataServerThread()
{
	if (m_tcpAppDataServerThread != nullptr)
	{
		m_tcpAppDataServerThread->quitAndWait();
		delete m_tcpAppDataServerThread;
		m_tcpAppDataServerThread = nullptr;
	}
}

void ArchivingService::startTcpArchRequestsServerThread()
{
	assert(m_tcpArchRequestsServerThread == nullptr);
	assert(m_archive != nullptr);

	if (m_archive == nullptr)
	{
		assert(false);
		return;
	}

	TcpArchRequestsServer* server = new TcpArchRequestsServer(softwareInfo(),
															  m_archive, logger());

	m_tcpArchRequestsServerThread = new Tcp::ListenerThread(m_serviceSettings.clientRequestIP,
															m_serviceSettings.securityLevel,
															server, logger());
	m_tcpArchRequestsServerThread->start();
}

void ArchivingService::stopTcpArchiveRequestsServerThread()
{
	if (m_tcpArchRequestsServerThread != nullptr)
	{
		m_tcpArchRequestsServerThread->quitAndWait();
		delete m_tcpArchRequestsServerThread;
		m_tcpArchRequestsServerThread = nullptr;
	}
}

void ArchivingService::logFileLoadResult(bool loadOk, const QString& fileName)
{
	if (loadOk == true)
	{
		DEBUG_LOG_MSG(logger(), QString("Load file %1 OK").arg(fileName));
	}
	else
	{
		DEBUG_LOG_ERR(logger(), QString("Load file %1 ERROR").arg(fileName));
	}
}

void ArchivingService::onConfigurationReady(const QByteArray configurationXmlData,
											const BuildFileInfoArray buildFileInfoArray,
											SessionParams sessionParams,
											std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	setSessionParams(sessionParams);

	TEST_PTR_RETURN(m_cfgLoaderThread);

	const ArchivingServiceSettings* typedSettingsPtr = dynamic_cast<const ArchivingServiceSettings*>(curSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		DEBUG_LOG_MSG(logger(), "Settings casting error!");
		return;
	}

	ArchivingServiceSettings newServiceSettings = *typedSettingsPtr;

	if (m_overwriteArchiveLocation.isEmpty() == false)
	{
		newServiceSettings.archiveLocation = m_overwriteArchiveLocation;
	}

	bool fileResult = true;

	m_archInfoFileData.clear();

	for(const OnlineLib::BuildFileInfo& bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		if (bfi.pathFileName.endsWith(File::ARCH_INFO_PROTO))
		{
			m_archInfoFileData.swap(fileData);
		}
	}

	if (fileResult == false)
	{
		DEBUG_LOG_ERR(logger(), "New configuration loading ERROR.");
		return;
	}

	DEBUG_LOG_MSG(logger(), "New configuration loading OK.");

	//

	DEBUG_LOG_MSG(logger(), "Applying new configuration.");

	stopAllThreads();

	m_buildInfo = m_cfgLoaderThread->buildInfo();
	m_serviceSettings = newServiceSettings;

	startAllThreads();
}

