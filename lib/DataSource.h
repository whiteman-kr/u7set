#pragma once

#include <QObject>

#include "../Proto/network.pb.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Queue.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/DataProtocols.h"
#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/CircularLogger.h"
#include "../CommonLib/Times.h"
#include "../CommonLib/HostAddressPort.h"
#include "ConstStrings.h"
#include "LanControllerInfo.h"

class DataSource
{
public:
	DataSource();
	virtual ~DataSource();

	// LM's properties
	//
	QString moduleEquipmentID() const { return m_moduleEquipmentID; }
	void setModuleEquipmentID(const QString& equipmentID) { m_moduleEquipmentID = equipmentID; }

	QString modulePresetName() const { return m_modulePresetName; }
	void setModulePresetName(const QString& presetName) { m_modulePresetName = presetName; }

	int lmNumber() const { return m_lmNumber; }
	void setLmNumber(int number) { m_lmNumber = number; }

	QString subsystemChannel() const { return m_subsystemChannel; }
	void setSubsystemChannel(const QString& channel) { m_subsystemChannel = channel; }

	int subsystemKey() const { return m_subsystemKey; }
	void setSubsystemKey(int key) { m_subsystemKey = key; }

	int moduleType() const { return m_moduleType; }
	void setModuleType(int moduleType) { m_moduleType = moduleType; }

	quint64 moduleUniqueID() const { return m_moduleUniqueID; }
	void setModuleUniqueID(quint64 uid) { m_moduleUniqueID = uid; }

	QString subsystemID() const { return m_subsystemID; }
	void setSubsystemID(const QString& id) { m_subsystemID = id; }

	QString moduleCaption() const { return m_moduleCaption; }
	void setModuleCaption(const QString& lmCaption) { m_moduleCaption = lmCaption; }

	int appDataFramesQuantity() const;
	int diagDataFramesQuantity() const;

	int appDataSizeBytes() const;
	int diagDataSizeBytes() const;

	int overrideAppDataWordCount() const;
	int overrideDiagDataWordCount() const;

	quint32 appDataUID() const;
	quint32 diagDataUID() const;
	quint64 tuningDataUID() const;

	quint64 ID() const { return m_id; }
	void setID(quint64 id) { m_id = id; }

	const LanControllersInfo& lanControllersInfo() const { return m_lanControllersInfo; }
	LanControllersInfo& lanControllersInfo() { return m_lanControllersInfo; }

	void appendAssociatedSignal(E::LanControllerType lanType, const QString& signalID);
	void clearAssociatedSignals(E::LanControllerType lanType);
	const QStringList& associatedSignals(E::LanControllerType lanType) const;

	QString profile() const { return m_profile; }
	void setProfile(QString profile) { m_profile = profile; }

	int moduleWorkcycle_ms() const { return m_moduleWorkcycle_mcs / 1000; }

	int rupVersion() const { return m_lanControllersInfo.rupVersion(); }
	int fotipVersion() const { return m_lanControllersInfo.fotipVersion(); }

	//

	void writeToXml(XmlWriteHelper& xml) const;
	bool readFromXml(XmlReadHelper& xml);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper&) const;
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper&);

	bool saveToProto(Network::DataSourceInfo* protoInfo) const;
	bool loadFromProto(const Network::DataSourceInfo& proto);

	//

private:
	quint64 generateID() const;

protected:
	quint64 m_id = 0;						// generate by DataSource::generateID() after readFromXml

	QString m_moduleEquipmentID;
	QString m_modulePresetName;
	int m_moduleType = 0;
	QString m_moduleCaption;
	quint64 m_moduleUniqueID = 0;			// generic 64-bit UniqueID of configuration, tuning and appLogic EEPROMs of LM

	QString m_subsystemID;
	int m_subsystemKey = 0;
	int m_lmNumber = 0;
	QString m_subsystemChannel;				// A, B, C...

	LanControllersInfo m_lanControllersInfo;		// array of LanControllerInfo!

	int m_moduleWorkcycle_mcs = 5000;		// module workcycle in MICROseconds

	QString m_profile;

	QStringList m_appSignals;
	QStringList m_diagSignals;
	QStringList m_tuningSignals;

	static QStringList m_emptyList;
};


class DataSourceOnline : public DataSource
{
private:
	struct RupFrameTime
	{
		quint32 sourceIP = 0;
		qint64 serverTime = 0;
		bool isSimFrame = false;

		Rup::Frame rupFrame;
	};

	static const int APP_DATA_SOURCE_TIMEOUT = 1000;
	static const int DATA_RECEIVING_RATE_CALC_PERIOD = 2000;

	static const QString DATE_TIME_FORMAT_STR;

protected:

	struct ParsingBuffer
	{
		quint16 framesQuantity = 0;
		Rup::Header* rupFramesHeaders = nullptr;	// array of REVERSED headers
		Rup::Data* rupFramesData = nullptr;
		qint64 frame0ServerTime = 0;
		bool isSimPacket = false;

		std::atomic<bool> readyToParsing{false};	// modified by both Receiver and ProcessingThread

		ParsingBuffer();
		~ParsingBuffer();

		void clear();
		void allocate(int frmsCount);
		bool copyRupFrame(int frameNo, qint64 serverTime,
						  bool simFrame, const Rup::Frame& rupFrame);
		void prepareToWriting();

