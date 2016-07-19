#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../lib/DeviceObject.h"
#include "AppDataService.h"
#include "TcpAppDataServer.h"

// -------------------------------------------------------------------------------
//
// AppDataServiceWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataServiceWorker::AppDataServiceWorker(const QString& serviceStrID,
									 const QString& cfgServiceIP1,
									 const QString& cfgServiceIP2) :
	ServiceWorker(ServiceType::AppDataService, serviceStrID, cfgServiceIP1, cfgServiceIP2),
	m_timer(this)
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		m_appDataChannelThread[channel] = nullptr;
	}
}


AppDataServiceWorker::~AppDataServiceWorker()
{
}


void AppDataServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo)
{
	serviceInfo.set_clientrequestip(m_settings.clientRequestIP.address32());
	serviceInfo.set_clientrequestport(m_settings.clientRequestIP.port());
}


void AppDataServiceWorker::runCfgLoaderThread()
{
	m_cfgLoaderThread = new CfgLoaderThread(serviceStrID(), 1,
											HostAddressPort(cfgServiceIP1(), PORT_CONFIGURATION_SERVICE_REQUEST),
											HostAddressPort(cfgServiceIP2(), PORT_CONFIGURATION_SERVICE_REQUEST));

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &AppDataServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();
}


void AppDataServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(false);
		return;
	}

	m_cfgLoaderThread->quit();

	delete m_cfgLoaderThread;
}


void AppDataServiceWorker::runTcpAppDataServer()
{
	assert(m_tcpAppDataServerThread == nullptr);

	TcpAppDataServer* tcpAppDataSever = new TcpAppDataServer();

	m_tcpAppDataServerThread = new TcpAppDataServerThread(	m_settings.clientRequestIP,
															tcpAppDataSever,
															m_appDataSources,
															m_appSignals,
															m_signalStates,
															m_unitInfo);
	m_tcpAppDataServerThread->start();
}


void AppDataServiceWorker::stopTcpAppDataServer()
{
	if (m_tcpAppDataServerThread != nullptr)
	{
		m_tcpAppDataServerThread->quitAndWait(10000);
		delete m_tcpAppDataServerThread;

		m_tcpAppDataServerThread = nullptr;
	}
}


void AppDataServiceWorker::runTimer()
{
	connect(&m_timer, &QTimer::timeout, this, &AppDataServiceWorker::onTimer);

	m_timer.setInterval(1000);
	m_timer.start();
}


void AppDataServiceWorker::stopTimer()
{
	m_timer.stop();
}


void AppDataServiceWorker::initialize()
{
	// Service Main Function initialization
	//
	runCfgLoaderThread();
	runTimer();
	qDebug() << "DataServiceMainFunctionWorker initialized";
}


void AppDataServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	clearConfiguration();

	stopTimer();

	stopTcpAppDataServer();
	stopCfgLoaderThread();

	qDebug() << "DataServiceWorker stoped";
}


void AppDataServiceWorker::onTimer()
{
}


void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	// stop all AppDataChannelThreads and
	// free all allocated resources
	//
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

		result = true;

		if (bfi.ID == CFG_FILE_ID_DATA_SOURCES)
		{
			result &= readDataSources(fileData);			// fill m_appDataSources
		}

		if (bfi.ID == CFG_FILE_ID_APP_SIGNALS)
		{
			result &= readAppSignals(fileData);				// fill m_unitInfo and m_appSignals
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
		applyNewConfiguration();
	}
}


bool AppDataServiceWorker::readConfiguration(const QByteArray& fileData)
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


bool AppDataServiceWorker::readDataSources(QByteArray& fileData)
{
	XmlReadHelper xml(fileData);

	bool result = true;

	m_appDataSources.clear();

	while (1)
	{
		bool find = xml.findElement(DataSource::ELEMENT_DATA_SOURCE);

		if (find == false)
		{
			break;
		}

		AppDataSource* dataSource = new AppDataSource();

		result &= dataSource->readFromXml(xml);

		if (result == false)
		{
			assert(false);
			qDebug() << "DataSource error reading from xml";
			delete dataSource;
			continue;
		}

		int channel = dataSource->lmChannel();

		if (channel < 0 || channel >= AppDataServiceSettings::DATA_CHANNEL_COUNT)
		{
			assert(false);
			qDebug() << "DataSource wrong channel";
			delete dataSource;
			continue;
		}

		m_appDataSources.insert(dataSource->lmAddress32(), dataSource);

		qDebug() << "DataSource: " << dataSource->lmEquipmentID() << "channel " << dataSource->lmChannel();
	}

	return result;
}


