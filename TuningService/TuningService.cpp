#include "../lib/WUtils.h"
#include "TuningService.h"


namespace Tuning
{
	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	const QString TuningServiceWorker::SETTING_TUNING_SIM_IP("TuningSimIP");

	TuningServiceWorker::TuningServiceWorker(const SoftwareInfo& softwareInfo,
											 const QString& serviceName,
											 int& argc,
											 char** argv,
											 CircularLoggerShared logger,
											 CircularLoggerShared tuningLog) :
		ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
		m_logger(logger),
		m_tuningLog(tuningLog)
	{
	}

	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}

	ServiceWorker* TuningServiceWorker::createInstance() const
	{
		TuningServiceWorker* newInstance = new TuningServiceWorker(softwareInfo(),
																   serviceName(),
																   argc(), argv(), m_logger, m_tuningLog);

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

		cp.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1,
								QString("IP-address of first Configuration Service (default port - %1).").
											arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST), "ip[:port]");
		cp.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2,
								QString("IP-address of second Configuration Service (default port - %1).").
											arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST), "ip[:port]");
		cp.addSingleValueOption("sim", SETTING_TUNING_SIM_IP,
								QString("IP-address of Simulator (default port - %1)").arg(PORT_LM_TUNING), "ip[:port]");
	}

	void TuningServiceWorker::loadSettings()
	{
		m_tuningSimIPStr = getStrSetting(SETTING_TUNING_SIM_IP);
		m_tuningSimIP.setAddressPortStr(m_tuningSimIPStr, PORT_LM_TUNING);

		DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_EQUIPMENT_ID).arg(equipmentID()));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg(SETTING_TUNING_SIM_IP).arg(m_tuningSimIP.addressPortStrIfSet()));
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

	const TuningSourceThread* TuningServiceWorker::getSourceThread(quint32 sourceIP) const
	{
		return m_sourceThreadMap.value(sourceIP, nullptr);
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

		stopTuningSourceThreads();

		if (activateControl == false)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return NetworkError::Success;
		}

		bool result = runTuningSourceThread(tuningSourceEquipmentID);

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

	bool TuningServiceWorker::clientIsConnected(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			if (m_activeClientInfo.equipmentID().isEmpty() == true)
			{
				m_activeClientInfo = softwareInfo;
				m_activeClientIP = clientIP;
			}
		}
		else
		{
			m_activeClientInfo.clear();
			m_activeClientIP.clear();
		}

		return true;
	}

	bool TuningServiceWorker::clientIsDisconnected(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			if (m_activeClientInfo.equipmentID() == softwareInfo.equipmentID() &&
				m_activeClientIP == clientIP)
			{
				m_activeClientInfo.clear();
				m_activeClientIP.clear();
			}
		}
		else
		{
			m_activeClientInfo.clear();
			m_activeClientIP.clear();
		}

		return true;
	}

	bool TuningServiceWorker::setActiveClient(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		AUTO_LOCK(m_mainMutex);

		if (m_cfgSettings.singleLmControl == true)
		{
			m_activeClientInfo = softwareInfo;
			m_activeClientIP = clientIP;
		}
		else
		{
			m_activeClientInfo.clear();
			m_activeClientIP.clear();
		}

		return true;
	}

	QString TuningServiceWorker::activeClientID() const
	{
		QString clientID;

		m_mainMutex.lock();

		clientID = m_activeClientInfo.equipmentID();

		m_mainMutex.unlock();

		return clientID;
	}

	QString TuningServiceWorker::activeClientIP() const
	{
		QString clientIP;

		m_mainMutex.lock();

		clientIP = m_activeClientIP;

		m_mainMutex.unlock();

		return clientIP;
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
		m_cfgLoaderThread = new CfgLoaderThread(softwareInfo(), 1, cfgServiceIP1(), cfgServiceIP2(), false, m_logger);

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
		stopTuningSourceThreads();
		clearServiceMaps();

		m_mainMutex.unlock();
	}

	void TuningServiceWorker::applyNewConfiguration()
	{
		DEBUG_LOG_MSG(m_logger, QString("Apply new configuration"));

		m_mainMutex.lock();

		buildServiceMaps();
		runTuningSourceThreads();
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
		TcpTuningServer* tcpTuningSever = new TcpTuningServer(*this, m_tuningSources, m_logger);

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

	bool TuningServiceWorker::readTuningDataSources(const QByteArray& fileData)
	{
		bool result = true;

		result = DataSourcesXML<TuningSource>::readFromXml(fileData, &m_tuningSources);

		if (result == true)
		{
			m_tuningSources.buildMaps();
		}

		return result;
	}

	void TuningServiceWorker::runTuningSourceThreads()
	{
		if (m_cfgSettings.singleLmControl == false)
		{
			// running all TuningSourceWorkers at once if SingleLmControl is disabled
			//
			runTuningSourceThread("");
		}
	}

	bool TuningServiceWorker::runTuningSourceThread(const QString& tuningSourceEquipmentID)
	{
		// if tuningSourceEquipmentID empty - run all sources workers
		// else - run specific source worker
		//
		assert(m_sourceThreadMap.size() == 0);

		bool result = false;

		for(const TuningSource& tuningSource : m_tuningSources)
		{
			if (tuningSourceEquipmentID.isEmpty() == false && tuningSource.lmEquipmentID() != tuningSourceEquipmentID)
			{
				continue;
			}

			if (tuningSource.hasTuningSignals() == false)
			{
				DEBUG_LOG_MSG(m_logger,
							  QString("Tuning source %1 has no signals. Controlling thread wouldn't be run.").
							  arg(tuningSource.lmEquipmentID()));
				continue;
			}

			// create TuningSourceWorkerThreads and fill m_sourceWorkerThreadMap
			//
			TuningSourceThread* sourceThread = new TuningSourceThread(m_cfgSettings, tuningSource, m_logger, m_tuningLog);

			m_sourceThreadMap.insert(tuningSource.lmAddress32(), sourceThread);

			sourceThread->start();
			sourceThread->waitWhileHandlerInitialized();

			setSourceThreadInTuningClientContexts(sourceThread);

			result = true;
		}

		return result;
	}

	void TuningServiceWorker::stopTuningSourceThreads()
	{
		// stop and delete TuningSourcThreads
		//
		for(TuningSourceThread* sourceThread : m_sourceThreadMap)
		{
			if (sourceThread == nullptr)
			{
				assert(false);
				continue;
			}

			removeSourceThreadFromTuningClientContexts(sourceThread);

			sourceThread->quitAndWait();
			delete sourceThread;
		}

		m_sourceThreadMap.clear();
	}

	void TuningServiceWorker::runSourcesListenerThread()
	{
		if (m_sourceThreadMap.size() == 0)
		{
			DEBUG_LOG_MSG(m_logger, QString("Tuning sources workers is not running. Listener thread is not run also."));
			return;
		}

		// create and run TuningSocketListenerThread
		//
		assert(m_socketListenerThread == nullptr);

		m_socketListenerThread = new TuningSocketListenerThread(m_cfgSettings.tuningDataIP, m_sourceThreadMap, m_logger);
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

	void TuningServiceWorker::setSourceThreadInTuningClientContexts(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		for(TuningClientContext* clientContext : m_clientContextMap)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			clientContext->setSourceThread(thread);
		}
	}

	void TuningServiceWorker::removeSourceThreadFromTuningClientContexts(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		for(TuningClientContext* clientContext : m_clientContextMap)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			clientContext->removeSourceThread(thread);
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

		//

		if (m_tuningSimIP.isSet() == true)
		{
			m_cfgSettings.tuningSimIP = m_tuningSimIP;
		}

		//

		DEBUG_LOG_MSG(m_logger, QString("Configuration reading success"));

		for(Builder::BuildFileInfo bfi : buildFileInfoArray)
		{
			QByteArray fileData;
			QString errStr;

			m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

			if (errStr.isEmpty() == false)
			{
				qDebug() << errStr;
				result = false;
				continue;
			}

			result = true;

			if (bfi.ID == CfgFileId::TUNING_SOURCES)
			{
				result &= readTuningDataSources(fileData);
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
			clearConfiguration();
			applyNewConfiguration();
		}
	}
}
