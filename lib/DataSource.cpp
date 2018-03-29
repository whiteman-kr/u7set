#include "../lib/DataSource.h"
#include "../lib/WUtils.h"
#include "../lib/Crc.h"

// -----------------------------------------------------------------------------
//
// DataSource class implementation
//
// -----------------------------------------------------------------------------

const char* const DataSource::ELEMENT_DATA_SOURCE = "DataSource";
const char* const DataSource::ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS = "AssociatedSignals";

const char* DataSource::DATA_TYPE_APP = "App";
const char* DataSource::DATA_TYPE_DIAG = "Diag";
const char* DataSource::DATA_TYPE_TUNING = "Tuning";

const char* DataSource::PROP_DATA_TYPE = "LmDataType";
const char* DataSource::PROP_LM_ID = "LmEquipmentID";
const char* DataSource::PROP_LM_NUMBER = "LmNumber";
const char* DataSource::PROP_LM_CHANNEL = "LmChannel";
const char* DataSource::PROP_LM_SUBSYSTEM_ID = "LmSubsystemID";
const char* DataSource::PROP_LM_SUBSYSTEM = "LmSubsystem";
const char* DataSource::PROP_LM_MODULE_TYPE = "LmModuleType";
const char* DataSource::PROP_LM_CAPTION = "LmCaption";
const char* DataSource::PROP_LM_ADAPTER_ID = "LmAdapterID";
const char* DataSource::PROP_LM_DATA_ENABLE = "LmDataEnable";
const char* DataSource::PROP_LM_DATA_IP = "LmDataIP";
const char* DataSource::PROP_LM_DATA_PORT = "LmDataPort";
const char* DataSource::PROP_LM_RUP_FRAMES_QUANTITY = "LmRupFramesQuantity";
const char* DataSource::PROP_LM_DATA_ID = "LmDataID";
const char* DataSource::PROP_LM_UNIQUE_ID = "LmUniqueID";
const char* DataSource::PROP_COUNT = "Count";
const char* DataSource::SIGNAL_ID_ELEMENT = "SignalID";


DataSource::DataSource()
{
}

DataSource::~DataSource()
{
}

QString DataSource::dataTypeToString(DataType dataType) const
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

	xml.writeStringAttribute(PROP_DATA_TYPE, dataTypeToString(m_lmDataType));
	xml.writeStringAttribute(PROP_LM_ID, m_lmEquipmentID);

	xml.writeIntAttribute(PROP_LM_MODULE_TYPE, m_lmModuleType);
	xml.writeStringAttribute(PROP_LM_SUBSYSTEM, m_lmSubsystem);
	xml.writeIntAttribute(PROP_LM_SUBSYSTEM_ID, m_lmSubsystemID);
	xml.writeIntAttribute(PROP_LM_NUMBER, m_lmNumber);
	xml.writeStringAttribute(PROP_LM_CHANNEL, m_lmSubsystemChannel);

	xml.writeStringAttribute(PROP_LM_CAPTION, m_lmCaption);
	xml.writeStringAttribute(PROP_LM_ADAPTER_ID, m_lmAdapterID);
	xml.writeBoolAttribute(PROP_LM_DATA_ENABLE, m_lmDataEnable);
	xml.writeStringAttribute(PROP_LM_DATA_IP, m_lmAddressPort.addressStr());
	xml.writeIntAttribute(PROP_LM_DATA_PORT, m_lmAddressPort.port());
	xml.writeIntAttribute(PROP_LM_RUP_FRAMES_QUANTITY, m_lmRupFramesQuantity);

	xml.writeUInt32Attribute(PROP_LM_DATA_ID, m_lmDataID, true);
	xml.writeUInt64Attribute(PROP_LM_UNIQUE_ID, m_lmUniqueID, true);

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

	result &= xml.readStringAttribute(PROP_DATA_TYPE, &str);
	m_lmDataType = stringToDataType(str);

	result &= xml.readStringAttribute(PROP_LM_ID, &m_lmEquipmentID);

	result &= xml.readIntAttribute(PROP_LM_MODULE_TYPE, &m_lmModuleType);
	result &= xml.readStringAttribute(PROP_LM_SUBSYSTEM,&m_lmSubsystem);
	result &= xml.readIntAttribute(PROP_LM_SUBSYSTEM_ID, &m_lmSubsystemID);
	result &= xml.readIntAttribute(PROP_LM_NUMBER, &m_lmNumber);
	result &= xml.readStringAttribute(PROP_LM_CHANNEL,&m_lmSubsystemChannel);

	result &= xml.readStringAttribute(PROP_LM_CAPTION,&m_lmCaption);
	result &= xml.readStringAttribute(PROP_LM_ADAPTER_ID, &m_lmAdapterID);
	result &= xml.readBoolAttribute(PROP_LM_DATA_ENABLE, &m_lmDataEnable);

	QString ipStr;
	int port = 0;

	result &= xml.readStringAttribute(PROP_LM_DATA_IP, &ipStr);
	result &= xml.readIntAttribute(PROP_LM_DATA_PORT, &port);

	m_lmAddressPort.setAddress(ipStr);
	m_lmAddressPort.setPort(port);

	result &= xml.readIntAttribute(PROP_LM_RUP_FRAMES_QUANTITY, &m_lmRupFramesQuantity);

	result &= xml.readUInt32Attribute(PROP_LM_DATA_ID, &m_lmDataID);
	result &= xml.readUInt64Attribute(PROP_LM_UNIQUE_ID, &m_lmUniqueID);

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
	proto->set_lmsubsystemid(m_lmSubsystemID);
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
	m_lmSubsystemID = proto.lmsubsystemid();
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
// DataSourcesXML class implementation
//
// -----------------------------------------------------------------------------


