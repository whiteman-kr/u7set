#include "DataSource.h"
#include "WUtils.h"
#include "Crc.h"
#include "DeviceHelper.h"

// ---------------------------------------------------------------------------------
//
//	DataSource::LmEthernetAdapterProperties class implementation
//
// ---------------------------------------------------------------------------------

const char* DataSource::LmEthernetAdapterProperties::PROP_TUNING_ENABLE = "TuningEnable";
const char* DataSource::LmEthernetAdapterProperties::PROP_TUNING_IP = "TuningIP";
const char* DataSource::LmEthernetAdapterProperties::PROP_TUNING_PORT = "TuningPort";
const char* DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID = "TuningServiceID";

const char* DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_ENABLE = "AppDataEnable";
const char* DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_IP = "AppDataIP";
const char* DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_PORT = "AppDataPort";
const char* DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID = "AppDataServiceID";
const char* DataSource::LmEthernetAdapterProperties::PROP_LM_APP_DATA_UID = "AppLANDataUID";
const char* DataSource::LmEthernetAdapterProperties::PROP_LM_APP_DATA_SIZE = "AppLANDataSize";

const char* DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_ENABLE = "DiagDataEnable";
const char* DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_IP = "DiagDataIP";
const char* DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_PORT = "DiagDataPort";
const char* DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID = "DiagDataServiceID";
const char* DataSource::LmEthernetAdapterProperties::PROP_LM_DIAG_DATA_UID = "DiagLANDataUID";
const char* DataSource::LmEthernetAdapterProperties::PROP_LM_DIAG_DATA_SIZE= "DiagLANDataSize";

const char* DataSource::LmEthernetAdapterProperties::LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR = "_ETHERNET0%1";

