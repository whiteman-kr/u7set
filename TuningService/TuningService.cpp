#include "../lib/WUtils.h"
#include "TuningService.h"


namespace Tuning
{
	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningServiceWorker::TuningServiceWorker(const QString& serviceName,
											 int& argc,
											 char** argv,
											 const VersionInfo& versionInfo,
											 std::shared_ptr<CircularLogger> logger) :
		ServiceWorker(ServiceType::TuningService, serviceName, argc, argv, versionInfo, logger),
		m_logger(logger),
		m_timer(this)
	{
	}


	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}

	ServiceWorker* TuningServiceWorker::createInstance() const
	{
		ServiceWorker* newInstance = new TuningServiceWorker(serviceName(), argc(), argv(), versionInfo(), m_logger);
		return newInstance;
	}


	void TuningServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
	{
		Q_UNUSED(serviceInfo);
	}


	void TuningServiceWorker::initCmdLineParser()
	{
		CommandLineParser& cp = cmdLineParser();

		cp.addSingleValueOption("id", "Service EquipmentID.", "EQUIPMENT_ID");
		cp.addSingleValueOption("b", "Path to RPCT project build.");
		cp.addSingleValueOption("cfgip1", "IP-addres of first Configuration Service.");
		cp.addSingleValueOption("cfgip2", "IP-addres of second Configuration Service.");
	}


	void TuningServiceWorker::processCmdLineSettings()
	{
		CommandLineParser& cp = cmdLineParser();

		if (cp.optionIsSet("id") == true)
		{
			setStrSetting("EquipmentID", cp.optionValue("id"));
		}

		if (cp.optionIsSet("b") == true)
		{
			setStrSetting("BuildPath", cp.optionValue("b"));
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


	void TuningServiceWorker::loadSettings()
	{
		m_equipmentID = getStrSetting("EquipmentID");

		m_buildPath = getStrSetting("BuildPath");

		m_cfgServiceIP1Str = getStrSetting("CfgServiceIP1");

		m_cfgServiceIP1 = HostAddressPort(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_REQUEST);

		m_cfgServiceIP2Str = getStrSetting("CfgServiceIP2");

		m_cfgServiceIP2 = HostAddressPort(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_REQUEST);

		DEBUG_LOG_MSG(m_logger, QString(tr("Load settings:")));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg("EquipmentID").arg(m_equipmentID));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2")).arg("BuildPath").arg(m_buildPath));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP1").arg(m_cfgServiceIP1Str).arg(m_cfgServiceIP1.addressPortStr()));
		DEBUG_LOG_MSG(m_logger, QString(tr("%1 = %2 (%3)")).arg("CfgServiceIP2").arg(m_cfgServiceIP2Str).arg(m_cfgServiceIP2.addressPortStr()));
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


	void TuningServiceWorker::initialize()
	{
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

		connect(&m_timer, &QTimer::timeout, this, &TuningServiceWorker::onTimer);

		m_timer.setInterval(333);
		m_timer.start();
	}


	void TuningServiceWorker::shutdown()
	{
		clearConfiguration();

		m_timer.stop();

		stopCfgLoaderThread();
	}


	void TuningServiceWorker::runCfgLoaderThread()
	{
		CfgLoader* cfgLoader = new CfgLoader(m_equipmentID, 1, m_cfgServiceIP1, m_cfgServiceIP2, false, m_logger);

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
		stopTuningSourceWorkers();
		clearServiceMaps();
	}


	void TuningServiceWorker::applyNewConfiguration()
	{
		DEBUG_LOG_MSG(m_logger, QString("Apply new configuration"));

		buildServiceMaps();
		runTuningSourceWorkers();
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

		result = xml.findElement("TuningLMs");

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


	void TuningServiceWorker::allocateSignalsAndStates()
	{
		/*QVector<TuningSourceInfo> info;

		m_tuningSources.getTuningDataSourcesInfo(info);*/

/*		// allocate Signals
		//
		m_appSignals.clear();
		m_signal2Source.clear();

		for(const TuningDataSourceInfo& source : info)
		{
			for(const Signal& signal : source.tuningSignals)
			{
				Signal* appSignal = new Signal();

				*appSignal = signal;

				m_appSignals.insert(appSignal->appSignalID(), appSignal);

				if (m_signal2Source.contains(appSignal->appSignalID()))
				{
					assert(false);
					qDebug() << "Duplicate AppSignalID" << appSignal->appSignalID();
				}
				else
				{
					m_signal2Source.insert(appSignal->appSignalID(), source.lmEquipmentID);
				}
			}
		}

		int signalCount = m_appSignals.count();

		// allocate Signal states
		//
		m_appSignalStates.clear();

		m_appSignalStates.setSize(signalCount);

		for(int i = 0; i < signalCount; i++)
		{
			Signal* appSignal = m_appSignals[i];
			AppSignalStateEx* appSignalState = m_appSignalStates[i];

			appSignalState->setSignalParams(i, appSignal);
		}*/
	}


	void TuningServiceWorker::runTuningSourceWorkers()
	{
		// create TuningSourceWorkerThreads and fill m_sourceWorkerThreadMap
		//
		assert(m_sourceWorkerThreadMap.size() == 0);

		for(TuningSource* tuningSource : m_tuningSources)
		{
			if (tuningSource == nullptr)
			{
				assert(false);
				continue;
			}

			TuningSourceWorkerThread* sourceWorkerThread = new TuningSourceWorkerThread(m_cfgSettings, *tuningSource, m_logger);

			if (sourceWorkerThread == nullptr)
			{
				assert(false);
				continue;
			}

			quint32 addr = sourceWorkerThread->sourceIP();

			m_sourceWorkerThreadMap.insert(addr, sourceWorkerThread);

			setWorkerInTuningClientContext(tuningSource->lmEquipmentID(), sourceWorkerThread->worker());
		}

		// create and run TuningSocketListenerThread
		//
		assert(m_socketListenerThread == nullptr);

		m_socketListenerThread = new TuningSocketListenerThread(m_cfgSettings.tuningDataIP, m_sourceWorkerThreadMap, m_logger);
		m_socketListenerThread->start();

		// run TuningSourceWorkerThreads
		//
		for(TuningSourceWorkerThread* sourceWorkerThread : m_sourceWorkerThreadMap)
		{
			sourceWorkerThread->start();
		}
	}


	void TuningServiceWorker::stopTuningSourceWorkers()
	{
		// stop and delete TuningSocketListenerThread
		//
		if (m_socketListenerThread != nullptr)
		{
			m_socketListenerThread->quitAndWait();
			delete m_socketListenerThread;
			m_socketListenerThread = nullptr;
		}

		// stop and delete TuningSourceWorkerThreads
		//
		for(TuningSourceWorkerThread* sourceWorkerThread : m_sourceWorkerThreadMap)
		{
			if (sourceWorkerThread == nullptr)
			{
				assert(false);
				continue;
			}

			sourceWorkerThread->quitAndWait();
			delete sourceWorkerThread;
		}

		m_sourceWorkerThreadMap.clear();
	}


	void TuningServiceWorker::setWorkerInTuningClientContext(const QString& sourceID, TuningSourceWorker* worker)
	{
		for(TuningClientContext* clientContext : m_clientContextMap)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			clientContext->setSourceWorker(sourceID, worker);
		}
	}


	void TuningServiceWorker::onTimer()
	{
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
