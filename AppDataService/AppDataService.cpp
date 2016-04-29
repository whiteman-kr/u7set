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
}


void AppDataServiceWorker::readConfigurationFiles()
{
	SerializeEquipmentFromXml("equipment.xml", m_deviceRoot);
	SerializeSignalsFromXml("appSignals.xml", m_unitInfo, m_signalSet);
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

void AppDataServiceWorker::runDataChannels()
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


void AppDataServiceWorker::initDataChannels()
{
	stopDataChannels();

	for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
	{
		m_appDataChannelThread[channel] = new DataChannelThread(channel,
																AppDataSource::DataType::App,
																m_settings.ethernetChannel[channel].appDataReceivingIP);
	}
}


void AppDataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

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

		if (bfi.ID == CFG_FILE_ID_DATA_SOURCES)
		{
			readDataSources(fileData);
			continue;
		}

		if (bfi.ID == CFG_FILE_ID_DATA_SOURCES)
		{
			readAppSignals(fileData);
			continue;
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

	while (1)
	{
		bool find = xml.findElement(AppDataSource::ELEMENT_APP_DATA_SOURCE);

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

		qDebug() << "DataSource: " << dataSource->lmStrID() << "channel " << dataSource->channel();

		if (dataSource->dataType() == AppDataSource::DataType::App)
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

	m_unitInfo.clear();

	while(xml.findElement("Unit") == true)
	{
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

	if (xml.findElement("Units") == false)
	{
		return false;
	}

	m_appSignals.clear();

	if (xml.findElement("Signals") == false)
	{
		return false;
	}

	int signalCount = 0;

	result &= xml.readIntAttribute("Count", &signalCount);

	m_appSignals.resize(signalCount);

	int count = 0;

	while(xml.findElement("Signal") == true)
	{
		if (count < signalCount)
		{
			result &= m_appSignals[count].readFromoXml(xml);

			count++;
		}
		else
		{
			assert(false);
			break;
		}
	}

	return result;
}