bool DataSource::LmEthernetAdapterProperties::getLmEthernetAdapterNetworkProperties(const Hardware::DeviceModule* lm, int adptrNo, Builder::IssueLogger* log)
{
	if (log == nullptr)
	{
		assert(false);
		return false;
	}

	if (lm == nullptr)
	{
		LOG_INTERNAL_ERROR(log);
		assert(false);
		return false;
	}

	if (adptrNo < LM_ETHERNET_ADAPTER1 ||
		adptrNo > LM_ETHERNET_ADAPTER3)
	{
		LOG_INTERNAL_ERROR(log);
		assert(false);
		return false;
	}

	adapterNo = adptrNo;

	QString suffix = QString(LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR).arg(adapterNo);

	Hardware::DeviceController* adapter = DeviceHelper::getChildControllerBySuffix(lm, suffix, log);

	if (adapter == nullptr)
	{
		return false;
	}

	adapterID = adapter->equipmentIdTemplate();

	bool result = true;

	if (adptrNo == LM_ETHERNET_ADAPTER1)
	{
		// tunig adapter
		//
		result &= DeviceHelper::getBoolProperty(adapter, PROP_TUNING_ENABLE, &tuningEnable, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_TUNING_IP, &tuningIP, log);
		result &= DeviceHelper::getIntProperty(adapter, PROP_TUNING_PORT, &tuningPort, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_TUNING_SERVICE_ID, &tuningServiceID, log);
		return result;
	}

	if (adptrNo == LM_ETHERNET_ADAPTER2 ||
		adptrNo == LM_ETHERNET_ADAPTER3)
	{
		int dataUID = 0;

		// application data adapter
		//
		result &= DeviceHelper::getBoolProperty(adapter, PROP_APP_DATA_ENABLE, &appDataEnable, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_APP_DATA_IP, &appDataIP, log);
		result &= DeviceHelper::getIntProperty(adapter, PROP_APP_DATA_PORT, &appDataPort, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_APP_DATA_SERVICE_ID, &appDataServiceID, log);

		result &= DeviceHelper::getIntProperty(lm, PROP_LM_APP_DATA_UID, &dataUID, log);

		appDataUID = dataUID;

		result &= DeviceHelper::getIntProperty(lm, PROP_LM_APP_DATA_SIZE, &appDataSize, log);

		appDataSize *= sizeof(quint16);		// size in words convert to size in bytes

		appDataFramesQuantity = appDataSize / sizeof(Rup::Frame::data) +
				((appDataSize % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);

		// diagnostics data adapter
		//
		result &= DeviceHelper::getBoolProperty(adapter, PROP_DIAG_DATA_ENABLE, &diagDataEnable, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_DIAG_DATA_IP, &diagDataIP, log);
		result &= DeviceHelper::getIntProperty(adapter, PROP_DIAG_DATA_PORT, &diagDataPort, log);
		result &= DeviceHelper::getStrProperty(adapter, PROP_DIAG_DATA_SERVICE_ID, &diagDataServiceID, log);

		diagDataUID = 0;
		diagDataSize = 0;
		diagDataFramesQuantity = 0;

/*		UNCOMMENT when LM will have PROP_LM_DIAG_DATA_UID and PROP_LM_DIAG_DATA_SIZE properties  !!!
 *
 * 		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_UID, &dataUID, log);

		diagDataUID = dataUID;

		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_SIZE, &diagDataSize, log);

		diagDataFramesQuantity = diagDataSize / sizeof(Rup::Frame::data) +
				((diagDataSize % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);
*/

		return result;
	}

	assert(false);
	return false;
}



// -----------------------------------------------------------------------------
//
// DataSource class implementation
//
// -----------------------------------------------------------------------------

const char* const DataSource::ELEMENT_DATA_SOURCES = "DataSources";
const char* const DataSource::ELEMENT_DATA_SOURCE = "DataSource";
const char* const DataSource::ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS = "AssociatedSignals";

const char* DataSource::DATA_TYPE_APP = "App";
const char* DataSource::DATA_TYPE_DIAG = "Diag";
const char* DataSource::DATA_TYPE_TUNING = "Tuning";

const char* DataSource::PROP_DEVICE_LM_NUMBER = "LMNumber";
const char* DataSource::PROP_DEVICE_SUBSYSTEM_CHANNEL = "SubsystemChannel";
const char* DataSource::PROP_DEVICE_SUBSYSTEM_ID = "SubsystemID";

const char* DataSource::PROP_LM_DATA_TYPE = "LmDataType";
const char* DataSource::PROP_LM_ID = "LmEquipmentID";
const char* DataSource::PROP_LM_NUMBER = "LmNumber";
const char* DataSource::PROP_LM_CHANNEL = "LmChannel";
const char* DataSource::PROP_LM_SUBSYSTEM_KEY = "LmSubsystemKey";
const char* DataSource::PROP_LM_SUBSYSTEM = "LmSubsystem";
const char* DataSource::PROP_LM_MODULE_TYPE = "LmModuleType";
const char* DataSource::PROP_LM_CAPTION = "LmCaption";
const char* DataSource::PROP_LM_ADAPTER_ID = "LmAdapterID";
const char* DataSource::PROP_LM_DATA_ENABLE = "LmDataEnable";
const char* DataSource::PROP_LM_DATA_IP = "LmDataIP";
const char* DataSource::PROP_LM_DATA_PORT = "LmDataPort";
const char* DataSource::PROP_LM_DATA_SIZE = "LmDataSize";
const char* DataSource::PROP_LM_RUP_FRAMES_QUANTITY = "LmRupFramesQuantity";
const char* DataSource::PROP_LM_DATA_ID = "LmDataID";
const char* DataSource::PROP_LM_UNIQUE_ID = "LmUniqueID";
const char* DataSource::PROP_SERVICE_ID = "ServiceID";
const char* DataSource::PROP_COUNT = "Count";


DataSource::DataSource()
{
}

DataSource::~DataSource()
{
}

bool DataSource::getLmPropertiesFromDevice(const Hardware::DeviceModule* lm,
										   DataType dataType,
										   int adapterNo,
										   const SubsystemKeyMap& subsystemKeyMap,
										   const QHash<QString, quint64>& lmUniqueIdMap,
										   Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lm, log);

	m_lmDataType = dataType;
	m_lmEquipmentID = lm->equipmentIdTemplate();
	m_lmModuleType = lm->moduleType();
	m_lmCaption = lm->caption();

	bool result = true;

	result &= DeviceHelper::getIntProperty(lm, PROP_DEVICE_LM_NUMBER, &m_lmNumber, log);
	result &= DeviceHelper::getStrProperty(lm, PROP_DEVICE_SUBSYSTEM_CHANNEL, &m_lmSubsystemChannel, log);
	result &= DeviceHelper::getStrProperty(lm, PROP_DEVICE_SUBSYSTEM_ID, &m_lmSubsystem, log);

	if (subsystemKeyMap.contains(m_lmSubsystem) == false)
	{
		// Subsystem '%1' is not found in subsystem set (Logic Module '%2')
		//
		log->errCFG3001(m_lmSubsystem, lm->equipmentIdTemplate());
		return false;
	}

	m_lmSubsystemKey = subsystemKeyMap.value(m_lmSubsystem);
	m_lmUniqueID = lmUniqueIdMap.value(lm->equipmentIdTemplate(), 0);

	LmEthernetAdapterProperties adapterProp;

	result &= adapterProp.getLmEthernetAdapterNetworkProperties(lm, adapterNo, log);

	m_lmAdapterID = adapterProp.adapterID;

	switch(m_lmDataType)
	{
	case DataType::App:
		m_lmDataEnable = adapterProp.appDataEnable;
		m_lmAddressPort.setAddressPort(adapterProp.appDataIP, adapterProp.appDataPort);
		m_lmDataID = adapterProp.appDataUID;
		m_lmDataSize = adapterProp.appDataSize;
		m_lmRupFramesQuantity = adapterProp.appDataFramesQuantity;
		m_serviceID = adapterProp.appDataServiceID;
		break;

	case DataType::Diag:
		m_lmDataEnable = adapterProp.diagDataEnable;
		m_lmAddressPort.setAddressPort(adapterProp.diagDataIP, adapterProp.diagDataPort);
		m_lmDataID = adapterProp.diagDataUID;
		m_lmDataSize = adapterProp.diagDataSize;
		m_lmRupFramesQuantity = adapterProp.diagDataFramesQuantity;
		m_serviceID = adapterProp.diagDataServiceID;
		break;

	case DataType::Tuning:
		m_lmDataEnable = adapterProp.tuningEnable;
		m_lmAddressPort.setAddressPort(adapterProp.tuningIP, adapterProp.tuningPort);
		m_lmDataID = 0;
		m_lmDataSize = 0;
		m_lmRupFramesQuantity = 0;
		m_serviceID = adapterProp.tuningServiceID;
		break;

	default:
		assert(false);
	}

	return result;
}

