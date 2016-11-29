#include "../lib/WUtils.h"
#include "TuningService.h"


namespace Tuning
{


	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningServiceWorker::TuningServiceWorker(const QString& serviceEquipmentID,
											 const QString& cfgServiceIP1,
											 const QString& cfgServiceIP2,
											 const QString& buildPath) :
		ServiceWorker(ServiceType::TuningService, serviceEquipmentID, cfgServiceIP1, cfgServiceIP2, buildPath),
		m_timer(this)
	{
	}


	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}


	void TuningServiceWorker::clear()
	{
		m_tuningSources.clear();
	}


	TuningServiceWorker* TuningServiceWorker::createInstance()
	{
		TuningServiceWorker* worker = new TuningServiceWorker(serviceEquipmentID(), cfgServiceIP1(), cfgServiceIP2(), buildPath());

		return worker;
	}


	void TuningServiceWorker::initialize()
	{
		if (buildPath().isEmpty() == true)
		{
			runCfgLoaderThread();
		}
		else
		{
			bool result = loadConfigurationFromFile(cfgFileName());

			if (result == true)
			{
				applyNewConfiguration();
			}
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
		HostAddressPort ip1(cfgServiceIP1(), PORT_CONFIGURATION_SERVICE_REQUEST);
		HostAddressPort ip2(cfgServiceIP2(), PORT_CONFIGURATION_SERVICE_REQUEST);

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
	}


	void TuningServiceWorker::applyNewConfiguration()
	{
		runTuningSourceWorkers();
		runTcpTuningServerThread();
	}


	void TuningServiceWorker::runTcpTuningServerThread()
	{
		TcpTuningServer* tcpTuningSever = new TcpTuningServer();

		m_tcpTuningServerThread = new TcpTuningServerThread(m_tuningSettings.clientRequestIP,
															tcpTuningSever,
															m_tuningSources);
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

		// stop and delete TuningSocketListenerThread
		//
		if (m_socketListenerThread != nullptr)
		{
			m_socketListenerThread->quitAndWait();
			delete m_socketListenerThread;
			m_socketListenerThread = nullptr;
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




/*
	void TuningServiceWorker::onSetSignalState(QString appSignalID, double value)
	{
		if (m_tuningSocket == nullptr)
		{
			assert(false);
			return;
		}

		if (m_signal2Source.contains(appSignalID) == false)
		{
			return;
		}

		QString sourceID = m_signal2Source[appSignalID];

		if (m_dataSources.contains(sourceID) == false)
		{
			return;
		}

		TuningDataSource* source = m_dataSources[sourceID];

		if (source == nullptr)
		{
			assert(false);
			return;
		}

		Tuning::SocketRequest sr;

		bool result = source->setSignalState(appSignalID, value, &sr);

		if (result == false)
		{
			return;
		}

		// send request
		//
		//	allready filled inside source->setSignalState(appSignalID, value, &sr):
		//
		//	sr.dataType
		//	sr.startAddressW - offset of frame!
		//	sr.frameData
		//

		sr.lmIP = source->lmAddress32();
		sr.lmPort = source->lmPort();
		sr.lmNumber = source->lmNumber();
		sr.lmSubsystemID = source->lmSubsystemID();
		sr.uniqueID = source->uniqueID();
		sr.numerator = source->numerator();
		sr.operation = Tuning::OperationCode::Write;
		sr.frameSizeW = m_tuningSettings.tuningRomFrameSizeW;
		sr.romSizeW = m_tuningSettings.tuningRomSizeW;

		sr.startAddressW += m_tuningSettings.tuningDataOffsetW;		// !!!

		sr.userRequest = true;

		source->incNumerator();
		source->setWaitReply();
		source->incSentRequestCount();

		requestPreprocessing(sr);

		m_tuningSocket->sendRequest(sr);
	}*/



}
