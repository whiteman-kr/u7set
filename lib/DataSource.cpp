#include "../lib/DataSource.h"
#include "../lib/WUtils.h"
#include "../lib/Crc.h"


void RupData::dump()
{
	QString s;

	for(quint16 i = 0; i < dataSize; i++)
	{
		QString v;

		if ((i % 16) == 0)
		{
			v.sprintf("%04X  ", static_cast<unsigned int>(i));
			s += v;
		}

		v.sprintf("%02X ", static_cast<unsigned int>(data[i]));

		s += v;

		if (i > 0 && (i % 7) == 0)
		{
			s += " ";
		}

		if (i > 0 && (i % 15) == 0)
		{
			qDebug() << s;

			s.clear();
		}
	}
}

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


DataSource::DataSource() :
	m_rupFramesQueue(Rup::MAX_FRAME_COUNT * 150)
{
}


DataSource::~DataSource()
{
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


void DataSource::stop()
{
	setState(E::DataSourceState::Stopped);
	m_dataReceivingRate = 0;
	m_receivedDataSize = 0;
}


void DataSource::resume()
{
	setState(E::DataSourceState::NoData);
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

	return "";
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


void DataSource::writeToXml(XmlWriteHelper& xml)
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

	QString signalsIDs;

	for(const QString& appSignalID : m_associatedSignals)
	{
		signalsIDs += QString("%1,").arg(appSignalID);
	}

	if (signalsIDs.length() > 0)
	{
		signalsIDs = signalsIDs.mid(0, signalsIDs.length() - 1);		// remove last comma
	}

	xml.writeString(signalsIDs);

	xml.writeEndElement();	// </AssociatedSignals>

	writeAdditionalSectionsToXml(xml);

	xml.writeEndElement();	// </AppDataSource>
}


bool DataSource::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	if (xml.name() != ELEMENT_DATA_SOURCE)
	{
		assert(false);
		return false;
	}

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


void DataSource::writeAdditionalSectionsToXml(XmlWriteHelper&)
{
}


bool DataSource::readAdditionalSectionsFromXml(XmlReadHelper&)
{
	return true;
}


void DataSource::processPacket(quint32 ip, Rup::Frame& rupFrame, Queue<RupData>& rupDataQueue)
{
	m_dataReceived = true;
	m_receivedFramesCount++;
	m_receivedDataSize += sizeof(rupFrame);

	if (rupFrame.header.protocolVersion != reverseUint16(5))
	{
		m_errorProtocolVersion++;

		return;
	}

	if (m_dataProcessingEnabled == false)
	{
		return;
	}

	rupFrame.header.reverseBytes();

	if (rupFrame.header.dataId != m_lmDataID)
	{
		m_errorDataID++;

		if (m_errorDataID > 0 && (m_errorDataID % 500) == 0)
		{
			QHostAddress host(ip);

			QString msg = QString("Wrong DataID from %1 (%2, waiting %3), packet processing skiped").
					arg(host.toString()).
					arg(rupFrame.header.dataId).
					arg(m_lmDataID);

			qDebug() << C_STR(msg)  << m_errorDataID;
		}
		return;
	}

	int framesQuantity = rupFrame.header.framesQuantity;

	if (framesQuantity > Rup::MAX_FRAME_COUNT)
	{
		assert(false);
		m_errorFramesQuantity++;
		return;
	}

	int frameNo = rupFrame.header.frameNumber;

	if (frameNo >= framesQuantity)
	{
		assert(false);
		m_errorFrameNo++;
		return ;
	}

	memcpy(m_rupFrames + frameNo, &rupFrame, sizeof(Rup::Frame));

	// check packet
	//
	bool dataReady = true;

	if (framesQuantity > 1)
	{
		quint16 numerator0 = m_rupFrames[0].header.numerator;

		for(int i = 1; i < framesQuantity; i++)
		{
			dataReady &= m_rupFrames[i].header.numerator == numerator0;
		}
	}

	if (dataReady == true)
	{
		m_receivedPacketCount++;

		m_lastPacketTime = QDateTime::currentMSecsSinceEpoch();
		m_state = E::DataSourceState::ReceiveData;

		int framesQuantity = m_rupFrames[0].header.framesQuantity;		// we have at least one m_rupFrame

		QDateTime plantTime;
		Rup::TimeStamp timeStamp = m_rupFrames[0].header.timeStamp;

		plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
		plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

		QDateTime currentTime = QDateTime::currentDateTimeUtc();

		// get rupData pointer and lock rupDataQueue
		//
		RupData* rupData = rupDataQueue.beginPush();

		// fill rupData header
		//
		rupData->sourceIP = ip;

		rupData->time.plant.timeStamp = plantTime.toMSecsSinceEpoch();
		rupData->time.system.timeStamp = currentTime.toMSecsSinceEpoch();
		rupData->time.local.timeStamp = currentTime.toLocalTime().toMSecsSinceEpoch();

		rupData->dataSize = framesQuantity * Rup::FRAME_DATA_SIZE;

		// merge frames data into rupDataQueue's buffer
		//
		for(int i = 0; i < framesQuantity; i++)
		{
			memcpy(rupData->data + i * Rup::FRAME_DATA_SIZE, m_rupFrames[i].data, Rup::FRAME_DATA_SIZE);
		}

		// push rupData and unlock rupDataQueue
		//
		rupDataQueue.completePush();
	}
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

	assert(m_id == generateID());

	return true;
}


void DataSource::pushRupFrame(const Rup::Frame& rupFrame)
{
	m_receivedFramesCount++;

	m_receivedDataSize += sizeof(Rup::Frame);

	quint32 headerNumerator = (quint32)reverseUint16(rupFrame.header.numerator);

	if (m_firstRupFrame == true)
	{
		m_rupFrameNumerator = headerNumerator;

		m_rupFrameNumerator++;

		m_firstRupFrame = false;
	}
	else
	{
		if (m_rupFrameNumerator != headerNumerator)
		{
			if (m_rupFrameNumerator < headerNumerator)
			{
				m_lostedFramesCount += headerNumerator - m_rupFrameNumerator;
			}
			else
			{
				m_lostedFramesCount += 65535 - m_rupFrameNumerator + headerNumerator;
			}

			m_rupFrameNumerator = headerNumerator;
		}

		m_rupFrameNumerator++;
	}

	m_rupFramesQueue.push(&rupFrame);
}