		const Rup::Header& frame0Header() const;
		const char* rupData() const;
		int rupDataSize() const;
	};

public:
	DataSourceOnline();
	virtual ~DataSourceOnline();

	bool initParsingBuffers(int framesQuantity);
	void clearParsingBuffers();

	// Functions used by receiver thread
	//
	void pushRupFrame(quint32 sourceIP,
					  qint64 serverTime,
					  bool isSimFrame,
					  const Rup::Frame& rupFrame,
					  const QThread* thread);

	bool updateStatistics_1s();

	// Functions used by data processing thread
	//
	bool parseNextBuffer(const QThread* thread);
	virtual bool parseBuffer(ParsingBuffer& readBuffer, const QThread* thread);

	void checkPlantTime(const Rup::TimeStamp& plantTimeStamp);

	bool takeProcessingOwnership(const QThread* processingThread);
	bool releaseProcessingOwnership(const QThread* processingThread);

	//

	qint64 uptime() const { return m_uptime; }
	void setUptime(qint64 uptime) { m_uptime = uptime; }
	void updateUptime();

	quint64 receivedDataID() const { return m_receivedDataID; }
	void setReceivedDataID(quint64 dataID) { m_receivedDataID = dataID; }

	qint64 rupFramePlantTime() const { return m_rupFramePlantTime; }
	QString rupFramePlantTimeStr() const;
	void setRupFramePlantTime(qint64 time) { m_rupFramePlantTime = time; }

	quint16 rupFrameNumerator() const { return static_cast<quint16>(m_rupFrameNumerator); }
	void setRupFrameNumerator(quint16 num) { m_rupFrameNumerator = num; }

	bool receivesData() const { return m_receivesData; }
	void setReceivesData(bool receives) { m_receivesData = receives; }

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

	qint64 errorPlantTimeFormat() const { return m_errorPlantTimeFormat; }
	void setErrorPlantTimeFormat(qint64 err) { m_errorPlantTimeFormat = err; }

	bool dataProcessingEnabled() const { return m_dataProcessingEnabled; }
	void setDataProcessingEnabled(bool enabled) { m_dataProcessingEnabled = enabled; }

	qint64 lastPacketSystemTime() const { return m_lastPacketServerTime; }
	QString lastPacketSystemTimeStr() const;
	void setLastPacketSystemTime(qint64 sysTime) { m_lastPacketServerTime = sysTime; }

	QString stateStr() const;

	// Used by PacketViewer
	//
	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

	void setTimeErrLog(CircularLoggerShared timeErrLog) { m_timeErrLog = timeErrLog; }

private:
	bool moveToNextWriteBuffer(const QThread* thread);
	bool moveToNextReadBuffer(const QThread* thread);

	QString getTimeStr(qint64 timeMs) const;
	QString getTimeStr(const Rup::TimeStamp& ts) const;

protected:
	// static information
	//
	QVector<int> m_relatedSignalIndexes;
	CircularLoggerShared m_timeErrLog;

	quint32 m_cachedDataUID = 0;

	// dynamic state information
	//
	bool m_dataProcessingEnabled = true;
	bool m_receivesData = false;

	qint64 m_uptime = 0;										// in seconds!
	quint64 m_receivedDataID = 0;

	qint64 m_rupFramePlantTime = 0;
	qint64 m_rupFrameNumerator = -1;			// qint64 is Ok!

	double m_dataReceivingRate = 0;
	qint64 m_receivedDataSize = 0;
	qint64 m_prevReceivedDataSize = 0;
	qint64 m_receivedFramesCount = 0;
	qint64 m_receivedPacketCount = 0;
	qint64 m_lostPacketCount = 0;
	qint64 m_processedPacketCount = 0;

	//

	qint64 m_errorProtocolVersion = 0;
	qint64 m_errorFramesQuantity = 0;
	qint64 m_errorFrameNo = 0;
	qint64 m_errorDataID = 0;
	qint64 m_errorFrameSize = 0;
	qint64 m_errorDuplicatePlantTime = 0;
	qint64 m_errorNonmonotonicPlantTime = 0;
	qint64 m_errorPlantTimeFormat = 0;

	//

	qint64 m_firstPacketServerTime = 0;
	qint64 m_lastPacketServerTime = 0;

	//

	std::atomic<const QThread*> m_processingOwner = { nullptr };

	//

	static const int PARSING_BUFFERS_COUNT = 5;

	std::vector<ParsingBuffer*> m_parsingBuffers;

	QueueIndex m_writeBufferIndex = 0;		// modified by packet Receiver only in pushRupFrame
	QueueIndex m_readBufferIndex = 0;		// modified only by ProcessingThread

	SimpleMutex m_parsingBuffersMutex;		// locks only while m_writeBufferIndex and m_readBufferIndex modyfied

	// result variables

	Times m_rupTimes;
	Times m_lastRupTimes;
	quint16 m_packetNo = 0;
	int m_rupDataSize = 0;
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
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(dataSources.count()));

	for(const TYPE& ds : dataSources)
	{
		ds.writeToXml(xml);
	}

	xml.writeEndElement();	// </DataSources>
	xml.writeEndDocument();

	return true;
}

template <typename TYPE>
bool DataSourcesXML<TYPE>::readFromXml(const QByteArray& fileData, QVector<TYPE>* dataSources)
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