bool DataSourcesXML::writeToXml(const QVector<DataSource>& dataSources, QByteArray* fileData)
{
	TEST_PTR_RETURN_FALSE(fileData);

	fileData->clear();

	QXmlStreamWriter xmlWriter(fileData);
	XmlWriteHelper xml(xmlWriter);

	xml.setAutoFormatting(true);
	xml.writeStartDocument();

	xml.writeStartElement("DataSources");
	xml.writeIntAttribute("Count", dataSources.count());

	for(const DataSource& ds : dataSources)
	{
		ds.writeToXml(xml);
	}

	xml.writeEndElement();	// </AppDataSources>
	xml.writeEndDocument();

	return true;
}

bool DataSourcesXML::readFromXml(const QByteArray& fileData, QVector<DataSource>* dataSources)
{
	TEST_PTR_RETURN_FALSE(dataSources);

	XmlReadHelper xml(fileData);

	dataSources->clear();

	bool result = true;

	if (xml.findElement("DataSources") == false)
	{
		return false;
	}

	int count = 0;

	if (xml.readIntAttribute("Count", &count) == false)
	{
		return false;
	}

	dataSources->resize(count);

	for(int i = 0; i < count; i++)
	{
		result &= (*dataSources)[i].readFromXml(xml);
	}

	return result;
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
		m_dataReadyToParsing = false;
		return false;
	}

	const Rup::TimeStamp& timeStamp = m_rupFramesHeaders[0].timeStamp;

	QDateTime plantTime;

	plantTime.setTimeSpec(Qt::UTC);	// don't delete this to prevent plantTime conversion from Local to UTC time!!!

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	m_rupDataTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupDataTimes.system.timeStamp = m_firstFrameServerTime;
	m_rupDataTimes.local.timeStamp = QDateTime::fromMSecsSinceEpoch(m_firstFrameServerTime).toMSecsSinceEpoch();

	m_rupDataSize = framesQuantity * sizeof(Rup::Data);

	m_dataReadyToParsing = true;

	return true;
}

bool DataSourceOnline::getDataToParsing(Times* times, const char** rupData, quint32* rupDataSize)
{
	if (m_dataReadyToParsing == false)
	{
		assert(false);
		return false;
	}

	if (times == nullptr || rupData == nullptr || rupDataSize == nullptr)
	{
		assert(false);
		return false;
	}

	*times = m_rupDataTimes;
	*rupData = reinterpret_cast<const char*>(m_rupFramesData);
	*rupDataSize = m_rupDataSize;

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


void DataSourceOnline::stop()
{
	setState(E::DataSourceState::Stopped);
	m_dataReceivingRate = 0;
	m_receivedDataSize = 0;
}


void DataSourceOnline::resume()
{
	setState(E::DataSourceState::NoData);
}


void DataSourceOnline::pushRupFrame(qint64 serverTime, const Rup::Frame& rupFrame)
{
	assert(m_rupFrameTimeQueue.isFull() == false);

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
}

bool DataSourceOnline::seizeProcessingOwnership(const QThread* processingThread)
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
	bool dataReady = false;

	int count = 0;

	do
	{
		RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPop();

		if (rupFrameTime == nullptr)
		{
			break;	// has no frames to processing, exit from processRupFrameTimeQueue, return FALSE
					//
					// m_rupFrameTimeQueue.completePop is not required
		}

		m_lastPacketSystemTime = QDateTime::currentMSecsSinceEpoch();
		m_state = E::DataSourceState::ReceiveData;

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
					QString msg = QString("Wrong DataID from %1 (%2, waiting %3), packet processing skiped").
							arg(lmAddressPort().addressStr()).
							arg(rupFrameHeader.dataId).
							arg(lmDataID());

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
			dataReady = collect(*rupFrameTime);

			if (dataReady == true)
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
	while(dataReady == false && count < 100);

	return dataReady;
}


