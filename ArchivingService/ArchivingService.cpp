#include <QXmlStreamReader>
#include <QMetaProperty>
#include "ArchivingService.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

ArchivingServiceWorker::ArchivingServiceWorker(const QString& serviceStrID,
									 const QString& cfgServiceIP1,
									 const QString& cfgServiceIP2,
									 const QString& buildPath) :
	ServiceWorker(ServiceType::AppDataService, serviceStrID, cfgServiceIP1, cfgServiceIP2, buildPath)
{
}


ArchivingServiceWorker::~ArchivingServiceWorker()
{
}


void ArchivingServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo)
{
	serviceInfo.set_clientrequestip(m_settings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_settings.clientRequestIP.port());
}


void ArchivingServiceWorker::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(serviceEquipmentID(), 1,
											HostAddressPort(cfgServiceIP1(), PORT_CONFIGURATION_SERVICE_REQUEST),
											HostAddressPort(cfgServiceIP2(), PORT_CONFIGURATION_SERVICE_REQUEST));

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ArchivingServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();
}


void ArchivingServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(false);
		return;
	}

	m_cfgLoaderThread->quit();

	delete m_cfgLoaderThread;
}


void ArchivingServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();

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



void ArchivingServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
}


void ArchivingServiceWorker::applyNewConfiguration()
{
}

