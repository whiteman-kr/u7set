#include "../lib/WUtils.h"
#include "TuningService.h"
#include "version.h"


namespace Tuning
{
	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	const char* const TuningServiceWorker::SETTING_EQUIPMENT_ID = "EquipmentID";
	const char* const TuningServiceWorker::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
	const char* const TuningServiceWorker::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";

	TuningServiceWorker::TuningServiceWorker(const SoftwareInfo& softwareInfo,
											 const QString& serviceName,
											 int& argc,
											 char** argv,
											 std::shared_ptr<CircularLogger> logger) :
		ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
		m_logger(logger)
	{
	}

	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}

	ServiceWorker* TuningServiceWorker::createInstance() const
	{
		TuningServiceWorker* newInstance = new TuningServiceWorker(softwareInfo(),serviceName(), argc(), argv(), m_logger);

		newInstance->init();

		return newInstance;
	}

	void TuningServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
	{
		serviceInfo.set_clientrequestip(m_cfgSettings.clientRequestIP.address32());
		serviceInfo.set_clientrequestport(m_cfgSettings.clientRequestIP.port());
	}

	void TuningServiceWorker::initCmdLineParser()
	{
		CommandLineParser& cp = cmdLineParser();

		cp.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
		cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "");
		cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "");
	}

	void TuningServiceWorker::loadSettings()
	{
		m_equipmentID = getStrSetting(SETTING_EQUIPMENT_ID);

		//

		QString addressPortStr = getStrSetting(SETTING_CFG_SERVICE_IP1);

		if (addressPortStr.isEmpty() == false)
		{
			if (m_cfgServiceIP1.setAddressPort(addressPortStr, PORT_CONFIGURATION_SERVICE_REQUEST) == false)
			{
				DEBUG_LOG_WRN(m_logger, QString("Invalid value of %1 setting - %2").arg(SETTING_CFG_SERVICE_IP1).arg(addressPortStr));
			}
		}

		//

		addressPortStr = getStrSetting(SETTING_CFG_SERVICE_IP2);

		if (addressPortStr.isEmpty() == false)
		{
			if (m_cfgServiceIP2.setAddressPort(addressPortStr, PORT_CONFIGURATION_SERVICE_REQUEST) == false)
			{
				DEBUG_LOG_WRN(m_logger, QString("Invalid value of %1 setting - %2").arg(SETTING_CFG_SERVICE_IP2).arg(addressPortStr));
			}
		}

		//

		DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(m_equipmentID));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(m_cfgServiceIP1.addressPortStr()));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(m_cfgServiceIP2.addressPortStr()));
	}

	void TuningServiceWorker::clear()
	{
		m_tuningSources.clear();
	}

	const TuningClientContext* TuningServiceWorker::getClientContext(QString clientID) const
	{
		return m_clientContextMap.getClientContext(clientID);
	}

	const TuningClientContext* TuningServiceWorker::getClientContext(const std::string& clientID) const
	{
		return m_clientContextMap.getClientContext(QString::fromStdString(clientID));
	}

	void TuningServiceWorker::getAllClientContexts(QVector<const TuningClientContext*>& clientContexts)
	{
		clientContexts.clear();

		for(const TuningClientContext* clntContext : m_clientContextMap)
		{
			clientContexts.append(clntContext);
		}
	}

	bool TuningServiceWorker::singleLmControl() const
	{
		return m_cfgSettings.singleLmControl;
	}

	// called from TcpTuningServer thread!!!
	//
	NetworkError TuningServiceWorker::changeControlledTuningSource(const QString& tuningSourceEquipmentID,
												bool activateControl,
												QString* controlledTuningSource,
												bool* controlIsActive)
	{
		if (controlledTuningSource == nullptr || controlIsActive == nullptr)
		{
			return NetworkError::InternalError;
		}

		if (m_cfgSettings.singleLmControl == false)
		{
			controlledTuningSource->clear();
			*controlIsActive = false;
			return NetworkError::SingleLmControlDisabled;
		}

		AUTO_LOCK(m_mainMutex);							// !!!!

		stopSourcesListenerThread();

		stopTuningSourceWorkers();

		if (activateControl == false)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return NetworkError::Success;
		}

		bool result = runTuningSourceWorker(tuningSourceEquipmentID);

		if (result == false)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return NetworkError::InternalError;
		}

		runSourcesListenerThread();

		*controlledTuningSource = tuningSourceEquipmentID;
		*controlIsActive = true;

		return NetworkError::Success;
	}

	bool TuningServiceWorker::clientIsConnected(const SoftwareInfo& softwareInfo)
	{
		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			if (m_activeClientInfo.equipmentID().isEmpty() == true)
			{
				m_activeClientInfo = softwareInfo;
			}
		}
		else
		{
			m_activeClientInfo.clear();
		}

		return true;
	}

	bool TuningServiceWorker::clientIsDisconnected(const SoftwareInfo& softwareInfo)
	{
		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			if (m_activeClientInfo.equipmentID() == softwareInfo.equipmentID())
			{
				m_activeClientInfo.clear();
			}
		}
		else
		{
			m_activeClientInfo.clear();
		}

		return true;
	}

	bool TuningServiceWorker::setActiveClient(const SoftwareInfo& softwareInfo)
	{
		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			m_activeClientInfo = softwareInfo;
		}
		else
		{
			m_activeClientInfo.clear();
		}

		return true;
	}

	QString TuningServiceWorker::activeClientID()
	{
		QString clientID;

		m_mainMutex.lock();

		clientID = m_activeClientInfo.equipmentID();

		m_mainMutex.unlock();

		return clientID;
	}

	void TuningServiceWorker::initialize()
	{
		runCfgLoaderThread();
	}

	void TuningServiceWorker::shutdown()
	{
		clearConfiguration();
		stopCfgLoaderThread();
	}

	void TuningServiceWorker::runCfgLoaderThread()
	{
		CfgLoader* cfgLoader = new CfgLoader(softwareInfo(), 1, m_cfgServiceIP1, m_cfgServiceIP2, false, m_logger);

		m_cfgLoaderThread = new CfgLoaderThread(cfgLoader);

		connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &TuningServiceWorker::onConfigurationReady);

		m_cfgLoaderThread->start();
		m_cfgLoaderThread->enableDownloadConfiguration();
	}

	void TuningServiceWorker::stopCfgLoaderThread()
	{
		if (m_cfgLoaderThread == nullptr)
		{
			return;
		}

		m_cfgLoaderThread->quit();

		delete m_cfgLoaderThread;
	}

	void TuningServiceWorker::clearConfiguration()
	{
		DEBUG_LOG_MSG(m_logger, QString("Clear current configuration"));

		stopTcpTuningServerThread();

		m_mainMutex.lock();

		stopSourcesListenerThread();
		stopTuningSourceWorkers();
		clearServiceMaps();

		m_mainMutex.unlock();
	}

	void TuningServiceWorker::applyNewConfiguration()
	{
		DEBUG_LOG_MSG(m_logger, QString("Apply new configuration"));

		m_mainMutex.lock();

		buildServiceMaps();
		runTuningSourceWorkers();
		runSourcesListenerThread();

		m_mainMutex.unlock();

		runTcpTuningServerThread();
	}

	void TuningServiceWorker::buildServiceMaps()
	{
		m_clientContextMap.init(m_cfgSettings, m_tuningSources);
	}


	void TuningServiceWorker::clearServiceMaps()
	{
		m_clientContextMap.clear();
	}


	void TuningServiceWorker::runTcpTuningServerThread()
	{
		TcpTuningServer* tcpTuningSever = new TcpTuningServer(*this, m_logger);

		m_tcpTuningServerThread = new TcpTuningServerThread(m_cfgSettings.clientRequestIP,
															tcpTuningSever,
															m_logger);
		m_tcpTuningServerThread->start();
	}

	void TuningServiceWorker::stopTcpTuningServerThread()
	{
		if (m_tcpTuningServerThread != nullptr)
		{
			m_tcpTuningServerThread->quitAndWait();

			delete m_tcpTuningServerThread;

			m_tcpTuningServerThread = nullptr;
		}
	}

	bool TuningServiceWorker::readConfiguration(const QByteArray& cfgXmlData)
	{
		QString str;

		XmlReadHelper xml(cfgXmlData);

		bool result = true;

		result &= m_cfgSettings.readFromXml(xml);
		result &= readTuningDataSources(xml);

		if  (result == true)
		{
			str = QString("Configuration is loaded successful");
		}
		else
		{
			str = QString("Loading configuration error");
		}

		qDebug() << C_STR(str);

		return result;
	}

	bool TuningServiceWorker::loadConfigurationFromFile(const QString& fileName)
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

		cfgXmlData = file.readAll();

		bool result = true;

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

	bool TuningServiceWorker::readTuningDataSources(XmlReadHelper& xml)
	{
		bool result = true;

		m_tuningSources.clear();

		result = xml.findElement("TuningSources");

		if (result == false)
		{
			return false;
		}

		int sourceCount = 0;

		result &= xml.readIntAttribute("Count", &sourceCount);

		for(int i = 0; i < sourceCount; i++)
		{
			result = xml.findElement(DataSource::ELEMENT_DATA_SOURCE);

			if (result == false)
			{
				return false;
			}

			TuningSource* ds = new TuningSource();

			result &= ds->readFromXml(xml);

			if (result == false)
			{
				delete ds;
				break;
			}

			m_tuningSources.insert(ds->lmEquipmentID(), ds);
		}

		m_tuningSources.buildIP2DataSourceMap();

		return result;
	}

	void TuningServiceWorker::runTuningSourceWorkers()
	{
		if (m_cfgSettings.singleLmControl == false)
		{
			// running all TuningSourceWorkers at once if SingleLmControl is disabled
			//
			runTuningSourceWorker("");
		}
	}

	bool TuningServiceWorker::runTuningSourceWorker(const QString& tuningSourceEquipmentID)
	{
		// if tuningSourceEquipmentID empty - run all sources workers
		// else - run specific source worker
		//
		assert(m_sourceWorkerThreadMap.size() == 0);

		bool result = false;

		for(TuningSource* tuningSource : m_tuningSources)
		{
			if (tuningSource == nullptr)
			{
				assert(false);
				continue;
			}

			if (tuningSourceEquipmentID.isEmpty() == false && tuningSource->lmEquipmentID() != tuningSourceEquipmentID)
			{
				continue;
			}

			// create TuningSourceWorkerThreads and fill m_sourceWorkerThreadMap
			//
			TuningSourceWorkerThread* sourceWorkerThread = new TuningSourceWorkerThread(m_cfgSettings, *tuningSource, m_logger);

			if (sourceWorkerThread == nullptr)
			{
				assert(false);
				continue;
			}

			quint32 addr = sourceWorkerThread->sourceIP();

			m_sourceWorkerThreadMap.insert(addr, sourceWorkerThread);

			setWorkerInTuningClientContext(sourceWorkerThread->worker());

			result = true;
		}

		// run TuningSourceWorkerThreads
		//
		for(TuningSourceWorkerThread* sourceWorkerThread : m_sourceWorkerThreadMap)
		{
			sourceWorkerThread->start();
		}

		return result;
	}

	void TuningServiceWorker::stopTuningSourceWorkers()
	{
		// stop and delete TuningSourceWorkerThreads
		//
		for(TuningSourceWorkerThread* sourceWorkerThread : m_sourceWorkerThreadMap)
		{
			if (sourceWorkerThread == nullptr)
			{
				assert(false);
				continue;
			}

			removeWorkerFromTuningClientContext(sourceWorkerThread->worker());

			sourceWorkerThread->quitAndWait();
			delete sourceWorkerThread;
		}

		m_sourceWorkerThreadMap.clear();
	}

	void TuningServiceWorker::runSourcesListenerThread()
	{
		if (m_sourceWorkerThreadMap.size() == 0)
		{
			DEBUG_LOG_MSG(m_logger, QString("Tuning sources workers is not running. Listener thread is not run also."));
			return;
		}

		// create and run TuningSocketListenerThread
		//
		assert(m_socketListenerThread == nullptr);

		m_socketListenerThread = new TuningSocketListenerThread(m_cfgSettings.tuningDataIP, m_sourceWorkerThreadMap, m_logger);
		m_socketListenerThread->start();
	}

	void TuningServiceWorker::stopSourcesListenerThread()
	{
		// stop and delete TuningSocketListenerThread
		//
		if (m_socketListenerThread != nullptr)
		{
			m_socketListenerThread->quitAndWait();
			delete m_socketListenerThread;
			m_socketListenerThread = nullptr;
		}
	}

	void TuningServiceWorker::setWorkerInTuningClientContext(TuningSourceWorker* worker)
	{
		TEST_PTR_RETURN(worker);

		for(TuningClientContext* clientContext : m_clientContextMap)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			clientContext->setSourceWorker(worker);
		}
	}

	void TuningServiceWorker::removeWorkerFromTuningClientContext(TuningSourceWorker* worker)
	{
		TEST_PTR_RETURN(worker);

		for(TuningClientContext* clientContext : m_clientContextMap)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			clientContext->removeSourceWorker(worker);
		}
	}

	void TuningServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
	{
		if (m_cfgLoaderThread == nullptr)
		{
			return;
		}

		DEBUG_LOG_MSG(m_logger, QString("Configuration is ready"));

		bool result = true;

		result = readConfiguration(configurationXmlData);

		if (result == false)
		{
			DEBUG_LOG_ERR(m_logger, QString("Configuration reading error"));
			return;
		}

		DEBUG_LOG_MSG(m_logger, QString("Configuration reading success"));

		clearConfiguration();

		applyNewConfiguration();
	}
}
