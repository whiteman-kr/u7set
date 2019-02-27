#include <QXmlStreamReader>
#include <QMetaProperty>
#include "ArchivingService.h"


// -------------------------------------------------------------------------------
//
// AppDataService class implementation
//
// -------------------------------------------------------------------------------

const char* const ArchivingService::SETTING_ARCHIVE_LOCATION = "ArchiveLocation";

ArchivingService::ArchivingService(const SoftwareInfo& softwareInfo,
											   const QString& serviceName,
											   int& argc,
											   char** argv,
											   CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}


ArchivingService::~ArchivingService()
{
	deleteArchSignalsProto();
}

ServiceWorker* ArchivingService::createInstance() const
{
	ArchivingService* archServiceWorker = new ArchivingService(softwareInfo(), serviceName(), argc(), argv(), logger());

	archServiceWorker->init();

	return archServiceWorker;
}

void ArchivingService::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	serviceInfo.set_clientrequestip(m_serviceSettings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_serviceSettings.clientRequestIP.port());
}

void ArchivingService::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
	cp.addSingleValueOption("location", SETTING_ARCHIVE_LOCATION, "Archive location (overwrite ArchiveLocation from project settings)", "");
}

void ArchivingService::loadSettings()
{
	m_overwriteArchiveLocation = QString(getStrSetting(SETTING_ARCHIVE_LOCATION));

	DEBUG_LOG_MSG(logger(), QString(tr("Load settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_ARCHIVE_LOCATION).arg(m_overwriteArchiveLocation));
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

	DEBUG_LOG_MSG(logger(), QString(tr("ArchivingServiceWorker stoped")));
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

	if (m_archive->isWorkable() == true)
	{
		startTcpAppDataServerThread();
		startTcpArchRequestsServerThread();
	}
}

void ArchivingService::stopAllThreads()
{
	stopTcpAppDataServerThread();
	stopTcpArchiveRequestsServerThread();

	stopArchive();
}

void ArchivingService::startArchive()
{
	TEST_PTR_RETURN(m_archSignalsProto);

	if (m_archive == nullptr)
	{
		m_archive = new Archive(m_buildInfo.project,
								equipmentID(),
								m_serviceSettings.archiveLocation,
								*m_archSignalsProto,
								m_serviceSettings.shortTermArchivePeriod,
								m_serviceSettings.longTermArchivePeriod,
								5,		// archive maintenance running delay (minutes)
								logger());

		deleteArchSignalsProto();				// no more required

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
	else
	{
		assert(false);
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

	m_tcpAppDataServerThread = new Tcp::ServerThread(m_serviceSettings.appDataRecevingIP, server, logger());
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

	TcpArchRequestsServer* server = new TcpArchRequestsServer(softwareInfo(), m_archive, logger());

	m_tcpArchRequestsServerThread = new Tcp::ServerThread(m_serviceSettings.clientRequestIP, server, logger());
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

bool ArchivingService::loadConfigurationXml(const QByteArray& fileData, ArchivingServiceSettings* settings)
{
	TEST_PTR_RETURN_FALSE(settings);

	XmlReadHelper xml(fileData);

	bool result = settings->readFromXml(xml);

	return result;
}

bool ArchivingService::loadArchSignalsProto(const QByteArray& fileData)
{
	deleteArchSignalsProto();

	m_archSignalsProto = new Proto::ArchSignals;

	bool result = m_archSignalsProto->ParseFromArray(reinterpret_cast<const void*>(fileData.constData()), fileData.size());

	return result;
}

void ArchivingService::deleteArchSignalsProto()
{
	if (m_archSignalsProto != nullptr)
	{
		delete m_archSignalsProto;
		m_archSignalsProto = nullptr;
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

void ArchivingService::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	TEST_PTR_RETURN(m_cfgLoaderThread);

	DEBUG_LOG_MSG(logger(), "Trying new configuration loading.");

	ArchivingServiceSettings newServiceSettings;

	bool fileResult = loadConfigurationXml(configurationXmlData, &newServiceSettings);

	if (m_overwriteArchiveLocation.isEmpty() == false)
	{
		newServiceSettings.archiveLocation = m_overwriteArchiveLocation;
	}

	logFileLoadResult(fileResult, "Configuration.xml");

	if (fileResult == false)
	{
		return;
	}

	for(const Builder::BuildFileInfo& bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		bool fileResult = true;

		if (bfi.pathFileName.endsWith("ArchSignals.proto"))
		{
			fileResult = loadArchSignalsProto(fileData);

			logFileLoadResult(fileResult, bfi.pathFileName);
		}

		if (fileResult == false)
		{
			break;
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

