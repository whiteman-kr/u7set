#include <QXmlStreamReader>
#include <QMetaProperty>
#include "ArchivingService.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

ArchivingServiceWorker::ArchivingServiceWorker(const QString& serviceName,
											   int& argc,
											   char** argv,
											   const VersionInfo& versionInfo,
											   std::shared_ptr<CircularLogger> logger) :
	ServiceWorker(ServiceType::AppDataService, serviceName, argc, argv, versionInfo, logger),
	m_logger(logger)
{
}


ArchivingServiceWorker::~ArchivingServiceWorker()
{
}


ServiceWorker* ArchivingServiceWorker::createInstance() const
{
	ArchivingServiceWorker* archServiceWorker = new ArchivingServiceWorker(serviceName(), argc(), argv(), versionInfo(), m_logger);

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

	cp.addSingleValueOption("id", "Service EquipmentID.", "EQUIPMENT_ID");
	cp.addSingleValueOption("cfgip1", "IP-addres of first Configuration Service.");
	cp.addSingleValueOption("cfgip2", "IP-addres of second Configuration Service.");
}


void ArchivingServiceWorker::processCmdLineSettings()
{
	CommandLineParser& cp = cmdLineParser();

	if (cp.optionIsSet("id") == true)
	{
		setStrSetting("EquipmentID", cp.optionValue("id"));
	}

	if (cp.optionIsSet("cfgip1") == true)
	{
		setStrSetting("CfgServiceIP1", cp.optionValue("cfgip1"));
	}

	if (cp.optionIsSet("cfgip2") == true)
	{
		setStrSetting("CfgServiceIP2", cp.optionValue("cfgip2"));
	}
}


void ArchivingServiceWorker::loadSettings()
{
	m_equipmentID = getStrSetting("EquipmentID");

	m_cfgServiceIP1Str = getStrSetting("CfgServiceIP1");

	m_cfgServiceIP1 = HostAddressPort(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	m_cfgServiceIP2Str = getStrSetting("CfgServiceIP2");

	m_cfgServiceIP2 = HostAddressPort(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg("EquipmentID").arg(m_equipmentID));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP1").arg(m_cfgServiceIP1Str).arg(m_cfgServiceIP1.addressPortStr()));
	DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP2").arg(m_cfgServiceIP2Str).arg(m_cfgServiceIP2.addressPortStr()));
}


void ArchivingServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ArchivingServiceWorker initialized")));
}


void ArchivingServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	clearConfiguration();

	stopCfgLoaderThread();

	DEBUG_LOG_MSG(m_logger, QString(tr("ArchivingServiceWorker stoped")));
}


void ArchivingServiceWorker::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(m_equipmentID, 1,m_cfgServiceIP1, m_cfgServiceIP2, false, nullptr);

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


void ArchivingServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
}


void ArchivingServiceWorker::applyNewConfiguration()
{
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


void ArchivingServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	clearConfiguration();

	bool result = readConfiguration(configurationXmlData);

	if (result == false)
	{
		return;
	}

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

/*		result = true;

		if (bfi.ID == CFG_FILE_ID_DATA_SOURCES)
		{
			result &= readDataSources(fileData);			// fill m_appDataSources
		}

		if (bfi.ID == CFG_FILE_ID_APP_SIGNALS)
		{
			result &= readAppSignals(fileData);				// fill m_unitInfo and m_appSignals
		}*/

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
		applyNewConfiguration();
	}
}