QString DataSource::dataTypeToString(DataType dataType)
{
	switch(dataType)
	{
	case DataType::App:
		return DATA_TYPE_APP;

	case DataType::Diag:
		return DATA_TYPE_DIAG;

	case DataType::Tuning:
		return DATA_TYPE_TUNING;

	default:
		assert(false);
	}

	return "???";
}

DataSource::DataType DataSource::stringToDataType(const QString& dataTypeStr)
{
	if (dataTypeStr == DATA_TYPE_APP)
	{
		return DataType::App;
	}

	if (dataTypeStr == DATA_TYPE_DIAG)
	{
		return DataType::Diag;
	}

	if (dataTypeStr == DATA_TYPE_TUNING)
	{
		return DataType::Tuning;
	}

	assert(false);

	return DataType::Diag;
}

void DataSource::writeToXml(XmlWriteHelper& xml) const
{
	xml.writeStartElement(ELEMENT_DATA_SOURCE);

	xml.writeStringAttribute(PROP_LM_DATA_TYPE, dataTypeToString(m_lmDataType));
	xml.writeStringAttribute(PROP_LM_ID, m_lmEquipmentID);

	xml.writeIntAttribute(PROP_LM_MODULE_TYPE, m_lmModuleType, true);
	xml.writeStringAttribute(PROP_LM_SUBSYSTEM, m_lmSubsystem);
	xml.writeIntAttribute(PROP_LM_SUBSYSTEM_KEY, m_lmSubsystemKey);
	xml.writeIntAttribute(PROP_LM_NUMBER, m_lmNumber);
	xml.writeStringAttribute(PROP_LM_CHANNEL, m_lmSubsystemChannel);

	xml.writeStringAttribute(PROP_LM_CAPTION, m_lmCaption);
	xml.writeStringAttribute(PROP_LM_ADAPTER_ID, m_lmAdapterID);
	xml.writeBoolAttribute(PROP_LM_DATA_ENABLE, m_lmDataEnable);
	xml.writeStringAttribute(PROP_LM_DATA_IP, m_lmAddressPort.addressStr());
	xml.writeIntAttribute(PROP_LM_DATA_PORT, m_lmAddressPort.port());
	xml.writeIntAttribute(PROP_LM_DATA_SIZE, m_lmDataSize);
	xml.writeIntAttribute(PROP_LM_RUP_FRAMES_QUANTITY, m_lmRupFramesQuantity);
	xml.writeUInt32Attribute(PROP_LM_DATA_ID, m_lmDataID, true);
	xml.writeUInt64Attribute(PROP_LM_UNIQUE_ID, m_lmUniqueID, true);

	xml.writeStringAttribute(PROP_SERVICE_ID, m_serviceID);

	xml.writeStartElement(ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS);

	xml.writeIntAttribute(PROP_COUNT, m_associatedSignals.count());

	xml.writeString(m_associatedSignals.join(","));

	xml.writeEndElement();	// </AssociatedSignals>

	writeAdditionalSectionsToXml(xml);

	xml.writeEndElement();	// </AppDataSource>
}

