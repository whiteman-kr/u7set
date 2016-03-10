#include "DataAquisitionService.h"
#include <QXmlStreamReader>
#include "../include/DeviceObject.h"
#include <QMetaProperty>


// DataServiceMainFunctionWorker class implementation
//

void DataServiceWorker::initDataSources()
{
	InitDataSources(m_dataSources, m_deviceRoot.get(), m_signalSet);
}


void DataServiceWorker::initListeningPorts()
{
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.11.254", 2000));
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.12.254", 2000));
}

void DataServiceWorker::readConfigurationFiles()
{
	SerializeEquipmentFromXml("equipment.xml", m_deviceRoot);
	SerializeSignalsFromXml("appSignals.xml", m_unitInfo, m_signalSet);
}

void DataServiceWorker::runUdpThreads()
{
	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_DATA_AQUISITION_SERVICE_INFO);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &DataServiceWorker::onInformationRequest);
	connect(this, &DataServiceWorker::ackInformationRequest, serverSocket, &UdpServerSocket::sendAck);

	m_infoSocketThread = new UdpSocketThread(serverSocket);

	m_infoSocketThread->start();
}


void DataServiceWorker::stopUdpThreads()
{
	delete m_infoSocketThread;
}


void DataServiceWorker::runFscDataReceivingThreads()
{
	for(int i = 0; i < m_fscDataAcquisitionAddressPorts.count(); i++)
	{
		FscDataAcquisitionThread* dataAcquisitionThread = new FscDataAcquisitionThread(m_fscDataAcquisitionAddressPorts[i]);

		m_fscDataAcquisitionThreads.append(dataAcquisitionThread);
	}
}


void DataServiceWorker::runCfgLoaderThread()
{
	CfgLoader* cfgLoader = new CfgLoader("SYSTEMID_RACKID_WS00_DACQSERVICE", 1, HostAddressPort("127.0.0.1", PORT_CONFIGURATION_SERVICE_REQUEST), HostAddressPort("227.33.0.1", PORT_CONFIGURATION_SERVICE_REQUEST));

    m_cfgLoaderThread = new CfgLoaderThread(cfgLoader);

	connect(cfgLoader, &CfgLoader::signal_configurationReady, this, &DataServiceWorker::onConfigurationReady);

	m_cfgLoaderThread->start();
}


void DataServiceWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(false);
		return;
	}

	m_cfgLoaderThread->quit();

	delete m_cfgLoaderThread;
}


void DataServiceWorker::stopFscDataReceivingThreads()
{
	for(int i = 0; i < m_fscDataAcquisitionThreads.count(); i++)
	{
		delete m_fscDataAcquisitionThreads[i];
	}

	m_fscDataAcquisitionThreads.clear();
}


void DataServiceWorker::initialize()
{
	// Service Main Function initialization
	//

	runCfgLoaderThread();

	readConfigurationFiles();
	initDataSources();

	runUdpThreads();
	runFscDataReceivingThreads();

	qDebug() << "DataServiceMainFunctionWorker initialized";
}


void DataServiceWorker::shutdown()
{
	// Service Main Function deinitialization
	//

	stopFscDataReceivingThreads();
	stopUdpThreads();

	stopCfgLoaderThread();

	qDebug() << "DataServiceWorker stoped";
}


void DataServiceWorker::onInformationRequest(UdpRequest request)
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


void DataServiceWorker::onGetDataSourcesIDs(UdpRequest& request)
{
	int dataSourcesCount = m_dataSources.count();

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

	emit ackInformationRequest(ack);
}


void DataServiceWorker::onGetDataSourcesInfo(UdpRequest& request)
{
	quint32 count = request.readDword();

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

	ackInformationRequest(ack);
}


void DataServiceWorker::onGetDataSourcesState(UdpRequest& request)
{
	quint32 count = request.readDword();

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

	ackInformationRequest(ack);
}


void DataServiceWorker::onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	qDebug() << "Configuration Ready!";

    if (m_cfgLoaderThread == nullptr)
    {
        return;
    }

    for(Builder::BuildFileInfo bfi : buildFileInfoArray)
    {
        QByteArray fileData;
        QString errStr;

        bool result = m_cfgLoaderThread->downloadCfgFile(bfi.pathFileName, &fileData, &errStr);

        if (result == true)
        {
            qDebug() << "File " << bfi.pathFileName << " download OK";
        }
    }
}


