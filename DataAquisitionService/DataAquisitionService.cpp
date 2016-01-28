#include "DataAquisitionService.h"
#include <QXmlStreamReader>
#include "../include/DeviceObject.h"
#include <QMetaProperty>

// DataAquisitionService class implementation
//

DataAquisitionService::DataAquisitionService(int argc, char ** argv) :
	BaseService(argc, argv, "FSC Data Aquisition Service", SERVICE_DATA_ACQUISITION, new DataServiceMainFunctionWorker())
{
	Hardware::Init();
}


DataAquisitionService::~DataAquisitionService()
{
}



// DataServiceMainFunctionWorker class implementation
//

void DataServiceMainFunctionWorker::initDataSources()
{
	InitDataSources(m_dataSources, m_deviceRoot.get(), m_signalSet);
}


void DataServiceMainFunctionWorker::initListeningPorts()
{
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.11.254", 2000));
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.12.254", 2000));
}

void DataServiceMainFunctionWorker::readConfigurationFiles()
{
	SerializeEquipmentFromXml("equipment.xml", m_deviceRoot);
	SerializeSignalsFromXml("appSignals.xml", m_unitInfo, m_signalSet);
}

void DataServiceMainFunctionWorker::runUdpThreads()
{
	// Information Socket Thread running
	//
	m_infoSocketThread = new UdpSocketThread();

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::Any, PORT_DATA_AQUISITION_SERVICE_INFO);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &DataServiceMainFunctionWorker::onInformationRequest);
	connect(this, &DataServiceMainFunctionWorker::ackInformationRequest, serverSocket, &UdpServerSocket::sendAck);

	m_infoSocketThread->run(serverSocket);
}


void DataServiceMainFunctionWorker::stopUdpThreads()
{
	delete m_infoSocketThread;
}


void DataServiceMainFunctionWorker::runFscDataReceivingThreads()
{
	for(int i = 0; i < m_fscDataAcquisitionAddressPorts.count(); i++)
	{
		FscDataAcquisitionThread* dataAcquisitionThread = new FscDataAcquisitionThread(m_fscDataAcquisitionAddressPorts[i]);

		m_fscDataAcquisitionThreads.append(dataAcquisitionThread);
	}
}


void DataServiceMainFunctionWorker::runCfgLoaderThread()
{
	CfgLoader* cfgLoader = new CfgLoader("SYSTEMID_RACKID_WS00_DACQSERVICE", 1, HostAddressPort("127.0.0.1", PORT_CONFIGURATION_SERVICE_REQUEST), HostAddressPort("227.33.0.1", PORT_CONFIGURATION_SERVICE_REQUEST));

	m_cfgLoaderThread = new Tcp::Thread(cfgLoader);

	connect(cfgLoader, &CfgLoader::signal_configurationReady, this, &DataServiceMainFunctionWorker::onConfigurationReady);

	m_cfgLoaderThread->start();
}


void DataServiceMainFunctionWorker::stopCfgLoaderThread()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(false);
		return;
	}

	m_cfgLoaderThread->quit();

	delete m_cfgLoaderThread;
}


void DataServiceMainFunctionWorker::stopFscDataReceivingThreads()
{
	for(int i = 0; i < m_fscDataAcquisitionThreads.count(); i++)
	{
		delete m_fscDataAcquisitionThreads[i];
	}

	m_fscDataAcquisitionThreads.clear();
}


void DataServiceMainFunctionWorker::initialize()
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


void DataServiceMainFunctionWorker::shutdown()
{
	// Service Main Function deinitialization
	//

	stopFscDataReceivingThreads();
	stopUdpThreads();

	stopCfgLoaderThread();

	qDebug() << "DataServiceMainFunctionWorker stoped";
}


void DataServiceMainFunctionWorker::onInformationRequest(UdpRequest request)
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


void DataServiceMainFunctionWorker::onGetDataSourcesIDs(UdpRequest& request)
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


void DataServiceMainFunctionWorker::onGetDataSourcesInfo(UdpRequest& request)
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


void DataServiceMainFunctionWorker::onGetDataSourcesState(UdpRequest& request)
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


void DataServiceMainFunctionWorker::onConfigurationReady(const QByteArray /*configurationXmlData*/, const BuildFileInfoArray /*buildFileInfoArray*/)
{
	qDebug() << "Configuration Ready!";
}


