#include <QXmlStreamReader>
#include <QMetaProperty>
#include "ArchivingService.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

ArchivingServiceWorker::ArchivingServiceWorker() :
	ServiceWorker(ServiceType::AppDataService)
{
}


ArchivingServiceWorker::~ArchivingServiceWorker()
{
}


void ArchivingServiceWorker::initCmdLineParser()
{
	CommandLineParser* clp = cmdLineParser();

	if (clp == nullptr)
	{
		assert(false);
		return;
	}

	clp->addSingleValueOption("cfgip1", "IP-addres of first Configuration Service.");
	clp->addSingleValueOption("cfgip2", "IP-addres of second Configuration Service.");
}


void ArchivingServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	if (m_buildPath.isEmpty() == true)
	{
		runCfgLoaderThread();
	}
	else
	{
		/*bool result = loadConfigurationFromFile(cfgFileName());

		if (result == true)
		{
			applyNewConfiguration();
		}*/
	}

	qDebug() << "ArchivingServiceWorker initialized";
}


void ArchivingServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	clearConfiguration();

	stopCfgLoaderThread();

	qDebug() << "ArchivingServiceWorker stoped";
}


void ArchivingServiceWorker::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(serviceEquipmentID(), 1,
											HostAddressPort(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_REQUEST),
											HostAddressPort(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_REQUEST));

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

	bool result = m_settings.readFromXml(xml);

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


void ArchivingServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo)
{
	serviceInfo.set_clientrequestip(m_settings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_settings.clientRequestIP.port());
}


