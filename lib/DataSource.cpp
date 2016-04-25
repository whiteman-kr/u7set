#include "../include/DataSource.h"


const char* const DataSource::ELEMENT_DATA_SOURCE = "DataSource";


char* DataSourceInfo::serialize(char* buffer, bool write)
{
	BEGIN_SERIALIZATION();

	SERIALIZE_VAR(quint32, ID);
	SERIALIZE_ARRAY(quint16, name, DATA_SOURCE_NAME_LEN);
	SERIALIZE_VAR(quint32, ip);
	SERIALIZE_VAR(quint32, partCount);

	END_SERIALIZATION();
}


char* DataSourceStatistics::serialize(char* buffer, bool write)
{
	BEGIN_SERIALIZATION();

	SERIALIZE_VAR(quint32, ID);
	SERIALIZE_VAR(quint32, state);
	SERIALIZE_VAR(quint64, uptime);
	SERIALIZE_VAR(quint64, receivedDataSize);
	SERIALIZE_VAR(double, dataReceivingRate);

	END_SERIALIZATION();
}


DataSource::DataSource()
{
	m_rupFrames = new RupFrame[RUP_MAX_FRAME_COUNT];
	m_framesData = new char[RUP_BUFFER_SIZE];
}

DataSource::~DataSource()
{

}

/*
DataSource::DataSource(quint32 id, QString name, QHostAddress hostAddress, quint32 partCount) :
	m_id(id),
	m_hostAddress(hostAddress),
	m_name(name),
	m_partCount(partCount)
{

}*/

/*
DataSource::DataSource(const DataSource& ds) :
	QObject()
{
	this->operator =(ds);
}*/

/*
DataSource& DataSource::operator = (const DataSource& ds)
{
	m_id = ds.ID();
	m_hostAddress = ds.hostAddress();
	m_name = ds.name();
	m_partCount = ds.partCount();
	m_relatedSignalIndexes = ds.m_relatedSignalIndexes;

	m_state = ds.state();
	m_uptime = ds.uptime();
	m_receivedDataSize = ds.receivedDataSize();
	m_dataReceivingRate = ds.dataReceivingRate();

	return *this;
}*/


void DataSource::stop()
{
	setState(DataSourceState::stopped);
	m_dataReceivingRate = 0;
	m_receivedDataSize = 0;
}


void DataSource::resume()
{
	setState(DataSourceState::noData);
}


void DataSource::getInfo(DataSourceInfo& dsi)
{
	dsi.ID = m_id;
	dsi.ip = m_hostAddress.toIPv4Address();
	dsi.partCount = m_partCount;
	Serializable::copyStringToBuffer(m_name, dsi.name, DATA_SOURCE_NAME_LEN);
}


void DataSource::setInfo(const DataSourceInfo& dsi)
{
	m_id = dsi.ID;
	m_hostAddress = QHostAddress(dsi.ip);
	m_partCount = dsi.partCount;
	Serializable::copyBufferToString(dsi.name, m_name);
}


void DataSource::getStatistics(DataSourceStatistics& dss)
{
	dss.ID = m_id;
	dss.state = static_cast<quint32>(m_state);
	dss.uptime = m_uptime;
	dss.receivedDataSize = m_receivedDataSize;
	dss.dataReceivingRate = m_dataReceivingRate;
}


void DataSource::setStatistics(const DataSourceStatistics& dss)
{
	Q_ASSERT(dss.ID == m_id);

	m_state = static_cast<DataSourceState>(dss.state);
	m_uptime = dss.uptime;
	m_receivedDataSize = dss.receivedDataSize;
	m_dataReceivingRate = dss.dataReceivingRate;
}


QString DataSource::dataTypeToString(DataType dataType)
{
	switch(dataType)
	{
	case DataType::App:
		return DATA_TYPE_APP;

	case DataType::Diag:
		return DATA_TYPE_DIAG;

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

	assert(false);

	return DataType::Diag;
}


void DataSource::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(ELEMENT_DATA_SOURCE);

	xml.writeIntAttribute(PROP_CHANNEL, m_channel);
	xml.writeStringAttribute(PROP_DATA_TYPE, dataTypeToString(m_dataType));
	xml.writeStringAttribute(PROP_LM_ID, m_lmStrID);
	xml.writeStringAttribute(PROP_LM_CAPTION, m_lmCaption);
	xml.writeStringAttribute(PROP_LM_ADAPTER_ID, m_lmAdapterStrID);
	xml.writeBoolAttribute(PROP_LM_DATA_ENABLE, m_lmDataEnable);
	xml.writeStringAttribute(PROP_LM_DATA_IP, m_lmAddressPort.addressStr());
	xml.writeIntAttribute(PROP_LM_DATA_PORT, m_lmAddressPort.port());
	xml.writeUlongAttribute(PROP_LM_DATA_ID, m_lmDataID, true);

	xml.writeEndElement();	// </DataSource>
}


bool DataSource::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	if (xml.name() != ELEMENT_DATA_SOURCE)
	{
		assert(false);
		return false;
	}

	result &= xml.readIntAttribute(PROP_CHANNEL, &m_channel);

	QString str;

	result &= xml.readStringAttribute(PROP_DATA_TYPE, &str);

	m_dataType = stringToDataType(str);

	result &= xml.readStringAttribute(PROP_LM_ID, &m_lmStrID);
	result &= xml.readStringAttribute(PROP_LM_CAPTION,&m_lmCaption);
	result &= xml.readStringAttribute(PROP_LM_ADAPTER_ID, &m_lmAdapterStrID);
	result &= xml.readBoolAttribute(PROP_LM_DATA_ENABLE, &m_lmDataEnable);

	QString ipStr;
	int port = 0;

	result &= xml.readStringAttribute(PROP_LM_DATA_IP, &ipStr);
	result &= xml.readIntAttribute(PROP_LM_DATA_PORT, &port);

	m_lmAddressPort.setAddress(ipStr);
	m_lmAddressPort.setPort(port);

	result &= xml.readUlongAttribute(PROP_LM_DATA_ID, &m_lmDataID);

	return result;
}


void DataSource::processPacket(quint32 ip, const RupFrame& rupFrame)
{
	m_framesQuantity = rupFrame.header.framesQuantity;

	if (m_framesQuantity > RUP_MAX_FRAME_COUNT)
	{
		assert(false);
		return;
	}

	int frameNo = rupFrame.header.frameNumber;

	if (frameNo >= RUP_MAX_FRAME_COUNT)
	{
		assert(false);
		return ;
	}

	memcpy(m_rupFrames + frameNo, &rupFrame, sizeof(RupFrame));

	// check packet
	//
	bool dataReady = true;

	if (m_framesQuantity > 1)
	{
		quint16 numerator0 = m_rupFrames[0].header.numerator;

		for(int i = 1; i < m_framesQuantity; i++)
		{
			dataReady &= m_rupFrames[i].header.numerator == numerator0;
		}
	}

	if (dataReady == true)
	{
		mergeFrames();
	}
}


void DataSource::allocateMemory()
{
}

void DataSource::mergeFrames()
{
}
