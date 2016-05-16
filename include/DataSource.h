#pragma once

#include <QObject>
#include "../include/SocketIO.h"
#include "../include/Queue.h"
#include "../include/XmlHelper.h"
#include "../include/DataProtocols.h"
#include "../include/DeviceObject.h"


enum DataSourceState
{
	noData = 0,
	receiveData = 1,
	stopped = 2
};


struct Times
{
	qint64	system = 0;
	qint64	local = 0;
	qint64	plant = 0;
};


struct RupData
{
	quint32 sourceIP;

	Times time;

	int dataSize;
	char data[RUP_FRAME_DATA_SIZE * RUP_MAX_FRAME_COUNT];
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
		Tuning,
	};

	static const char* const ELEMENT_DATA_SOURCE;
	static const char* const ELEMENT_DATA_SOURCE_ASSOCIATED_SIGNALS;

protected:

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
	quint64 m_receivedFramesCount = 0;
	quint64 m_receivedPacketCount = 0;
	double m_dataReceivingRate = 0;

	// XML-serializable members
	//
	static const char* DATA_TYPE_APP;
	static const char* DATA_TYPE_DIAG;
	static const char* DATA_TYPE_TUNING;

	static const char* PROP_DATA_TYPE;
	static const char* PROP_CHANNEL;
	static const char* PROP_LM_ID;
	static const char* PROP_LM_NUMBER;
	static const char* PROP_LM_SUBSYSTEM_ID;
	static const char* PROP_LM_SUBSYSTEM;
	static const char* PROP_LM_MODULE_TYPE;
	static const char* PROP_LM_CAPTION;
	static const char* PROP_LM_ADAPTER_ID;
	static const char* PROP_LM_DATA_ENABLE;
	static const char* PROP_LM_DATA_IP;
	static const char* PROP_LM_DATA_PORT;
	static const char* PROP_LM_DATA_ID;
	static const char* PROP_COUNT;
	static const char* SIGNAL_ID_ELEMENT;

	// Properties from LM
	//
	int m_lmChannel = 0;
	DataType m_lmDataType = DataType::App;
	QString m_lmEquipmentID;
	int m_lmNumber = 0;
	int m_lmModuleType = 0;
	int m_lmSubsystemID = 0;
	QString m_lmSubsystem;
	QString m_lmCaption;
	QString m_lmAdapterID;
	bool m_lmDataEnable = false;
	HostAddressPort m_lmAddressPort;
	quint64 m_lmDataID = 0;

	QStringList m_associatedSignals;

	//

	RupFrame* m_rupFrames = nullptr;
	char* m_framesData = nullptr;

public:
	DataSource();
	~DataSource();

//	DataSource(quint32 id, QString name, QHostAddress hostAddress, quint32 partCount);
//	DataSource(const DataSource& ds);
//	DataSource& operator = (const DataSource& ds);

	int lmChannel() const { return m_lmChannel; }
	void setLmChannel(int channel) { m_lmChannel = channel; }

	DataType lmDataType() const { return m_lmDataType; }
	void setLmDataType(DataType dataType) { m_lmDataType = dataType; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	int lmNumber() const { return m_lmNumber; }
	void setLmNumber(int lmNumber) { m_lmNumber = lmNumber; }

	int lmSubsystemID() const { return m_lmSubsystemID; }
	void setLmSubsystemID(int subsystemID) { m_lmSubsystemID = subsystemID; }

	int lmModuleType() const { return m_lmModuleType; }
	void setLmModuleType(int lmModueType) { m_lmModuleType = lmModueType; }

	QString lmSubsystem() const { return m_lmSubsystem; }
	void setLmSubsystem(const QString& lmSubsystem) { m_lmSubsystem = lmSubsystem; }

	QString lmCaption() const { return m_lmCaption; }
	void setLmCaption(const QString& lmCaption) { m_lmCaption = lmCaption; }

	QString lmAdapterID() const { return m_lmAdapterID; }
	void setLmAdapterID(const QString& lmAdapterID) { m_lmAdapterID = lmAdapterID; }

	bool lmDataEnable() const { return m_lmDataEnable; }
	void setLmDataEnable(bool lmDataEnable) { m_lmDataEnable = lmDataEnable; }

	QString lmAddressStr() const { return m_lmAddressPort.addressStr(); }
	quint32 lmAddress32() const { return m_lmAddressPort.address32(); }

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

	QString dataTypeToString(DataType lmDataType);
	DataType stringToDataType(const QString& dataTypeStr);

	void writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper&);
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper&);

	void processPacket(quint32 ip, const RupFrame& rupFrame, Queue<RupData>& rupDataQueue);

	void addAssociatedSignal(const QString& appSignalID) { m_associatedSignals.append(appSignalID); }
	void clearAssociatedSignals() { m_associatedSignals.clear(); }

	const QStringList& associatedSignals() const { return m_associatedSignals; }
};
