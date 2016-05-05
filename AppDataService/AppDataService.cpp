#include <QXmlStreamReader>
#include <QMetaProperty>
#include "../include/DeviceObject.h"
#include "AppDataService.h"


// -------------------------------------------------------------------------------
//
// DataServiceWorker class implementation
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
	clearConfiguration();
}


void AppDataServiceWorker::runUdpThreads()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_APP_DATA_SERVICE_INFO);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &AppDataServiceWorker::onInformationRequest);
	connect(this, &AppDataServiceWorker::ackInformationRequest, serverSocket, &UdpServerSocket::sendAck);

	m_infoSocketThread = new UdpSocketThread(serverSocket);

	m_infoSocketThread->start();
}


void AppDataServiceWorker::stopUdpThreads()
{
	delete m_infoSocketThread;
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
	runUdpThreads();
	runTimer();

	qDebug() << "DataServiceMainFunctionWorker initialized";
}


void AppDataServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//
	stopDataChannels();

	stopTimer();

	//stopFscDataReceivingThreads();
	stopUdpThreads();

	stopCfgLoaderThread();

	qDebug() << "DataServiceWorker stoped";
}


void AppDataServiceWorker::onInformationRequest(UdpRequest request)
{
	switch(request.ID())
	{
	case RQID_GET_DATA_SOURCES_IDS:
		onGetDataSourcesIDs(request);
		break;

	case RQID_GET_DATA_SOURCES_INFO:
		onGetDataSourcesInfo(request);
		break;

	case RQID_GET_DATA_SOURCES_STATISTICS:
		onGetDataSourcesState(request);
		break;

	default:
		assert(false);
	}
}


void AppDataServiceWorker::onGetDataSourcesIDs(UdpRequest& request)
{
	/*int dataSourcesCount = m_dataSources.count();

	QVector<quint32> dataSourcesID;

	dataSourcesID.resize(dataSourcesCount);

	int i = 0;

	QHashIterator<quint32, DataSource> iterator(m_dataSources);

	while (iterator.hasNext() && i < dataSourcesCount)
	{
		iterator.next();

		dataSourcesID[i] = iterator.key();

		i++;
	}

	// Sort IDs by ascending
	//
	for(int i = 0; i < dataSourcesCount - 1; i++)
	{
		for(int j = i + 1; j < dataSourcesCount; j++)
		{
			if (dataSourcesID[i] > dataSourcesID[j])
			{
				quint32 tmp = dataSourcesID[i];
				dataSourcesID[i] = dataSourcesID[j];
				dataSourcesID[j] = tmp;
			}
		}
	}

	UdpRequest ack;

	ack.initAck(request);

	ack.writeDword(dataSourcesCount);

	for(int i = 0; i < dataSourcesCount; i++)
	{
		ack.writeDword(dataSourcesID[i]);
	}

	emit ackInformationRequest(ack);*/
}


void AppDataServiceWorker::onTimer()
{
	static int a = 0;

	a++;

	if (a == 10)
	{
		m_cfgLoaderThread->enableDownloadConfiguration();
	}

}


void AppDataServiceWorker::onGetDataSourcesInfo(UdpRequest& request)
{
/*	quint32 count = request.readDword();

	QVector<DataSourceInfo> dsInfo;

	for(quint32 i = 0; i < count; i++)
	{
		quint32 sourceID = request.readDword();

		if (m_dataSources.contains(sourceID))
		{
			DataSourceInfo dsi;

			DataSource ds = m_dataSources.value(sourceID);

			ds.getInfo(dsi);

			dsInfo.append(dsi);
		}
	}

	UdpRequest ack;

	ack.initAck(request);

	count = static_cast<quint32>(dsInfo.count());

	ack.writeDword(count);

	for(quint32 i = 0; i < count; i++)
	{
		ack.writeStruct(&dsInfo[i]);
	}

	ackInformationRequest(ack);*/
}


void AppDataServiceWorker::onGetDataSourcesState(UdpRequest& request)
{
/*	quint32 count = request.readDword();

	QVector<DataSourceStatistics> dsStatistics;

	for(quint32 i = 0; i < count; i++)
	{
		quint32 sourceID = request.readDword();

		if (m_dataSources.contains(sourceID))
		{
			DataSourceStatistics dss;

			DataSource ds = m_dataSources.value(sourceID);

			ds.getStatistics(dss);

			dsStatistics.append(dss);
		}
	}

	UdpRequest ack;

	ack.initAck(request);

	count = static_cast<quint32>(dsStatistics.count());

	ack.writeDword(count);

	for(quint32 i = 0; i < count; i++)
	{
		ack.writeStruct(&dsStatistics[i]);
	}

	ackInformationRequest(ack);*/
}


void AppDataServiceWorker::initDataChannels()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		m_appDataChannelThread[channel] = new AppDataChannelThread(channel,
						m_settings.appDataServiceChannel[channel].appDataReceivingIP);
	}
}


void AppDataServiceWorker::runDataChannels()
{
	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		m_appDataChannelThread[channel]->prepare(m_appSignals, &m_signalStates);

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


void AppDataServiceWorker::stopDataChannels()
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


void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
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

	initDataChannels();

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
			result &= readDataSources(fileData);
		}

		if (bfi.ID == CFG_FILE_ID_APP_SIGNALS)
		{
			result &= readAppSignals(fileData);
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

	runDataChannels();
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
		bool find = xml.findElement(DataSource::ELEMENT_APP_DATA_SOURCE);

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

		int channel = dataSource->channel();

		if (channel < 0 || channel >= AppDataServiceSettings::DATA_CHANNEL_COUNT)
		{
			assert(false);
			qDebug() << "DataSource wrong channel";
			delete dataSource;
			continue;
		}

		m_appDataSources.insert(dataSource->lmAddress32(), dataSource);

		qDebug() << "DataSource: " << dataSource->lmStrID() << "channel " << dataSource->channel();

		if (dataSource->dataType() == DataSource::DataType::App)
		{
			if (m_appDataChannelThread[channel] != nullptr)
			{
				m_appDataChannelThread[channel]->addDataSource(dataSource);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
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

	for(int count = 0; count < signalCount; count++)
	{
		if (xml.findElement("Signal") == false)
		{
			result = false;
			break;
		}

		Signal* signal = new Signal();

		bool res = signal->readFromoXml(xml);

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

	createAndInitSignalStates();

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
		AppSignalState* signalState = m_signalStates[index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->signal = signal;
		signalState->index = index;

		index++;
	}
}


void AppDataServiceWorker::clearConfiguration()
{
	// free all resources allocated in onConfigurationReady
	//
	stopDataChannels();

	m_unitInfo.clear();

	m_appSignals.clear();
	m_appDataSources.clear();
	m_signalStates.clear();
}
