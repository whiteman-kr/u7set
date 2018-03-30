#pragma once

#include <QObject>
#include "../lib/SocketIO.h"
#include "../lib/Queue.h"
#include "../lib/XmlHelper.h"
#include "../lib/DataProtocols.h"
#include "../lib/DeviceObject.h"
#include "../lib/AppSignal.h"
#include "../Proto/network.pb.h"
#include "../lib/SimpleThread.h"


class DataSource /*: public QObject*/
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

	// XML-serializable members
	//
	static const char* DATA_TYPE_APP;
	static const char* DATA_TYPE_DIAG;
	static const char* DATA_TYPE_TUNING;

	static const char* PROP_DATA_TYPE;
	static const char* PROP_CHANNEL;
	static const char* PROP_LM_ID;
	static const char* PROP_LM_NUMBER;
	static const char* PROP_LM_CHANNEL;
	static const char* PROP_LM_SUBSYSTEM_ID;
	static const char* PROP_LM_SUBSYSTEM;
	static const char* PROP_LM_MODULE_TYPE;
	static const char* PROP_LM_CAPTION;
	static const char* PROP_LM_ADAPTER_ID;
	static const char* PROP_LM_DATA_ENABLE;
	static const char* PROP_LM_DATA_IP;
	static const char* PROP_LM_DATA_PORT;
	static const char* PROP_LM_RUP_FRAMES_QUANTITY;
	static const char* PROP_LM_DATA_ID;
	static const char* PROP_LM_UNIQUE_ID;
	static const char* PROP_COUNT;
	static const char* SIGNAL_ID_ELEMENT;

private:
public:
	DataSource();
	~DataSource();

	// LM's properties
	//
	DataType lmDataType() const { return m_lmDataType; }
	QString lmDataTypeStr() const { return dataTypeToString(m_lmDataType); }
	void setLmDataType(DataType dataType) { m_lmDataType = dataType; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	int lmNumber() const { return m_lmNumber; }
	void setLmNumber(int lmNumber) { m_lmNumber = lmNumber; }

	QString lmSubsystemChannel() const { return m_lmSubsystemChannel; }
	void setLmSubsystemChannel(const QString& lmChannel) { m_lmSubsystemChannel = lmChannel; }

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
	HostAddressPort lmAddressPort() const { return m_lmAddressPort; }

	int lmPort() const { return m_lmAddressPort.port(); }
	void setLmPort(int port) { m_lmAddressPort.setPort(port); }

	int lmRupFramesQuantity() const { return m_lmRupFramesQuantity; }
	void setLmRupFramesQuantity(int lmRupFramesQuantity) { m_lmRupFramesQuantity = lmRupFramesQuantity; }

	quint64 lmUniqueID() const { return m_lmUniqueID; }
	void setLmUniqueID(quint64 uniqueID) { m_lmUniqueID = uniqueID; }

	quint32 lmDataID() const { return m_lmDataID; }
	void setLmDataID(quint32 lmDataID) { m_lmDataID = lmDataID; }

	quint64 ID() const { return m_id; }
	void setID(quint32 id) { m_id = id; }

	QString dataTypeToString(DataType lmDataType) const;
	DataType stringToDataType(const QString& dataTypeStr);

	void writeToXml(XmlWriteHelper& xml) const;
	bool readFromXml(XmlReadHelper& xml);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper&) const;
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper&);

	void addAssociatedSignal(const QString& appSignalID) { m_associatedSignals.append(appSignalID); }
	void clearAssociatedSignals() { m_associatedSignals.clear(); }

	const QStringList& associatedSignals() const { return m_associatedSignals; }

	bool getInfo(Network::DataSourceInfo* protoInfo) const;
	bool setInfo(const Network::DataSourceInfo& proto);

private:
	quint64 generateID() const;

