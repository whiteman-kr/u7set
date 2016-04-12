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
public:
	enum class DataType
	{
		App,
		Diag,
	};

private:

	// static information
	//
	quint32 m_id = 0;
	QHostAddress m_hostAddress;
	QString m_name;
	quint32 m_partCount = 1;
	QVector<int> m_relatedSignalIndexes;


	// dynamic information
	//
	DataSourceState m_state = DataSourceState::noData;
	quint64 m_uptime = 0;
	quint64 m_receivedDataSize = 0;
	double m_dataReceivingRate = 0;

	// XML-serializable members
	//
	int m_ethernetChannel = 0;
	DataType m_dataType = DataType::App;
	QString m_lmStrID;
	QString m_lmCaption;
	QString m_lmAdapterStrID;
	bool m_lmDataEnable = false;
	HostAddressPort m_lmAddressPort;
	quint32 m_lmDataID = 0;

public:
	DataSource(quint32 id, QString name, QHostAddress hostAddress, quint32 partCount);

	DataSource() {}
	DataSource(const DataSource& ds);
	DataSource& operator = (const DataSource& ds);

	int ethernetChannel() const { return m_ethernetChannel; }
	void setEthernetChannel(int channel) { m_ethernetChannel = channel; }

	DataType dataType() const { return m_dataType; }
	void setDataType(DataType dataType) { m_dataType = dataType; }

	QString lmStrID() const { return m_lmStrID; }
	void setLmStrID(const QString& lmStrID) { m_lmStrID = lmStrID; }

	QString lmCaption() const { return m_lmCaption; }
	void setLmCaption(const QString& lmCaption) { m_lmCaption = lmCaption; }

	QString lmAdapterStrID() const { return m_lmAdapterStrID; }
	void setLmAdapterStrID(const QString& lmAdapterStrID) { m_lmAdapterStrID = lmAdapterStrID; }

	bool lmDataEnable() const { return m_lmDataEnable; }
	void setLmDataEnable(bool lmDataEnable) { m_lmDataEnable = lmDataEnable; }

	QString lmAddressStr() const { return m_lmAddressPort.addressStr(); }
	void setLmAddressStr(const QString& addressStr) { m_lmAddressPort.setAddress(addressStr); }

	QHostAddress lmAddress() const { return m_lmAddressPort.address(); }

	int lmPort() const { return m_lmAddressPort.port(); }
	void setLmPort(int port) { m_lmAddressPort.setPort(port); }

	quint32 lmDataID() const { return m_lmDataID; }
	void setLmDataID(quint32 lmDataID) { m_lmDataID = lmDataID; }



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

	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

	void getInfo(DataSourceInfo& dsi);
	void setInfo(const DataSourceInfo& dsi);

	void getStatistics(DataSourceStatistics& dss);
	void setStatistics(const DataSourceStatistics& dss);

	void stop();
	void resume();

	static QString dataTypeToString(DataType dataType);
	static DataType stringToDataType(const QString& dataTypeStr);

	void serializeToXml(QXmlStreamWriter& xml);
	void serializeFromXml(QXmlStreamWriter& xml);
};