bool DataSource::readFromXml(XmlReadHelper& xml)
{
	if (xml.findElement(ELEMENT_DATA_SOURCE) == false)
	{
		return false;
	}

	bool result = true;

	QString str;

	result &= xml.readStringAttribute(PROP_LM_DATA_TYPE, &str);
	m_lmDataType = stringToDataType(str);

	result &= xml.readStringAttribute(PROP_LM_ID, &m_lmEquipmentID);

	result &= xml.readIntAttribute(PROP_LM_MODULE_TYPE, &m_lmModuleType);
	result &= xml.readStringAttribute(PROP_LM_SUBSYSTEM,&m_lmSubsystem);
	result &= xml.readIntAttribute(PROP_LM_SUBSYSTEM_KEY, &m_lmSubsystemKey);
	result &= xml.readIntAttribute(PROP_LM_NUMBER, &m_lmNumber);
	result &= xml.readStringAttribute(PROP_LM_CHANNEL,&m_lmSubsystemChannel);

	result &= xml.readStringAttribute(PROP_LM_CAPTION, &m_lmCaption);
	result &= xml.readStringAttribute(PROP_LM_ADAPTER_ID, &m_lmAdapterID);
	result &= xml.readBoolAttribute(PROP_LM_DATA_ENABLE, &m_lmDataEnable);

	QString ipStr;
	int port = 0;

	result &= xml.readStringAttribute(PROP_LM_DATA_IP, &ipStr);
	result &= xml.readIntAttribute(PROP_LM_DATA_PORT, &port);

	m_lmAddressPort.setAddress(ipStr);
	m_lmAddressPort.setPort(port);

	result &= xml.readIntAttribute(PROP_LM_DATA_SIZE, &m_lmDataSize);
	result &= xml.readIntAttribute(PROP_LM_RUP_FRAMES_QUANTITY, &m_lmRupFramesQuantity);

	result &= xml.readUInt32Attribute(PROP_LM_DATA_ID, &m_lmDataID);
	result &= xml.readUInt64Attribute(PROP_LM_UNIQUE_ID, &m_lmUniqueID);

	result &= xml.readStringAttribute(PROP_SERVICE_ID, &m_lmCaption);

	if (xml.findElement(ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS) == false)
	{
		return false;
	}

	int signalCount = 0;

	result &= xml.readIntAttribute(PROP_COUNT, &signalCount);

	QString signalIDs;

	result &= xml.readStringElement(ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS, &signalIDs);

	m_associatedSignals = signalIDs.split(",", QString::SkipEmptyParts);

	if (signalCount != m_associatedSignals.count())
	{
		assert(false);
		return false;
	}

	result &= readAdditionalSectionsFromXml(xml);

	m_id = generateID();

	return result;
}

void DataSource::writeAdditionalSectionsToXml(XmlWriteHelper&) const
{
}

bool DataSource::readAdditionalSectionsFromXml(XmlReadHelper&)
{
	return true;
}

