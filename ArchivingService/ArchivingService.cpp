#include <QXmlStreamReader>
#include <QMetaProperty>
#include "ArchivingService.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

ArchivingServiceWorker::ArchivingServiceWorker(const SoftwareInfo& softwareInfo,
											   const QString& serviceName,
											   int& argc,
											   char** argv,
											   CircularLoggerShared logger) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger)
{
}


ArchivingServiceWorker::~ArchivingServiceWorker()
{
}

ServiceWorker* ArchivingServiceWorker::createInstance() const
{
	ArchivingServiceWorker* archServiceWorker = new ArchivingServiceWorker(softwareInfo(), serviceName(), argc(), argv(), logger());

	archServiceWorker->init();

	return archServiceWorker;
}

void ArchivingServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	serviceInfo.set_clientrequestip(m_cfgSettings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_cfgSettings.clientRequestIP.port());
}

void ArchivingServiceWorker::initCmdLineParser()
{
	CommandLineParser& cp = cmdLineParser();

	cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
	cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
}

void ArchivingServiceWorker::loadSettings()
{
	DEBUG_LOG_MSG(logger(), QString(tr("Load settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStr()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStr()));
}

void ArchivingServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), QString(tr("ArchivingServiceWorker initialized")));
}

void ArchivingServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	stopAllThread();

	stopCfgLoaderThread();

	DEBUG_LOG_MSG(logger(), QString(tr("ArchivingServiceWorker stoped")));
}

void ArchivingServiceWorker::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, logger());

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ArchivingServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();
}

void ArchivingServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread != nullptr)
	{
		m_cfgLoaderThread->quit();

		delete m_cfgLoaderThread;
	}
}

void ArchivingServiceWorker::runAllThreads()
{
	runArchWriteThread();
	runArchRequestThread();
	runTcpAppDataServerThread();
	runTcpArchRequestsServerThread();
}

void ArchivingServiceWorker::stopAllThread()
{
	stopTcpArchiveRequestsServerThread();
	stopTcpAppDataServerThread();
	stopArchRequestThread();
	stopArchWriteThread();
}

void ArchivingServiceWorker::createArchive()
{
	assert(m_archive == nullptr);

	m_archive = std::make_shared<Archive>(m_buildInfo.project, equipmentID(), "d:/Temp", m_cfgSettings.dbHost, logger());
}

void ArchivingServiceWorker::deleteArchive()
{
	m_archive.reset();
}

void ArchivingServiceWorker::runArchWriteThread()
{
	assert(m_archWriteThread == nullptr);

	m_archWriteThread = new ArchWriteThread(m_cfgSettings.dbHost,
											m_archive,
											logger());

	m_archWriteThread->start();

	//

	assert(m_fileArchWriter == nullptr);

	m_fileArchWriter = new FileArchWriter(m_archive, logger());

	m_fileArchWriter->start();
}

void ArchivingServiceWorker::stopArchWriteThread()
{
	if (m_archWriteThread != nullptr)
	{
		m_archWriteThread->quitAndWait();
		delete m_archWriteThread;
		m_archWriteThread = nullptr;
	}

	//

	if (m_fileArchWriter != nullptr)
	{
		m_fileArchWriter->quitAndWait();
		delete m_fileArchWriter;
		m_fileArchWriter = nullptr;
	}
}

void ArchivingServiceWorker::runTcpAppDataServerThread()
{
	assert(m_tcpAppDataServerThread == nullptr);

	TcpAppDataServer* server = new TcpAppDataServer(softwareInfo(), m_archive);

	m_tcpAppDataServerThread = new TcpAppDataServerThread(m_cfgSettings.appDataServiceRequestIP, server, logger());

	m_tcpAppDataServerThread->start();
}

void ArchivingServiceWorker::stopTcpAppDataServerThread()
{
	if (m_tcpAppDataServerThread != nullptr)
	{
		m_tcpAppDataServerThread->quitAndWait();
		delete m_tcpAppDataServerThread;
		m_tcpAppDataServerThread = nullptr;
	}
}

void ArchivingServiceWorker::runTcpArchRequestsServerThread()
{
	assert(m_tcpArchiveRequestsServerThread == nullptr);

	if (m_archRequestThread == nullptr)
	{
		assert(false);
		return;
	}

	TcpArchRequestsServer* server = new TcpArchRequestsServer(softwareInfo(), *m_archRequestThread, logger());

	m_tcpArchiveRequestsServerThread = new TcpArchiveRequestsServerThread(m_cfgSettings.clientRequestIP,
																		  server,
																		  logger());
	m_tcpArchiveRequestsServerThread->start();
}

void ArchivingServiceWorker::stopTcpArchiveRequestsServerThread()
{
	if (m_tcpArchiveRequestsServerThread != nullptr)
	{
		m_tcpArchiveRequestsServerThread->quitAndWait();
		delete m_tcpArchiveRequestsServerThread;
		m_tcpArchiveRequestsServerThread = nullptr;
	}
}

void ArchivingServiceWorker::runArchRequestThread()
{
	assert(m_archRequestThread == nullptr);
	assert(m_fileArchWriter != nullptr);

	m_archRequestThread = new ArchRequestThread(m_archive, m_fileArchWriter, logger());

	m_archRequestThread->start();
}

void ArchivingServiceWorker::stopArchRequestThread()
{
	if (m_archRequestThread != nullptr)
	{
		m_archRequestThread->quitAndWait();
		delete m_archRequestThread;
		m_archRequestThread = nullptr;
	}
}

bool ArchivingServiceWorker::readConfiguration(const QByteArray& fileData)
{
	XmlReadHelper xml(fileData);

	bool result = m_cfgSettings.readFromXml(xml);

	if (result == true)
	{
		qDebug() << "Reading settings - OK";
	}
	else
	{
		qDebug() << "Settings read ERROR!";
	}

	return result;
}

bool ArchivingServiceWorker::loadConfigurationFromFile(const QString& fileName)
{
	QString str;

	QByteArray cfgXmlData;

	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		str = QString("Error open configuration file: %1").arg(fileName);

		qDebug() << C_STR(str);

		return false;
	}

	bool result = true;

	cfgXmlData = file.readAll();

	result = readConfiguration(cfgXmlData);

	if  (result == true)
	{
		str = QString("Configuration is loaded from file: %1").arg(fileName);
	}
	else
	{
		str = QString("Loading configuration error from file: %1").arg(fileName);
	}

	qDebug() << C_STR(str);

	return result;
}

bool ArchivingServiceWorker::initArchSignals(const QByteArray& fileData)
{
	Proto::ArchSignals archSignals;

	bool result = archSignals.ParseFromArray(reinterpret_cast<const void*>(fileData.constData()), fileData.size());

	if (result == false)
	{
		assert(false);
		return false;
	}

	m_archive->initArchSignals(archSignals);

	return true;
}

void ArchivingServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	stopAllThread();

	deleteArchive();

	bool result = readConfiguration(configurationXmlData);

	if (result == false)
	{
		return;
	}

	m_buildInfo = m_cfgLoaderThread->buildInfo();

	createArchive();

	for(Builder::BuildFileInfo bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		if (bfi.pathFileName.endsWith("ArchSignals.proto"))
		{
			initArchSignals(fileData);
		}

		if (result == true)
		{
			qDebug() << "Read file " << bfi.pathFileName << " OK";
		}
		else
		{
			qDebug() << "Read file " << bfi.pathFileName << " ERROR";
			break;
		}
	}

	if (result == true)
	{
		runAllThreads();
	}
}

