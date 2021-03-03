#pragma once

#include <QObject>

#include "../Proto/network.pb.h"

#include "SocketIO.h"
#include "Queue.h"
#include "XmlHelper.h"
#include "DataProtocols.h"
#include "DeviceObject.h"
#include "AppSignal.h"
#include "SimpleThread.h"
#include "WUtils.h"
#include "ConstStrings.h"

class DataSource
{
public:
	enum class DataType
	{
		App,
		Diag,
		Tuning,
	};

	static const QString DATA_TYPE_APP;
	static const QString DATA_TYPE_DIAG;
	static const QString DATA_TYPE_TUNING;

private:

public:
	DataSource();
	virtual ~DataSource();

	// LM's properties
	//
	DataType lmDataType() const { return m_lmDataType; }
	int lmDataTypeInt() const { return static_cast<int>(m_lmDataType); }
	QString lmDataTypeStr() const { return dataTypeToString(m_lmDataType); }
	void setLmDataType(DataType dataType) { m_lmDataType = dataType; }

	QString lmEquipmentID() const { return m_lmEquipmentID; }
	void setLmEquipmentID(const QString& lmEquipmentID) { m_lmEquipmentID = lmEquipmentID; }

	QString lmPresetName() const { return m_lmPresetName; }
	void setLmPresetName(const QString& lmPresetName) { m_lmPresetName = lmPresetName; }

	int lmNumber() const { return m_lmNumber; }
	void setLmNumber(int lmNumber) { m_lmNumber = lmNumber; }

	QString lmSubsystemChannel() const { return m_lmSubsystemChannel; }
	void setLmSubsystemChannel(const QString& lmChannel) { m_lmSubsystemChannel = lmChannel; }

	int lmSubsystemKey() const { return m_lmSubsystemKey; }
	void setLmSubsystemKey(int subsystemKey) { m_lmSubsystemKey = subsystemKey; }

	int lmModuleType() const { return m_lmModuleType; }
	void setLmModuleType(int lmModueType) { m_lmModuleType = lmModueType; }

	QString lmSubsystemID() const { return m_lmSubsystemID; }
	void setLmSubsystemID(const QString& lmSubsystemID) { m_lmSubsystemID = lmSubsystemID; }

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
	void setLmAddressPort(const HostAddressPort& addrPort) { m_lmAddressPort = addrPort; }

	int lmPort() const { return m_lmAddressPort.port(); }
	void setLmPort(int port) { Q_ASSERT(port >= 0 && port <= 65535); m_lmAddressPort.setPort(static_cast<quint16>(port)); }

	int lmRupFramesQuantity() const { return m_lmRupFramesQuantity; }
	void setLmRupFramesQuantity(int lmRupFramesQuantity) { m_lmRupFramesQuantity = lmRupFramesQuantity; }

	quint64 lmUniqueID() const { return m_lmUniqueID; }
	void setLmUniqueID(quint64 uniqueID) { m_lmUniqueID = uniqueID; }

	quint32 lmDataID() const { return m_lmDataID; }
	void setLmDataID(quint32 lmDataID) { m_lmDataID = lmDataID; }

	int lmDataSize() const { return m_lmDataSize; }
	void setLmDataSize(int lmDataSize) { m_lmDataSize = lmDataSize; }

	QString serviceID()	 const { return m_serviceID; }
	void setServiceID(const QString& serviceID) { m_serviceID = serviceID; }

	quint64 ID() const { return m_id; }
	void setID(quint64 id) { m_id = id; }

	//

	static QString dataTypeToString(DataType lmDataType);
	static DataType stringToDataType(const QString& dataTypeStr);

	void writeToXml(XmlWriteHelper& xml) const;
	bool readFromXml(XmlReadHelper& xml);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper&) const;
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper&);

	void addAssociatedSignal(const QString& appSignalID) { m_associatedSignals.append(appSignalID); }
	void clearAssociatedSignals() { m_associatedSignals.clear(); }

	const QStringList& associatedSignals() const { return m_associatedSignals; }

	bool getInfo(Network::DataSourceInfo* protoInfo) const;
	bool setInfo(const Network::DataSourceInfo& proto);

	int lmWorkcycle_mcs() const { return m_lmWorkcycle_mcs; }
	int lmWorkcycle_ms() const { return m_lmWorkcycle_mcs / 1000; }

	static bool lanControllerFunctions(E::LanControllerType type, bool* tuning, bool* appData, bool* diagData);