bool DataSource::getInfo(Network::DataSourceInfo* proto) const
{
	if (proto == nullptr)
	{
		assert(false);
		return false;
	}

	proto->set_id(m_id);
	proto->set_lmequipmentid(m_lmEquipmentID.toStdString());
	proto->set_lmcaption(m_lmCaption.toStdString());
	proto->set_lmdatatype(TO_INT(m_lmDataType));
	proto->set_lmip(m_lmAddressPort.addressStr().toStdString());
	proto->set_lmport(m_lmAddressPort.port());
	proto->set_lmsubsystemid(m_lmSubsystemKey);
	proto->set_lmsubsystem(m_lmSubsystem.toStdString());
	proto->set_lmsubsystemchannel(m_lmSubsystemChannel.toStdString());
	proto->set_lmnumber(m_lmNumber);
	proto->set_lmmoduletype(m_lmModuleType);
	proto->set_lmadapterid(m_lmAdapterID.toStdString());
	proto->set_lmdataenable(m_lmDataEnable);
	proto->set_lmdataid(m_lmDataID);
	proto->set_lmuniqueid(m_lmUniqueID);
	proto->set_lmrupframesquantity(m_lmRupFramesQuantity);

	return true;
}


bool DataSource::setInfo(const Network::DataSourceInfo& proto)
{
	m_id = proto.id();
	m_lmEquipmentID = QString::fromStdString(proto.lmequipmentid());
	m_lmCaption = QString::fromStdString(proto.lmcaption());
	m_lmDataType = static_cast<DataType>(proto.lmdatatype());
	m_lmAddressPort.setAddressPort(QString::fromStdString(proto.lmip()), proto.lmport());
	m_lmSubsystemKey = proto.lmsubsystemid();
	m_lmSubsystem = QString::fromStdString(proto.lmsubsystem());
	m_lmSubsystemChannel = QString::fromStdString(proto.lmsubsystemchannel());
	m_lmNumber = proto.lmnumber();
	m_lmModuleType = proto.lmmoduletype();
	m_lmAdapterID = QString::fromStdString(proto.lmadapterid());
	m_lmDataEnable = proto.lmdataenable();
	m_lmDataID = proto.lmdataid();
	m_lmUniqueID = proto.lmuniqueid();
	m_lmRupFramesQuantity = proto.lmrupframesquantity();

	return true;
}

quint64 DataSource::generateID() const
{
	if (m_lmAdapterID.isEmpty())
	{
		assert(false);
		return 0;
	}

	Crc64 crc;

	crc.add(m_lmAdapterID);
	crc.add(TO_INT(m_lmDataType));
	crc.add(static_cast<int>(m_lmAddressPort.address32()));
	crc.add(static_cast<int>(m_lmAddressPort.port()));

	return crc.result();
}

// -----------------------------------------------------------------------------
//
// DataSourceOnline class implementation
//
// -----------------------------------------------------------------------------

DataSourceOnline::DataSourceOnline() :
	m_rupFrameTimeQueue(200 /*5 * 200 * Rup::MAX_FRAME_COUNT*/)
{
}

DataSourceOnline::~DataSourceOnline()
{
	if (m_rupFramesHeaders != nullptr)
	{
		delete [] m_rupFramesHeaders;
	}

	if (m_rupFramesData != nullptr)
	{
		delete [] m_rupFramesData;
	}
}

bool DataSourceOnline::init()
{
	m_rupFrameTimeQueue.resize(lmRupFramesQuantity() * 200 * 3);			// 3 seconds queue

	return true;
}