private:
	// Properties from LM
	//
	DataType m_lmDataType = DataType::App;
	QString m_lmEquipmentID;
	int m_lmNumber = 0;
	int m_lmModuleType = 0;
	int m_lmSubsystemID = 0;
	QString m_lmSubsystemChannel;				// A, B, C...
	QString m_lmSubsystem;
	QString m_lmCaption;
	QString m_lmAdapterID;
	bool m_lmDataEnable = false;
	HostAddressPort m_lmAddressPort;
	quint32 m_lmDataID = 0;
	quint64 m_lmUniqueID = 0;				// generic 64-bit UniqueID of configuration, tuning and appLogic EEPROMs of LM
	int m_lmRupFramesQuantity = 0;

	QStringList m_associatedSignals;

	//

	quint64 m_id = 0;					// generate by DataSource::generateID() after readFromXml
};


class DataSourcesXML
{
public:
	static bool writeToXml(const QVector<DataSource>& dataSources, QByteArray* fileData);
	static bool readFromXml(const QByteArray& fileData, QVector<DataSource>* dataSources);
};

class DataSourceOnline : public DataSource
{
private:
	struct RupFrameTime
	{
		qint64 serverTime;

		Rup::Frame rupFrame;

		void dump();
	};

public:
	DataSourceOnline();
	~DataSourceOnline();

	bool init();

//	void stop();
//	void resume();

	//

	E::DataSourceState state() const { return m_state; }
	void setState(E::DataSourceState state) { m_state = state; }

	qint64 uptime() const { return m_uptime; }
	void setUptime(qint64 uptime) { m_uptime = uptime; }

	quint64 receivedDataID() const { return m_receivedDataID; }
	void setReceivedDataID(quint64 dataID) { m_receivedDataID = dataID; }

	int rupFramesQueueSize() const { return m_rupFramesQueueSize; }
	void setRupFramesQueueSize(int size) { m_rupFramesQueueSize = size; }

	int rupFramesQueueMaxSize() const { return m_rupFramesQueueMaxSize; }
	void setRupFramesQueueMaxSize(int size) { m_rupFramesQueueMaxSize = size; }

	qint64 rupFramePlantTime() const { return m_rupFramePlantTime; }
	void setRupFramePlantTime(qint64 time) { m_rupFramePlantTime = time; }

	qint32 rupFrameNumerator() const { return m_rupFrameNumerator; }
	void setRupFrameNumerator(qint32 num) { m_rupFrameNumerator = num; }

	bool dataReceives() const { return m_dataReceives; }
	void setDataReceives(bool receives) { m_dataReceives = receives; }

	double dataReceivingRate() const { return m_dataReceivingRate; }
	void setDataReceivingRate(double rate) { m_dataReceivingRate = rate; }

	quint64 receivedDataSize() const { return m_receivedDataSize; }
	void setReceivedDataSize(quint64 dataSize) { m_receivedDataSize = dataSize; }

	qint64 receivedFramesCount() const { return m_receivedFramesCount; }
	void setReceivedFramesCount(qint64 framesCount) { m_receivedFramesCount = framesCount; }

	qint64 receivedPacketCount() const { return m_receivedPacketCount; }
	void setReceivedPacketCount(qint64 packetCount) { m_receivedPacketCount = packetCount; }

	qint64 lostedPacketCount() const { return m_lostedPacketCount; }
	void setLostedPacketCount(qint64 packetCount) { m_lostedPacketCount = packetCount; }

	qint64 processedPacketCount() const { return m_processedPacketCount; }
	void setProcessedPacketCount(qint64 packetCount) { m_processedPacketCount = packetCount; }

	qint64 errorProtocolVersion() const { return m_errorProtocolVersion; }
	void setErrorProtocolVersion(qint64 err) { m_errorProtocolVersion = err; }

	qint64 errorFramesQuantity() const { return m_errorFramesQuantity; }
	void setErrorFramesQuantity(qint64 err) { m_errorFramesQuantity = err; }

