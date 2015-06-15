#include "DataAquisitionService.h"
#include <QXmlStreamReader>
#include "../include/DeviceObject.h"
#include <QMetaProperty>

// DataAquisitionService class implementation
//

DataAquisitionService::DataAquisitionService(int argc, char ** argv) :
	BaseService(argc, argv, "FSC Data Aquisition Service", STP_FSC_ACQUISITION, new DataServiceMainFunctionWorker())
{
}


DataAquisitionService::~DataAquisitionService()
{
}



// DataServiceMainFunctionWorker class implementation
//

void DataServiceMainFunctionWorker::initDataSources()
{
	m_dataSources.clear();

	if (m_deviceRoot == nullptr)
	{
		return;
	}

	Hardware::equipmentWalker(m_deviceRoot.get(), [this](Hardware::DeviceObject* currentDevice)
	{
		if (currentDevice == nullptr)
		{
			return;
		}
		if (typeid(*currentDevice) != typeid(Hardware::DeviceModule))
		{
			return;
		}
		Hardware::DeviceModule* currentModule = dynamic_cast<Hardware::DeviceModule*>(currentDevice);
		if (currentModule == nullptr)
		{
			return;
		}
		if (currentModule->moduleFamily() != Hardware::DeviceModule::LM)
		{
			return;
		}
		if (currentModule->property("Network\\RegServerIP").isValid())
		{
			int key = m_dataSources.count() + 1;
			QString ipStr = currentModule->property("Network\\RegServerIP").toString();
			QHostAddress ha(ipStr);
			quint32 ip = ha.toIPv4Address();
			DataSource ds(ip, QString("Data Source %1").arg(key), ha, 1);
			m_dataSources.insert(key, ds);

		}
	});
}


void DataServiceMainFunctionWorker::initListeningPorts()
{
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.11.254", 2000));
	m_fscDataAcquisitionAddressPorts.append(HostAddressPort("192.168.12.254", 2000));
}

void DataServiceMainFunctionWorker::readConfigurationFiles()
{
	readEquipmentConfig();
	readApplicationSignalsConfig();
}

void DataServiceMainFunctionWorker::readEquipmentConfig()
{
	QXmlStreamReader equipmentReader;
	QFile file("equipment.xml");

	Hardware::DeviceObject* pCurrentDevice;

	if (file.open(QIODevice::ReadOnly))
	{
		equipmentReader.setDevice(&file);
		Hardware::Init();

		while (!equipmentReader.atEnd())
		{
			QXmlStreamReader::TokenType token = equipmentReader.readNext();

			switch (token)
			{
			case QXmlStreamReader::StartElement:
			{
				const QXmlStreamAttributes& attr = equipmentReader.attributes();
				const QString classNameHash = attr.value("classNameHash").toString();
				if (classNameHash.isEmpty())
				{
					qDebug() << "Attribute classNameHash of DeviceObject not found";
					continue;
				}
				bool ok = false;
				quint32 hash = classNameHash.toUInt(&ok, 16);
				if (!ok)
				{
					qDebug() << QString("Could not interpret hash %s").arg(classNameHash);
					continue;
				}
				std::shared_ptr<Hardware::DeviceObject> pDeviceObject(Hardware::DeviceObjectFactory.Create(hash));
				if (pDeviceObject == nullptr)
				{
					qDebug() << QString("Unknown element %s found").arg(equipmentReader.name().toString());
					continue;
				}

				if (typeid(*pDeviceObject) == typeid(Hardware::DeviceRoot))
				{
					pCurrentDevice = pDeviceObject.get();
					m_deviceRoot = std::dynamic_pointer_cast<Hardware::DeviceRoot>(pDeviceObject);
					continue;
				}

				if (pCurrentDevice == nullptr)
				{
					qDebug() << "DeviceRoot should be the root xml element";
					return;
				}

				const QMetaObject* metaObject = pDeviceObject->metaObject();
				for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
				{
					const QMetaProperty& property = metaObject->property(i);
					if (property.isValid())
					{
						const char* name = property.name();
						QVariant tmp = QVariant::fromValue(attr.value(name).toString());
						assert(tmp.convert(pDeviceObject->property(name).userType()));
						pDeviceObject->setProperty(name, tmp);
					}
				}

				pDeviceObject->setStrId(attr.value("StrID").toString());
				pDeviceObject->setCaption(attr.value("Caption").toString());
				pDeviceObject->setChildRestriction(attr.value("ChildRestriction").toString());
				pDeviceObject->setPlace(attr.value("Place").toInt());
				int childrenCount = attr.value("ChildrenCount").toInt();
				pDeviceObject->setDynamicProperties(attr.value("DynamicProperties").toString());

				pCurrentDevice->addChild(pDeviceObject);
				if (childrenCount > 0)
				{
					pCurrentDevice = pDeviceObject.get();
				}
				break;
			}
			case QXmlStreamReader::EndElement:
				if (typeid(*pCurrentDevice) != typeid(Hardware::DeviceRoot))
				{
					if (pCurrentDevice->parent() == nullptr)
					{
						assert(false);
						break;
					}
					pCurrentDevice = pCurrentDevice->parent();
				}
				else
				{
					return;	// Closing root element, nothing to read left
				}
				break;
			default:
				continue;
			}
		}
		if (equipmentReader.hasError())
		{
			qDebug() << "Parse equipment.xml error";
		}
	}
}

void DataServiceMainFunctionWorker::readApplicationSignalsConfig()
{
	QXmlStreamReader applicationSignalsReader;
	QFile file("applicationSignals.xml");

	if (file.open(QIODevice::ReadOnly))
	{
		DataFormatList dataFormatInfo;
		applicationSignalsReader.setDevice(&file);

		while (!applicationSignalsReader.atEnd())
		{
			QXmlStreamReader::TokenType token = applicationSignalsReader.readNext();

			switch (token)
			{
			case QXmlStreamReader::StartElement:
			{
				const QXmlStreamAttributes& attr = applicationSignalsReader.attributes();
				if (applicationSignalsReader.name() == "unit")
				{
					m_unitInfo.append(attr.value("ID").toInt(), attr.value("name").toString());
				}
				if (applicationSignalsReader.name() == "signal")
				{
					Signal* pSignal = new Signal;
					pSignal->serializeFields(attr, dataFormatInfo, m_unitInfo);
					m_signalSet.append(pSignal->ID(), pSignal);
				}
				break;
			}
			default:
				continue;
			}
		}
		if (applicationSignalsReader.hasError())
		{
			qDebug() << "Parse equipment.xml error";
		}
	}
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