bool DataSourceOnline::collect(const RupFrameTime& rupFrameTime)
{
	// rupFrameTime.rupFrame.header already reverseByted !

	const Rup::Header& rupFrameHeader = rupFrameTime.rupFrame.header;

	quint32 framesQuantity = rupFrameHeader.framesQuantity;

	if (framesQuantity > m_framesQuantityAllocated)
	{
		if (reallocate(framesQuantity) == false)
		{
			return false;
		}
	}

	quint32 frameNumber = rupFrameHeader.frameNumber;

	if (frameNumber >= framesQuantity)
	{
		m_errorFrameNo++;
		return false;
	}

	if (frameNumber >= m_framesQuantityAllocated)
	{
		assert(false);
		return false;
	}

	if (frameNumber == 0)
	{
		m_firstFrameServerTime = rupFrameTime.serverTime;
	}

	// copy RUP frame header
	//
	memcpy(m_rupFramesHeaders + frameNumber, &rupFrameHeader, sizeof(rupFrameHeader));

	// copy RUP frame data
	//
	memcpy(m_rupFramesData + frameNumber, &rupFrameTime.rupFrame.data, sizeof(rupFrameTime.rupFrame.data));

	m_receivedFramesCount++;

	// check packet parts
	//
	bool dataReady = true;

	quint16 numerator0 = m_rupFramesHeaders[0].numerator;

	for(quint32 i = 1; i < framesQuantity; i++)
	{
		dataReady &= m_rupFramesHeaders[i].numerator == numerator0;
	}

	if (dataReady == false)
	{
		return false;
	}

	m_receivedPacketCount++;

	const Rup::TimeStamp& timeStamp = m_rupFramesHeaders[0].timeStamp;

	QDateTime plantTime;

	// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
	//
	plantTime.setTimeSpec(Qt::UTC);

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	m_rupDataTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupDataTimes.system.timeStamp = m_firstFrameServerTime;

	QDateTime localTime = QDateTime::fromMSecsSinceEpoch(m_firstFrameServerTime);

	// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
	//
	localTime.setTimeSpec(Qt::UTC);

	m_rupDataTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

	m_rupDataSize = framesQuantity * sizeof(Rup::Data);

	m_lastRupDataTimes = m_rupDataTimes;

	return true;
}

bool DataSourceOnline::getDataToParsing(Times* times, const char** rupData, quint32* rupDataSize, bool* dataReceivingTimeout)
{
	if (m_dataReadyToParsing == false)
	{
		assert(false);
		return false;
	}

#ifdef QT_DEBUG

	if (times == nullptr || rupData == nullptr || rupDataSize == nullptr || dataReceivingTimeout == nullptr)
	{
		assert(false);
		return false;
	}

#endif

	*times = m_rupDataTimes;
	*rupData = reinterpret_cast<const char*>(m_rupFramesData);
	*rupDataSize = m_rupDataSize;
	*dataReceivingTimeout = m_dataRecevingTimeout;

	m_dataReadyToParsing = false;

	return true;
}

bool DataSourceOnline::reallocate(quint32 framesQuantity)
{
	m_dataReadyToParsing = false;					// !!!  m_rupFramesData will be freed

	if (m_rupFramesHeaders != nullptr)
	{
		delete [] m_rupFramesHeaders;
		m_rupFramesHeaders = nullptr;
	}

	if (m_rupFramesData != nullptr)
	{
		delete [] m_rupFramesData;
		m_rupFramesData = nullptr;
	}

	m_framesQuantityAllocated = 0;

	if (framesQuantity < 1 || framesQuantity > Rup::MAX_FRAME_COUNT)
	{
		assert(false);
		return false;
	}

	m_rupFramesHeaders = new Rup::Header[framesQuantity];
	m_rupFramesData = new Rup::Data[framesQuantity];

	m_framesQuantityAllocated = framesQuantity;

	return true;
}

/*
void DataSourceOnline::stop()
{
	setState(E::DataSourceState::Stopped);
	m_dataReceivingRate = 0;
	m_receivedDataSize = 0;
}


void DataSourceOnline::resume()
{
	setState(E::DataSourceState::NoData);
}*/


void DataSourceOnline::pushRupFrame(qint64 serverTime, const Rup::Frame& rupFrame)
{
	RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPush();

	if (rupFrameTime != nullptr)
	{
		rupFrameTime->serverTime = serverTime;
		memcpy(&rupFrameTime->rupFrame, &rupFrame, sizeof(rupFrame));
	}
	else
	{
		// is not an error - queue is full
	}

	m_rupFrameTimeQueue.completePush();

	m_rupFramesQueueSize = m_rupFrameTimeQueue.size();
	m_rupFramesQueueMaxSize = m_rupFrameTimeQueue.maxSize();
}

