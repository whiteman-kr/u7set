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


DataSource::DataSource(const DataSource& ds)
{
	this->operator =(ds);
}


DataSource& DataSource::operator = (const DataSource& ds)
{
	m_id = ds.ID();
	m_hostAddress = ds.hostAddress();
	m_name = ds.name();
	m_partCount = ds.partCount();

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
	dsi.copyStringToBuffer(m_name, dsi.name, DATA_SOURCE_NAME_LEN);
}


void DataSource::setInfo(const DataSourceInfo& dsi)
{
/*	m_id = dsi.ID;
	m_hostAddress = dsi.ip;
	m_port = dsi.port;
	m_partCount = dsi.partCount;
	copyBufferToString(dsi.name, DATA_SOURCE_NAME_LEN, m_name);*/
}

