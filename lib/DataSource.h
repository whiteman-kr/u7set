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
	static const char* PROP_LM_SUBSYSTEM_ID;
	static const char* PROP_LM_SUBSYSTEM;
	static const char* PROP_LM_MODULE_TYPE;
	static const char* PROP_LM_CAPTION;
	static const char* PROP_LM_ADAPTER_ID;
	static const char* PROP_LM_DATA_ENABLE;
	static const char* PROP_LM_DATA_IP;
	static const char* PROP_LM_DATA_PORT;
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

	quint64 uniqueID() const { return m_uniqueID; }
	void setUniqueID(quint64 uniqueID) { m_uniqueID = uniqueID; }

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
	bool setInfo(const Network::DataSourceInfo& protoInfo);

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
	QString m_lmChannel = 0;
	QString m_lmSubsystem;
	QString m_lmCaption;
	QString m_lmAdapterID;
	bool m_lmDataEnable = false;
	HostAddressPort m_lmAddressPort;
	quint32 m_lmDataID = 0;
	quint64 m_uniqueID = 0;				// generic 64-bit UniqueID of configuration, tuning and appLogic EEPROMs of LM

	QStringList m_associatedSignals;

	quint64 m_id = 0;
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

	void stop();
	void resume();

	//

	QHostAddress hostAddress() const { return m_hostAddress; }
	void setHostAddress(QHostAddress hostAddress) { m_hostAddress = hostAddress; }

	quint32 partCount() const { return m_partCount; }
	void setPartCount(quint32 partCount) { m_partCount = partCount; }

	QString name() const { return m_name; }

	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }


	E::DataSourceState state() const { return m_state; }
	void setState(E::DataSourceState state) { m_state = state; }

	quint64 uptime() const { return m_uptime; }
	void setUptime(quint64 uptime) { m_uptime = uptime; }

	quint64 receivedDataSize() const { return m_receivedDataSize; }
	void setReceivedDataSize(quint64 dataSize) { m_receivedDataSize = dataSize; }

	double dataReceivingRate() const { return m_dataReceivingRate; }
	void setDataReceivingRate(double rate) { m_dataReceivingRate = rate; }

	qint64 errorProtocolVersion() const { return m_errorProtocolVersion; }
	void setErrorProtocolVersion(qint64 err) { m_errorProtocolVersion = err; }

	qint64 errorFramesQuantity() const { return m_errorFramesQuantity; }
	void setErrorFramesQuantity(qint64 err) { m_errorFramesQuantity = err; }

	qint64 errorFrameNo() const { return m_errorFrameNo; }
	void setErrorFrameNo(qint64 errFrameNo) { m_errorFrameNo = errFrameNo; }

	qint64 lostedFramesCount() const { return m_lostedFramesCount; }
	void setLostedFramesCount(qint64 lostedCount) { m_lostedFramesCount = lostedCount; }

	qint64 errorDataID() const { return m_errorDataID; }

	qint64 errorBadFrameSize() const { return m_errorBadFrameSize; }
	void setErrorBadFrameSize(qint64 errBadFrame) { m_errorBadFrameSize = errBadFrame; }

	bool hasErrors() const { return m_hasErrors; }
	void setHasErrors(bool hasErrors) { m_hasErrors = hasErrors; }

	bool dataProcessingEnabled() const { return m_dataProcessingEnabled; }
	void setDataProcessingEnabled(bool enabled) { m_dataProcessingEnabled = enabled; }

	qint64 receivedFramesCount() const { return m_receivedFramesCount; }
	void setReceivedFramesCount(qint64 framesCount) { m_receivedFramesCount = framesCount; }

	qint64 receivedPacketCount() const { return m_receivedPacketCount; }
	void setReceivedPacketCount(qint64 packetCount) { m_receivedPacketCount = packetCount; }

	qint64 lastPacketTime() const { return m_lastPacketTime; }
	void setLastPacketTime(qint64 time) { m_lastPacketTime = time; }

	// Functions used by receiver thread
	//
	void pushRupFrame(qint64 serverTime, const Rup::Frame& rupFrame);
	void incBadFrameSizeError() { m_errorBadFrameSize++; }

	// Functions used by data processing thread
	//
	bool seizeProcessingOwnership(const QThread* processingThread);
	bool releaseProcessingOwnership(const QThread* processingThread);

	bool processRupFrameTimeQueue();
	bool getDataToParsing(Times* times, const char** rupData, quint32* rupDataSize);

	bool rupFramesQueueIsEmpty() const { return m_rupFrameTimeQueue.isEmpty(); }
	int rupFramesQueueMaxSize() const { return m_rupFrameTimeQueue.maxSize(); }

private:
	bool collect(const RupFrameTime& rupFrameTime);
	bool reallocate(quint32 framesQuantity);

private:
	// static information
	//
	QHostAddress m_hostAddress;
	QString m_name;
	quint32 m_partCount = 1;
	QVector<int> m_relatedSignalIndexes;

	// dynamic state information
	//
	E::DataSourceState m_state = E::DataSourceState::NoData;
	qint64 m_uptime = 0;
	qint64 m_receivedDataSize = 0;
	qint64 m_receivedFramesCount = 0;
	qint64 m_receivedPacketCount = 0;
	double m_dataReceivingRate = 0;
	bool m_dataReceived = false;

	qint64 m_errorProtocolVersion = 0;
	qint64 m_errorFramesQuantity = 0;
	qint64 m_errorFrameNo = 0;
	qint64 m_lostedFramesCount = 0;
	qint64 m_errorDataID = 0;
	qint64 m_errorBadFrameSize = 0;

	bool m_hasErrors = false;

	bool m_dataProcessingEnabled = true;

	qint64 m_lastPacketTime = 0;

	bool m_firstRupFrame = true;
	quint16 m_rupFrameNumerator = 0;

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
