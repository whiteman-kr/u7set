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
const char* DataSource::PROP_CHANNEL = "LmChannel";
const char* DataSource::PROP_LM_ID = "LmEquipmentID";
const char* DataSource::PROP_LM_NUMBER = "LmNumber";
const char* DataSource::PROP_LM_SUBSYSTEM_ID = "LmSubsystemID";
const char* DataSource::PROP_LM_SUBSYSTEM = "LmSubsystem";
const char* DataSource::PROP_LM_MODULE_TYPE = "LmModuleType";
const char* DataSource::PROP_LM_CAPTION = "LmCaption";
const char* DataSource::PROP_LM_ADAPTER_ID = "LmAdapterID";
const char* DataSource::PROP_LM_DATA_ENABLE = "LmDataEnable";
const char* DataSource::PROP_LM_DATA_IP = "LmDataIP";
const char* DataSource::PROP_LM_DATA_PORT = "LmDataPort";
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

	xml.writeStringAttribute(PROP_LM_CAPTION, m_lmCaption);
	xml.writeStringAttribute(PROP_LM_ADAPTER_ID, m_lmAdapterID);
	xml.writeBoolAttribute(PROP_LM_DATA_ENABLE, m_lmDataEnable);
	xml.writeStringAttribute(PROP_LM_DATA_IP, m_lmAddressPort.addressStr());
	xml.writeIntAttribute(PROP_LM_DATA_PORT, m_lmAddressPort.port());
	xml.writeUInt32Attribute(PROP_LM_DATA_ID, m_lmDataID, false);
	xml.writeUInt64Attribute(PROP_LM_UNIQUE_ID, m_uniqueID, true);

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

	result &= xml.readStringAttribute(PROP_LM_CAPTION,&m_lmCaption);
	result &= xml.readStringAttribute(PROP_LM_ADAPTER_ID, &m_lmAdapterID);
	result &= xml.readBoolAttribute(PROP_LM_DATA_ENABLE, &m_lmDataEnable);

	QString ipStr;
	int port = 0;

	result &= xml.readStringAttribute(PROP_LM_DATA_IP, &ipStr);
	result &= xml.readIntAttribute(PROP_LM_DATA_PORT, &port);

	m_lmAddressPort.setAddress(ipStr);
	m_lmAddressPort.setPort(port);

	result &= xml.readUInt32Attribute(PROP_LM_DATA_ID, &m_lmDataID);
	result &= xml.readUInt64Attribute(PROP_LM_UNIQUE_ID, &m_uniqueID);

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

bool DataSource::getInfo(Network::DataSourceInfo* protoInfo) const
{
	if (protoInfo == nullptr)
	{
		assert(false);
		return false;
	}

	protoInfo->set_id(m_id);
	protoInfo->set_equipmentid(m_lmEquipmentID.toStdString());
	protoInfo->set_caption(m_lmCaption.toStdString());
	protoInfo->set_datatype(TO_INT(m_lmDataType));
	protoInfo->set_ip(m_lmAddressPort.addressStr().toStdString());
	protoInfo->set_port(m_lmAddressPort.port());
	protoInfo->set_subsystemid(m_lmSubsystemID);
	protoInfo->set_subsystem(m_lmSubsystem.toStdString());
	protoInfo->set_lmnumber(m_lmNumber);
	protoInfo->set_lmmoduletype(m_lmModuleType);
	protoInfo->set_lmadapterid(m_lmAdapterID.toStdString());
	protoInfo->set_lmdataenable(m_lmDataEnable);
	protoInfo->set_lmdataid(m_lmDataID);

	return true;
}


bool DataSource::setInfo(const Network::DataSourceInfo& protoInfo)
{
	m_id = protoInfo.id();
	m_lmEquipmentID = QString::fromStdString(protoInfo.equipmentid());
	m_lmCaption = QString::fromStdString(protoInfo.caption());
	m_lmDataType = static_cast<DataType>(protoInfo.datatype());
	m_lmAddressPort.setAddress(QString::fromStdString(protoInfo.ip()));
	m_lmAddressPort.setPort(protoInfo.port());
	m_lmSubsystemID = protoInfo.subsystemid();
	m_lmSubsystem = QString::fromStdString(protoInfo.subsystem());
	m_lmNumber = protoInfo.lmnumber();
	m_lmModuleType = protoInfo.lmmoduletype();
	m_lmAdapterID = QString::fromStdString(protoInfo.lmadapterid());
	m_lmDataEnable = protoInfo.lmdataenable();
	m_lmDataID = protoInfo.lmdataid();

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
	m_rupFrameTimeQueue(1)
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
	m_rupFrameTimeQueue.resize(m_partCount * 400);			// 2 seconds queue

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

bool DataSourceOnline::seizeProcessingOwnership(const SimpleThreadWorker* processingWorker)
{
	const SimpleThreadWorker* expected = nullptr;

	bool result = m_processingOwner.compare_exchange_strong(expected,  processingWorker);

	// if ownership has been taken by processingWorker - function returns TRUE
	//
	// result == FALSE is Ok, means that another thread is already take ownership

	return result;
}

bool DataSourceOnline::releaseProcessingOwnership(const SimpleThreadWorker* processingWorker)
{
	bool result = m_processingOwner.compare_exchange_strong(processingWorker,  nullptr);

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

		if (rupFrameTime != nullptr)
		{
			m_lastPacketTime = QDateTime::currentMSecsSinceEpoch();
			m_state = E::DataSourceState::ReceiveData;

			do
			{
				m_dataReceived = true;
				m_receivedFramesCount++;
				m_receivedDataSize += sizeof(Rup::Frame);

				if (m_dataProcessingEnabled == false)
				{
					break;
				}

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
								m_lostedFramesCount += numerator - m_rupFrameNumerator;
							}
							else
							{
								m_lostedFramesCount += 0xFFFF - m_rupFrameNumerator + numerator;
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
		}
		else
		{
			break;	// has no frames to processing, exit from processRupFrameTimeQueue, return FALSE
		}

		count++;
	}
	while(dataReady == false && count < 100);

	return dataReady;
}