bool AppDataServiceWorker::readAppSignals(QByteArray& fileData)
{
	bool result = true;

	XmlReadHelper xml(fileData);

	if (xml.findElement("Units") == false)
	{
		return false;
	}

	int unitCount = 0;

	result &= xml.readIntAttribute("Count", &unitCount);

	for(int count = 0; count < unitCount; count++)
	{
		if(xml.findElement("Unit") == false)
		{
			result = false;
			break;
		}

		int unitID = 0;
		QString unitCaption;

		result &= xml.readIntAttribute("ID", &unitID);
		result &= xml.readStringAttribute("Caption", &unitCaption);

		m_unitInfo.append(unitID, unitCaption);
	}

	if (m_unitInfo.count() != unitCount)
	{
		qDebug() << "Units loading error";
		assert(false);
		return false;
	}

	if (xml.findElement("Signals") == false)
	{
		return false;
	}

	int signalCount = 0;

	result &= xml.readIntAttribute("Count", &signalCount);

	//quint64 time1 = QDateTime::currentMSecsSinceEpoch();

	for(int count = 0; count < signalCount; count++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

		Signal* signal = new Signal();

		bool res = signal->readFromXml(xml);	// time-expensive function !!!

		if (res == true)
		{
			if (m_appSignals.contains(signal->appSignalID()) == false)
			{
				m_appSignals.insert(signal->appSignalID(), signal);
			}
			else
			{
				res = false;
			}
		}

		if (res == false)
		{
			delete signal;
		}

		result &= res;
	}

	//quint64 time2 = QDateTime::currentMSecsSinceEpoch();
	//qDebug() << "time " << (time2 - time1) << " per 1 " << (time2 - time1)/289;

	m_appSignals.buildHash2Signal();

	return result;
}


void AppDataServiceWorker::createAndInitSignalStates()
{
	m_signalStates.clear();

	if (m_appSignals.isEmpty())
	{
		return;
	}

	int signalCount = m_appSignals.count();

	m_signalStates.setSize(signalCount);

	int index = 0;

	for(Signal* signal : m_appSignals)
	{
		AppSignalStateEx* signalState = m_signalStates[index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setSignalParams(index, signal);

		index++;
	}

	m_signalStates.buidlHash2State();
}


void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopTcpAppDataServer();
	stopDataChannelThreads();

	m_unitInfo.clear();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_signalStates.clear();
}


void AppDataServiceWorker::applyNewConfiguration()
{
	createAndInitSignalStates();

	initDataChannelThreads();

	runDataChannelThreads();

	runTcpAppDataServer();
}


void AppDataServiceWorker::initDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		// create AppDataChannelThread
		//
		m_appDataChannelThread[channel] = new AppDataChannelThread(channel,
						m_settings.appDataServiceChannel[channel].appDataReceivingIP);

		// add AppDataSources to channel
		//
		for(DataSource* dataSource : m_appDataSources)
		{
			if (dataSource->lmDataType() == DataSource::DataType::App &&
				dataSource->lmChannel() == channel)
			{
				m_appDataChannelThread[channel]->addDataSource(dataSource);
			}
		}

		// build required data structures inside AppDataChannel
		//
		m_appDataChannelThread[channel]->prepare(m_appSignals, &m_signalStates);
	}
}


void AppDataServiceWorker::runDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		if (m_appDataChannelThread[channel] != nullptr)
		{
			m_appDataChannelThread[channel]->start();
		}
		else
		{
			assert(false);
		}
	}
}


void AppDataServiceWorker::stopDataChannelThreads()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		if (m_appDataChannelThread[channel] != nullptr)
		{
			m_appDataChannelThread[channel]->quitAndWait();
			delete m_appDataChannelThread[channel];
			m_appDataChannelThread[channel] = nullptr;
		}
	}
}