bool DataSourceOnline::takeProcessingOwnership(const QThread* processingThread)
{
	const QThread* expected = nullptr;

	bool result = m_processingOwner.compare_exchange_strong(expected,  processingThread);

	// if ownership has been taken by processingWorker - function returns TRUE
	//
	// result == FALSE is Ok, means that another thread is already take ownership

	return result;
}

bool DataSourceOnline::releaseProcessingOwnership(const QThread* processingThread)
{
	bool result = m_processingOwner.compare_exchange_strong(processingThread,  nullptr);

	assert(result == true);	// releaseProcessingOwnership must be called by processingWorker == m_processingOwner only !!!

	return result;
}

bool DataSourceOnline::processRupFrameTimeQueue()
{
	int count = 0;

	do
	{
		RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPop();

		if (rupFrameTime == nullptr)
		{
			if (m_dataReceives == true)
			{
				// check m_lastPacketSystemTime
				//
				qint64 now = QDateTime::currentMSecsSinceEpoch();

				if (now - m_lastPacketSystemTime > APP_DATA_SOURCE_TIMEOUT)
				{
					m_rupDataTimes = m_lastRupDataTimes;
					m_rupDataTimes += APP_DATA_SOURCE_TIMEOUT;

					m_dataRecevingTimeout = true;
					m_dataReceives = false;
					m_dataReadyToParsing = true;
				}
			}

			break;	// has no frames to processing, exit from processRupFrameTimeQueue, return FALSE
					//
					// m_rupFrameTimeQueue.completePop is not required
		}

		m_lastPacketSystemTime = QDateTime::currentMSecsSinceEpoch();
		m_state = E::DataSourceState::ReceiveData;

		m_dataRecevingTimeout = false;
		m_dataReceives = true;
		m_receivedFramesCount++;
		m_receivedDataSize += sizeof(Rup::Frame);

		if (m_dataProcessingEnabled == false)
		{
			break;
		}

		do
		{
			Rup::Header& rupFrameHeader = rupFrameTime->rupFrame.header;

			rupFrameHeader.reverseBytes();

			// rupFrame's protocol version checking
			//
			if (rupFrameHeader.protocolVersion != 5)
			{
				m_errorProtocolVersion++;
				break;
			}

			// rupFrame's data ID checking
			//
			if (rupFrameHeader.dataId != lmDataID())
			{
				m_errorDataID++;

				if (m_errorDataID > 0 && (m_errorDataID % 500) == 0)
				{
					QString msg = QString("Wrong DataID from %1 (0x%2, waiting 0x%3), packet processing skiped").
							arg(lmAddressPort().addressStr()).
							arg(QString::number(rupFrameHeader.dataId, 16)).
							arg(QString::number(lmDataID(), 16));

					qDebug() << C_STR(msg);
				}

				break;
			}

			// RupFrame's framesQuantity and frameNo checkng
			//
			if (rupFrameHeader.framesQuantity > Rup::MAX_FRAME_COUNT)
			{
				m_errorFramesQuantity++;
				break;
			}

			if (rupFrameHeader.frameNumber >= rupFrameHeader.framesQuantity)
			{
				m_errorFrameNo++;
				break;
			}

			// collect rupFrames
			//
			m_dataReadyToParsing = collect(*rupFrameTime);

			if (m_dataReadyToParsing == true)
			{
				// rupFrame's numerator tracking
				//
				quint16 numerator = rupFrameHeader.numerator;

				if (m_firstRupFrame == true)
				{
					m_rupFrameNumerator = numerator;
					m_firstRupFrame = false;
				}
				else
				{
					if (m_rupFrameNumerator != numerator)
					{
						if (m_rupFrameNumerator < numerator)
						{
							m_lostedPacketCount += numerator - m_rupFrameNumerator;
						}
						else
						{
							m_lostedPacketCount += 0xFFFF - m_rupFrameNumerator + numerator;
						}

						m_rupFrameNumerator = numerator;
					}
				}

				m_rupFrameNumerator++;
			}

			break;
		}
		while(1);

		m_rupFrameTimeQueue.completePop();

		count++;
	}
	while(m_dataReadyToParsing == false && count < 100);

	return m_dataReadyToParsing;
}