	qint64 errorFrameNo() const { return m_errorFrameNo; }
	void setErrorFrameNo(qint64 errFrameNo) { m_errorFrameNo = errFrameNo; }

	qint64 errorDataID() const { return m_errorDataID; }
	void setErrorDataID(qint64 err) { m_errorDataID = err; }

	qint64 errorFrameSize() const { return m_errorFrameSize; }
	void setErrorFrameSize(qint64 errFrameSize) { m_errorFrameSize = errFrameSize; }

	qint64 errorDuplicatePlantTime() const { return m_errorDuplicatePlantTime; }
	void setErrorDuplicatePlantTime(qint64 err) { m_errorDuplicatePlantTime = err; }

	qint64 errorNonmonotonicPlantTime() const { return m_errorNonmonotonicPlantTime; }
	void setErrorNonmonotonicPlantTime(qint64 err) { m_errorNonmonotonicPlantTime = err; }

	bool dataProcessingEnabled() const { return m_dataProcessingEnabled; }
	void setDataProcessingEnabled(bool enabled) { m_dataProcessingEnabled = enabled; }

	qint64 lastPacketSystemTime() const { return m_lastPacketSystemTime; }
	void setLastPacketSystemTime(qint64 sysTime) { m_lastPacketSystemTime = sysTime; }

	// Functions used by receiver thread
	//
	void pushRupFrame(qint64 serverTime, const Rup::Frame& rupFrame);
	void incFrameSizeError() { m_errorFrameSize++; }

	// Functions used by data processing thread
	//
	bool seizeProcessingOwnership(const QThread* processingThread);
	bool releaseProcessingOwnership(const QThread* processingThread);

	bool processRupFrameTimeQueue();
	bool getDataToParsing(Times* times, const char** rupData, quint32* rupDataSize);

	bool rupFramesQueueIsEmpty() const { return m_rupFrameTimeQueue.isEmpty(); }

	// Used by PacketViewer
	//
	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

private:
	bool collect(const RupFrameTime& rupFrameTime);
	bool reallocate(quint32 framesQuantity);

private:
	// static information
	//
	QVector<int> m_relatedSignalIndexes;

	// dynamic state information
	//
	E::DataSourceState m_state = E::DataSourceState::NoData;
	qint64 m_uptime = 0;
	quint64 m_receivedDataID = 0;

	qint32 m_rupFramesQueueSize = 0;
	qint32 m_rupFramesQueueMaxSize = 0;

	qint32 m_rupFramePlantTime = 0;
	quint16 m_rupFrameNumerator = 0;
	bool m_dataReceives = false;
	double m_dataReceivingRate = 0;
	qint64 m_receivedDataSize = 0;
	qint64 m_receivedFramesCount = 0;
	qint64 m_receivedPacketCount = 0;
	qint64 m_lostedPacketCount = 0;
	qint64 m_processedPacketCount = 0;

	//

	qint64 m_errorProtocolVersion = 0;
	qint64 m_errorFramesQuantity = 0;
	qint64 m_errorFrameNo = 0;
	qint64 m_errorDataID = 0;
	qint64 m_errorFrameSize = 0;
	qint64 m_errorDuplicatePlantTime = 0;
	qint64 m_errorNonmonotonicPlantTime = 0;

	bool m_dataProcessingEnabled = true;

	//

	qint64 m_lastPacketSystemTime = 0;
	bool m_firstRupFrame = true;

	//

	LockFreeQueue<RupFrameTime> m_rupFrameTimeQueue;				// fast non-blocking queue filled by AppDataReceiver

	//

	std::atomic<const QThread*> m_processingOwner = nullptr;

	//

	quint32 m_framesQuantityAllocated = 0;
	Rup::Header* m_rupFramesHeaders = nullptr;
	Rup::Data* m_rupFramesData = nullptr;

	qint64 m_firstFrameServerTime = 0;

	// result variables

	bool m_dataReadyToParsing = false;

	Times m_rupDataTimes;
	quint32 m_rupDataSize = 0;
};