private:
	quint64 generateID() const;

private:
	// Properties from LM
	//
	DataType m_lmDataType = DataType::App;
	QString m_lmEquipmentID;
	QString m_lmPresetName;
	int m_lmNumber = 0;
	int m_lmModuleType = 0;
	int m_lmSubsystemKey = 0;
	QString m_lmSubsystemChannel;				// A, B, C...
	QString m_lmSubsystemID;
	QString m_lmCaption;
	QString m_lmAdapterID;
	bool m_lmDataEnable = false;
	HostAddressPort m_lmAddressPort;
	quint32 m_lmDataID = 0;
	quint64 m_lmUniqueID = 0;				// generic 64-bit UniqueID of configuration, tuning and appLogic EEPROMs of LM
	int m_lmDataSize = 0;
	int m_lmRupFramesQuantity = 0;
	int m_lmWorkcycle_mcs = 5000;

	QString m_serviceID;

	QStringList m_associatedSignals;

	//

	quint64 m_id = 0;					// generate by DataSource::generateID() after readFromXml
};


class DataSourceOnline : public DataSource
{
private:
	struct RupFrameTime
	{
		qint64 serverTime = 0;
		bool isSimFrame = false;

		Rup::Frame rupFrame;
	};

	static const int APP_DATA_SOURCE_TIMEOUT = 1000;
	static const int DATA_RECEIVING_RATE_CALC_PERIOD = 2000;

	static const QString DATE_TIME_FORMAT_STR;

public:
	DataSourceOnline();
	~DataSourceOnline();

	bool initQueue();

	//

	E::DataSourceState state() const { return m_state; }
	void setState(E::DataSourceState state) { m_state = state; }

	qint64 uptime() const { return m_uptime; }
	void setUptime(qint64 uptime) { m_uptime = uptime; }
	void updateUptime();

	quint64 receivedDataID() const { return m_receivedDataID; }
	void setReceivedDataID(quint64 dataID) { m_receivedDataID = dataID; }

	int rupFramesQueueSize() const { return m_rupFramesQueueSize; }
	void setRupFramesQueueSize(int size) { m_rupFramesQueueSize = size; }

	int rupFramesQueueCurSize() const { return m_rupFramesQueueCurSize; }
	void setRupFramesQueueCurSize(int size) { m_rupFramesQueueCurSize = size; }

	int rupFramesQueueCurMaxSize() const { return m_rupFramesQueueCurMaxSize; }
	void setRupFramesQueueCurMaxSize(int size) { m_rupFramesQueueCurMaxSize = size; }

	qint64 rupFramePlantTime() const { return m_rupFramePlantTime; }
	QString rupFramePlantTimeStr() const;
	void setRupFramePlantTime(qint64 time) { m_rupFramePlantTime = time; }

	quint16 rupFrameNumerator() const { return m_rupFrameNumerator; }
	void setRupFrameNumerator(quint16 num) { m_rupFrameNumerator = num; }

	bool dataReceives() const { return m_dataReceives; }
	void setDataReceives(bool receives) { m_dataReceives = receives; }

	double dataReceivingRate() const { return m_dataReceivingRate; }
	void setDataReceivingRate(double rate) { m_dataReceivingRate = rate; }

	qint64 receivedDataSize() const { return m_receivedDataSize; }
	void setReceivedDataSize(qint64 dataSize) { m_receivedDataSize = dataSize; }

	qint64 receivedFramesCount() const { return m_receivedFramesCount; }
	void setReceivedFramesCount(qint64 framesCount) { m_receivedFramesCount = framesCount; }

	qint64 receivedPacketCount() const { return m_receivedPacketCount; }
	void setReceivedPacketCount(qint64 packetCount) { m_receivedPacketCount = packetCount; }

	qint64 lostPacketCount() const { return m_lostPacketCount; }
	void setLostPacketCount(qint64 packetCount) { m_lostPacketCount = packetCount; }

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
	QString lastPacketSystemTimeStr() const;
	void setLastPacketSystemTime(qint64 sysTime) { m_lastPacketSystemTime = sysTime; }

	// Functions used by receiver thread
	//
	void pushRupFrame(qint64 serverTime,
					  bool isSimFrame,
					  const Rup::Frame& rupFrame,
					  const QThread* thread);

	void incFrameSizeError() { m_errorFrameSize++; }

	// Functions used by data processing thread
	//
	bool takeProcessingOwnership(const QThread* processingThread);
	bool releaseProcessingOwnership(const QThread* processingThread);

