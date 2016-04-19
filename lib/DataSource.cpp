#include "../include/DataSource.h"


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


DataSource::DataSource(quint32 id, QString name, QHostAddress hostAddress, quint32 partCount) :
	m_id(id),
	m_hostAddress(hostAddress),
	m_name(name),
	m_partCount(partCount)
{
}


DataSource::DataSource(const DataSource& ds) :
	QObject()
{
	this->operator =(ds);
}


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
}


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
		return "App";

	case DataType::Diag:
		return "Diag";

	default:
		assert(false);
	}

	return "";
}


DataSource::DataType DataSource::stringToDataType(const QString& dataTypeStr)
{
	if (dataTypeStr == "App")
	{
		return DataType::App;
	}

	if (dataTypeStr == "Diag")
	{
		return DataType::Diag;
	}

	assert(false);
	return DataType::App;
}


void DataSource::serializeToXml(QXmlStreamWriter& xml)
{
	xml.writeStartElement("DataSource");

	xml.writeAttribute("EthernetChannel", QString::number(m_ethernetChannel));
	xml.writeAttribute("Type", dataTypeToString(m_dataType));
	xml.writeAttribute("LmStrID", m_lmStrID);
	xml.writeAttribute("LmCaption", m_lmCaption);
	xml.writeAttribute("LmAdapterStrID", m_lmAdapterStrID);
	xml.writeAttribute("LmDataEnable", m_lmDataEnable ? "true" : "false");
	xml.writeAttribute("LmDataIP", m_lmAddressPort.addressStr());
	xml.writeAttribute("LmDataPort", QString::number(m_lmAddressPort.port()));
	xml.writeAttribute("LmDataID", QString::number(m_lmDataID));

	xml.writeEndElement();	// </DataSource>
}


void DataSource::serializeFromXml(QXmlStreamWriter& xml)
{
	Q_UNUSED(xml);
	/*xml.writeStartElement("DataSource");

	xml.writeAttribute("EthernetChannel", QString::number(m_ethernetChannel));
	xml.writeAttribute("Type", dataTypeToString(m_dataType));
	xml.writeAttribute("LmStrID", m_lmStrID);
	xml.writeAttribute("LmCaption", m_lmCaption());
	xml.writeAttribute("LmAdapterStrID", m_lmAdapterStrID);
	xml.writeAttribute("LmDataEnable", m_lmDataEnable ? "true" : "false");
	xml.writeAttribute("LmDataIP", m_lmAddressPort.addressStr());
	xml.writeAttribute("LmDataPort", QString::number(m_lmAddressPort.port()));

	xml.writeEndElement();	// </DataSource>*/
}

