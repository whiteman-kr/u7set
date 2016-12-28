#include "../lib/WUtils.h"
#include "TuningService.h"


namespace Tuning
{


	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningServiceWorker::TuningServiceWorker() :
		ServiceWorker(ServiceType::TuningService),
		m_timer(this)
	{
	}


	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}


	void TuningServiceWorker::initCmdLineParser()
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


	void TuningServiceWorker::clear()
	{
		m_tuningSources.clear();
	}


/*	TuningServiceWorker* TuningServiceWorker::createInstance()
	{
		TuningServiceWorker* worker = new TuningServiceWorker(serviceEquipmentID(), cfgServiceIP1(), cfgServiceIP2(), buildPath());

		return worker;
	}*/


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
		HostAddressPort ip1(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_REQUEST);
		HostAddressPort ip2(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_REQUEST);

		m_cfgLoaderThread = new CfgLoaderThread(serviceEquipmentID(), 1, ip1, ip2);

		connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &TuningServiceWorker::onConfigurationReady);

		m_cfgLoaderThread->start();
		m_cfgLoaderThread->enableDownloadConfiguration();

		QString str = QString("ConfigurationService communication thread is running, IP1 = %1, IP2 = %2").
				arg(ip1.addressPortStr()).arg(ip2.addressPortStr());

		qDebug() << C_STR(str);
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
		stopTcpTuningServerThread();
		stopTuningSourceWorkers();
		clearServiceMaps();
	}


	void TuningServiceWorker::applyNewConfiguration()
	{
		buildServiceMaps();
		runTuningSourceWorkers();
		runTcpTuningServerThread();
	}


	void TuningServiceWorker::buildServiceMaps()
	{
		m_clientContextMap.init(m_tuningSettings, m_tuningSources);
	}


	void TuningServiceWorker::clearServiceMaps()
	{
		m_clientContextMap.clear();
	}


	void TuningServiceWorker::runTcpTuningServerThread()
	{
		TcpTuningServer* tcpTuningSever = new TcpTuningServer(*this);

		m_tcpTuningServerThread = new TcpTuningServerThread(m_tuningSettings.clientRequestIP,
															tcpTuningSever);
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

		result &= m_tuningSettings.readFromXml(xml);
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

			TuningSourceWorkerThread* sourceWorkerThread = new TuningSourceWorkerThread(m_tuningSettings, *tuningSource);

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

		m_socketListenerThread = new TuningSocketListenerThread(m_tuningSettings.tuningDataIP, m_sourceWorkerThreadMap);
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

		clearConfiguration();

		bool result = true;

		result = readConfiguration(configurationXmlData);

		if (result == false)
		{
			return;
		}

		applyNewConfiguration();
	}
}