	bool processRupFrameTimeQueue(const QThread* thread);
	bool getDataToParsing(Times* times,
						  bool* isSimPacket,
						  quint16* packetNo,
						  const char** rupData,
						  int* rupDataSize,
						  bool* dataReceivingTimeout);

	bool rupFramesQueueIsEmpty() const { return m_rupFrameTimeQueue.isEmpty(QThread::currentThread()); }

	// Used by PacketViewer
	//
	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

private:
	bool collect(const RupFrameTime& rupFrameTime);
	bool reallocate(quint32 framesQuantity);

	void calcDataReceivingRate();

	QString getTimeStr(qint64 timeMs) const;

private:
	// static information
	//
	QVector<int> m_relatedSignalIndexes;

	// dynamic state information
	//
	E::DataSourceState m_state = E::DataSourceState::NoData;
	qint64 m_uptime = 0;										// in seconds!
	quint64 m_receivedDataID = 0;

	qint32 m_rupFramesQueueSize = 0;
	qint32 m_rupFramesQueueCurSize = 0;
	qint32 m_rupFramesQueueCurMaxSize = 0;

	qint64 m_rupFramePlantTime = 0;
	quint16 m_rupFrameNumerator = 0;
	bool m_dataReceives = false;

	double m_dataReceivingRate = 0;
	qint64 m_receivedDataSize = 0;
	qint64 m_receivedFramesCount = 0;
	qint64 m_receivedPacketCount = 0;
	qint64 m_lostPacketCount = 0;
	qint64 m_processedPacketCount = 0;

	bool m_dataRecevingTimeout = false;

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

	qint64 m_firstPacketSystemTime = 0;
	qint64 m_lastPacketSystemTime = 0;
	bool m_firstRupFrame = true;

	//

	FastThreadSafeQueue<RupFrameTime> m_rupFrameTimeQueue;				// filled by AppDataReceiver

	//

	std::atomic<const QThread*> m_processingOwner = { nullptr };

	//

	quint32 m_framesQuantityAllocated = 0;
	Rup::Header* m_rupFramesHeaders = nullptr;
	Rup::Data* m_rupFramesData = nullptr;
	qint64 m_frame0ServerTime = 0;
	bool m_isSimPacket = false;

	// result variables

	bool m_dataReadyToParsing = false;

	Times m_rupDataTimes;
	Times m_lastRupDataTimes;
	quint16 m_packetNo = 0;
	int m_rupDataSize = 0;

	// variables to calc data receiving rate
	//
	bool m_firstCalc = true;
	int m_calcFramesCtr = 0;
	qint64 m_prevCalcTime = -1;
	qint64 m_prevReceivedSize = -1;
};


template <typename TYPE>				// TYPE should be DataSource-derived class
class DataSourcesXML
{
public:
	static bool writeToXml(const QVector<TYPE>& dataSources, QByteArray* fileData);
	static bool readFromXml(const QByteArray& fileData, QVector<TYPE>* dataSources);
};


// -----------------------------------------------------------------------------
//
// DataSourcesXML class implementation
//
// -----------------------------------------------------------------------------

template <typename TYPE>
bool DataSourcesXML<TYPE>::writeToXml(const QVector<TYPE>& dataSources, QByteArray* fileData)
{
	if (fileData == nullptr)
	{
		assert(false);
		return false;
	}

	fileData->clear();

	QXmlStreamWriter xmlWriter(fileData);
	XmlWriteHelper xml(xmlWriter);

	xml.setAutoFormatting(true);
	xml.writeStartDocument();

	xml.writeStartElement(XmlElement::DATA_SOURCES);
	xml.writeIntAttribute(XmlAttribute::COUNT, dataSources.count());

	for(const TYPE& ds : dataSources)
	{
		ds.writeToXml(xml);
	}

	xml.writeEndElement();	// </DataSources>
	xml.writeEndDocument();

	return true;
}

template <typename TYPE>
bool DataSourcesXML<TYPE>::readFromXml(const QByteArray& fileData, QVector<TYPE> *dataSources)
{
	TEST_PTR_RETURN_FALSE(dataSources);

	XmlReadHelper xml(fileData);

	dataSources->clear();

	bool result = true;

	if (xml.findElement(XmlElement::DATA_SOURCES) == false)
	{
		return false;
	}

	int count = 0;

	if (xml.readIntAttribute(XmlAttribute::COUNT, &count) == false)
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


