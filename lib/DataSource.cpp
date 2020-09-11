#include "DataSource.h"
#include "WUtils.h"
#include "Crc.h"
#include "DeviceHelper.h"
#include "LanControllerInfoHelper.h"

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
const char* DataSource::PROP_LM_PRESET_NAME = "LmPresetName";
const char* DataSource::PROP_LM_NUMBER = "LmNumber";
const char* DataSource::PROP_LM_CHANNEL = "LmChannel";
const char* DataSource::PROP_LM_SUBSYSTEM_KEY = "LmSubsystemKey";
const char* DataSource::PROP_LM_SUBSYSTEM_ID = "LmSubsystemID";
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
										   E::LanControllerType lanControllerType,
                                           const Hardware::EquipmentSet& equipmentSet,
										   const SubsystemKeyMap& subsystemKeyMap,
										   const QHash<QString, quint64>& lmUniqueIdMap,
										   Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lm, log);

	m_lmDataType = dataType;
	m_lmEquipmentID = lm->equipmentIdTemplate();
	m_lmPresetName = lm->presetName();
	m_lmModuleType = lm->moduleType();
	m_lmCaption = lm->caption();

	bool result = true;

	result &= DeviceHelper::getIntProperty(lm, PROP_DEVICE_LM_NUMBER, &m_lmNumber, log);
	result &= DeviceHelper::getStrProperty(lm, PROP_DEVICE_SUBSYSTEM_CHANNEL, &m_lmSubsystemChannel, log);
	result &= DeviceHelper::getStrProperty(lm, PROP_DEVICE_SUBSYSTEM_ID, &m_lmSubsystemID, log);

	if (subsystemKeyMap.contains(m_lmSubsystemID) == false)
	{
		// Subsystem '%1' is not found in subsystem set (Logic Module '%2')
		//
		log->errCFG3001(m_lmSubsystemID, lm->equipmentIdTemplate());
		return false;
	}

	m_lmSubsystemKey = subsystemKeyMap.value(m_lmSubsystemID);
	m_lmUniqueID = lmUniqueIdMap.value(lm->equipmentIdTemplate(), 0);

	LanControllerInfo lanControllerInfo;

	result &= LanControllerInfoHelper::getInfo(*lm, adapterNo, lanControllerType, &lanControllerInfo, equipmentSet, log);

	m_lmAdapterID = lanControllerInfo.equipmentID;

	switch(m_lmDataType)
	{
	case DataType::App:

		assert(lanControllerType == E::LanControllerType::AppData || lanControllerType == E::LanControllerType::AppAndDiagData);

		m_lmDataEnable = lanControllerInfo.appDataEnable;
		m_lmAddressPort.setAddressPort(lanControllerInfo.appDataIP, lanControllerInfo.appDataPort);
		m_lmDataID = lanControllerInfo.appDataUID;
		m_lmDataSize = lanControllerInfo.appDataSizeBytes;
		m_lmRupFramesQuantity = lanControllerInfo.appDataFramesQuantity;
		m_serviceID = lanControllerInfo.appDataServiceID;
		break;

	case DataType::Diag:

		assert(lanControllerType == E::LanControllerType::DiagData || lanControllerType == E::LanControllerType::AppAndDiagData);

		m_lmDataEnable = lanControllerInfo.diagDataEnable;
		m_lmAddressPort.setAddressPort(lanControllerInfo.diagDataIP, lanControllerInfo.diagDataPort);
		m_lmDataID = lanControllerInfo.diagDataUID;
		m_lmDataSize = lanControllerInfo.diagDataSizeBytes;
		m_lmRupFramesQuantity = lanControllerInfo.diagDataFramesQuantity;
		m_serviceID = lanControllerInfo.diagDataServiceID;
		break;

	case DataType::Tuning:

		assert(lanControllerType == E::LanControllerType::Tuning);

		m_lmDataEnable = lanControllerInfo.tuningEnable;
		m_lmAddressPort.setAddressPort(lanControllerInfo.tuningIP, lanControllerInfo.tuningPort);
		m_lmDataID = 0;
		m_lmDataSize = 0;
		m_lmRupFramesQuantity = 0;
		m_serviceID = lanControllerInfo.tuningServiceID;
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
	xml.writeStringAttribute(PROP_LM_PRESET_NAME, m_lmPresetName);

	xml.writeIntAttribute(PROP_LM_MODULE_TYPE, m_lmModuleType, true);
	xml.writeStringAttribute(PROP_LM_SUBSYSTEM_ID, m_lmSubsystemID);
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
	result &= xml.readStringAttribute(PROP_LM_PRESET_NAME, &m_lmPresetName);

	result &= xml.readIntAttribute(PROP_LM_MODULE_TYPE, &m_lmModuleType);
	result &= xml.readStringAttribute(PROP_LM_SUBSYSTEM_ID,&m_lmSubsystemID);
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

	m_associatedSignals = signalIDs.split(",", Qt::SkipEmptyParts);

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
	proto->set_lmpresetname(m_lmPresetName.toStdString());
	proto->set_lmcaption(m_lmCaption.toStdString());
	proto->set_lmdatatype(TO_INT(m_lmDataType));
	proto->set_lmip(m_lmAddressPort.addressStr().toStdString());
	proto->set_lmport(m_lmAddressPort.port());
	proto->set_lmsubsystemkey(m_lmSubsystemKey);
	proto->set_lmsubsystemid(m_lmSubsystemID.toStdString());
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
	m_lmPresetName = QString::fromStdString(proto.lmpresetname());
	m_lmCaption = QString::fromStdString(proto.lmcaption());
	m_lmDataType = static_cast<DataType>(proto.lmdatatype());
	m_lmAddressPort.setAddressPort(QString::fromStdString(proto.lmip()), proto.lmport());
	m_lmSubsystemKey = proto.lmsubsystemkey();
	m_lmSubsystemID = QString::fromStdString(proto.lmsubsystemid());
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

bool DataSource::lanControllerFunctions(E::LanControllerType type, bool* tuning, bool* appData, bool* diagData)
{
	if (tuning == nullptr ||
		appData == nullptr ||
		diagData == nullptr)
	{
		assert(false);
		return false;
	}

	bool result = true;

	*tuning = *appData = *diagData = false;

	switch(type)
	{
	case E::LanControllerType::Unknown:
		assert(false);
		result = false;
		break;

	case E::LanControllerType::Tuning:
		*tuning = true;
		break;

	case E::LanControllerType::AppData:
		*appData = true;
		break;

	case E::LanControllerType::DiagData:
		*diagData = true;
		break;

	case E::LanControllerType::AppAndDiagData:
		*appData = *diagData = true;
		break;

	default:
		assert(false);
		result = false;
	}

	return result;
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
	m_rupFrameTimeQueue(10)
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

bool DataSourceOnline::initQueue()
{
	int queueSize = lmRupFramesQuantity() * 200 * 3;	// 3 seconds queue

	m_rupFrameTimeQueue.resize(queueSize);

	setRupFramesQueueSize(queueSize);

	return true;
}

void DataSourceOnline::updateUptime()
{
	if (m_firstPacketSystemTime == 0 || m_lastPacketSystemTime == 0)
	{
		m_uptime = 0;
	}
	else
	{
		m_uptime = (m_lastPacketSystemTime - m_firstPacketSystemTime) / 1000;
	}
}

bool DataSourceOnline::collect(const RupFrameTime& rupFrameTime)
{
	// rupFrameTime.rupFrame.header already reverseByted !
	//
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
		m_frame0ServerTime = rupFrameTime.serverTime;
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

	m_packetNo = numerator0;

	m_receivedPacketCount++;

	const Rup::TimeStamp& timeStamp = m_rupFramesHeaders[0].timeStamp;

	QDateTime plantTime;

	// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
	//
	plantTime.setTimeSpec(Qt::UTC);

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	m_rupDataTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupDataTimes.system.timeStamp = m_frame0ServerTime;

	QDateTime localTime = QDateTime::fromMSecsSinceEpoch(m_frame0ServerTime);

	// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
	//
	localTime.setTimeSpec(Qt::UTC);

	m_rupDataTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

	m_rupDataSize = framesQuantity * sizeof(Rup::Data);

	m_lastRupDataTimes = m_rupDataTimes;

	return true;
}

bool DataSourceOnline::getDataToParsing(Times* times, quint16* packetNo, const char** rupData, int* rupDataSize, bool* dataReceivingTimeout)
{
	if (m_dataReadyToParsing == false)
	{
		assert(false);
		return false;
	}

#ifdef QT_DEBUG

	if (times == nullptr || packetNo == nullptr || rupData == nullptr || rupDataSize == nullptr || dataReceivingTimeout == nullptr)
	{
		assert(false);
		return false;
	}

#endif

	*times = m_rupDataTimes;
	*packetNo = m_packetNo;
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

void DataSourceOnline::calcDataReceivingRate()
{
	if (m_prevCalcTime == -1)
	{
		m_prevCalcTime = QDateTime::currentMSecsSinceEpoch();
		m_prevReceivedSize = m_receivedDataSize;
		m_firstCalc = true;
		return;
	}

	m_calcFramesCtr++;

	if (m_calcFramesCtr < 100)
	{
		return;
	}

	m_calcFramesCtr = 0;

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	qint64 dT = now - m_prevCalcTime;

	if (m_firstCalc == false)
	{
		if (dT < DATA_RECEIVING_RATE_CALC_PERIOD)
		{
			return;
		}
	}

	m_firstCalc = false;

	m_dataReceivingRate = static_cast<double>(m_receivedDataSize - m_prevReceivedSize) / (dT / 1000.0);		// Bytes per second

	m_prevCalcTime = now;
	m_prevReceivedSize = m_receivedDataSize;

	return;
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


void DataSourceOnline::pushRupFrame(qint64 serverTime, const Rup::Frame& rupFrame, const QThread* thread)
{
	RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPush(thread);

	if (rupFrameTime != nullptr)
	{
		rupFrameTime->serverTime = serverTime;
		memcpy(&rupFrameTime->rupFrame, &rupFrame, sizeof(rupFrame));
	}
	else
	{
		// is not an error - queue is full
	}

	m_rupFrameTimeQueue.completePush(thread, &m_rupFramesQueueCurSize, &m_rupFramesQueueCurMaxSize);
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

bool DataSourceOnline::processRupFrameTimeQueue(const QThread* thread)
{
	int count = 0;

	m_dataReadyToParsing = false;

	do
	{
		RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPop(thread);

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

					m_state = E::DataSourceState::NoData;

					m_dataRecevingTimeout = true;
					m_dataReceives = false;
					m_dataReadyToParsing = true;
					m_firstRupFrame = true;

					m_firstPacketSystemTime = 0;
					m_lastPacketSystemTime = 0;

					m_dataReceivingRate = 0;

					m_prevCalcTime = -1;

					updateUptime();
				}
			}

			break;			// has no frames to processing, exit from processRupFrameTimeQueue, return FALSE
							// m_rupFrameTimeQueue.completePop is not required !!!
		}

		m_lastPacketSystemTime = rupFrameTime->serverTime;

		if (m_firstPacketSystemTime == 0)
		{
			m_firstPacketSystemTime = m_lastPacketSystemTime;
		}

		updateUptime();

		if (m_state == E::DataSourceState::NoData)
		{
			m_receivedDataSize = 0;
			m_receivedFramesCount = 0;
			m_receivedPacketCount = 0;
			m_lostPacketCount = 0;
			m_processedPacketCount = 0;

			m_errorProtocolVersion = 0;
			m_errorFramesQuantity = 0;
			m_errorFrameNo = 0;
			m_errorDataID = 0;
			m_errorFrameSize = 0;
			m_errorDuplicatePlantTime = 0;
			m_errorNonmonotonicPlantTime = 0;
		}

		m_state = E::DataSourceState::ReceiveData;

		m_dataRecevingTimeout = false;
		m_dataReceives = true;
		m_receivedDataSize += sizeof(Rup::Frame);

		calcDataReceivingRate();

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
			m_receivedDataID = rupFrameHeader.dataId;

			if (m_receivedDataID != lmDataID())
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
				m_rupFramePlantTime = m_rupDataTimes.plant.timeStamp;
				m_processedPacketCount++;

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
							m_lostPacketCount += numerator - m_rupFrameNumerator;
						}
						else
						{
							m_lostPacketCount += 0xFFFF - m_rupFrameNumerator + numerator;
						}

						m_rupFrameNumerator = numerator;
					}
				}

				m_rupFrameNumerator++;
			}

			break;
		}
		while(1);

		m_rupFrameTimeQueue.completePop(thread);

		count++;
	}
	while(m_dataReadyToParsing == false && count < 100);

	return m_dataReadyToParsing;
}


