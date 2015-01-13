#pragma once

#include <QObject>
#include "../include/SocketIO.h"


enum DataSourceState
{
	noData = 0,
	receiveData = 1,
	stopped = 2
};


// RQID_GET_DATA_SOURCES_INFO request data format
//
const int DATA_SOURCE_NAME_LEN = 32;

struct DataSourceInfo : public Serializable
{
	quint32 ID;
	quint16 name[DATA_SOURCE_NAME_LEN];
	quint32 ip;
	quint32 partCount;

	DataSourceInfo() : Serializable(1) {}

protected:
	virtual char *serialize(char *buffer, bool write) override;
};


// RQID_GET_DATA_SOURCES_STATISTICS request data format
//
struct DataSourceStatistics : public Serializable
{
	quint32 ID;
	quint32 state;
	quint64 uptime;
	quint64 receivedDataSize;
	double dataReceivingRate;

	DataSourceStatistics() : Serializable(1) {}

protected:
	virtual char *serialize(char *buffer, bool write) override;
};


class DataSource : public QObject
{
private:
	// static information
	//
	quint32 m_id = 0;
	QHostAddress m_hostAddress;
	QString m_name;
	quint32 m_partCount = 1;

	// dynamic information
	//
	DataSourceState m_state = DataSourceState::noData;
	quint64 m_uptime = 0;
	quint64 m_receivedDataSize = 0;
	double m_dataReceivingRate = 0;

public:
	DataSource(quint32 id, QString name, QHostAddress hostAddress, quint32 partCount);

	DataSource() {}
	DataSource(const DataSource& ds);
	DataSource& operator = (const DataSource& ds);

	quint32 ID() const { return m_id; }
	QHostAddress hostAddress() const { return m_hostAddress; }
	quint32 partCount() const { return m_partCount; }
	QString name() const { return m_name; }

	DataSourceState state() const { return m_state; }
	quint64 uptime() const { return m_uptime; }
	quint64 receivedDataSize() const { return m_receivedDataSize; }
	double dataReceivingRate() const { return m_dataReceivingRate; }

	void setID(quint32 id) { m_id = id; }
	void setHostAddress(QHostAddress hostAddress) { m_hostAddress = hostAddress; }
	void partCount(quint32 partCount) { m_partCount = partCount; }

	void setState(DataSourceState state) { m_state = state; }

	void getInfo(DataSourceInfo& dsi);
	void setInfo(const DataSourceInfo& dsi);

	void getStatistics(DataSourceStatistics& dss);
	void setStatistics(const DataSourceStatistics& dss);

	void stop();
	void resume();
};



