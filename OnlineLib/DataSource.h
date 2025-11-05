#pragma once

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Queue.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/CircularLogger.h"
#include <CommonLib/Times.h>
#include <CommonLib/ConstStrings.h>
#include <HardwareLib/DataProtocols.h>
#include <HardwareLib/LanControllerInfo.h>

namespace Network
{
	class DataSourceInfo;
}

namespace OnlineLib
{
	class DataSource
	{
	public:
		DataSource();
		virtual ~DataSource();

		// Module properties

		QString moduleEquipmentID() const { return m_moduleEquipmentID; }
		void setModuleEquipmentID(const QString& equipmentID) { m_moduleEquipmentID = equipmentID; }

		QString moduleCaption() const { return m_moduleCaption; }
		void setModuleCaption(const QString& lmCaption) { m_moduleCaption = lmCaption; }

		int moduleType() const { return m_moduleType; }
		void setModuleType(int moduleType) { m_moduleType = moduleType; }

		QString modulePresetName() const { return m_modulePresetName; }
		void setModulePresetName(const QString& presetName) { m_modulePresetName = presetName; }

		quint64 moduleUniqueID() const { return m_moduleUniqueID; }
		void setModuleUniqueID(quint64 uid) { m_moduleUniqueID = uid; }

		int moduleWorkcycle_ms() const { return m_moduleWorkcycle_mcs / 1000; }
		int moduleWorkcycle_mcs() const { return m_moduleWorkcycle_mcs; }
		void setModuleWorkcycle_mcs(int mcs) { m_moduleWorkcycle_mcs = mcs; }

		int rupVersion() const { return m_lanControllersInfo.rupVersion(); }

		// Subsystem properties

		QString subsystemID() const { return m_subsystemID; }
		void setSubsystemID(const QString& id) { m_subsystemID = id; }

		int subsystemKey() const { return m_subsystemKey; }
		void setSubsystemKey(int key) { m_subsystemKey = key; }

		int lmNumber() const { return m_lmNumber; }
		void setLmNumber(int number) { m_lmNumber = number; }

		QString subsystemChannel() const { return m_subsystemChannel; }
		void setSubsystemChannel(const QString& channel) { m_subsystemChannel = channel; }

		// AppData properties
		//
		quint32 rupAppDataUID() const;
		int appDataFramesQuantity() const;
		int appDataSizeBytes() const;
		int overrideAppDataWordCount() const;
		int appSignalsCount() const;

		// DiagData properties
		//
		quint32 rupDiagDataUID() const;
		int diagDataSizeBytes() const;
		int diagDataFramesQuantity() const;
		int overrideDiagDataWordCount() const;
		int diagSignalsCount() const;

		// Tuning properties
		//
		quint32 rupTuningDataUID() const;
		quint64 fotipTuningDataUID() const;
		int fotipVersion() const;

		//

		quint64 ID() const { return m_id; }
		void setID(quint64 id) { m_id = id; }

		const LanControllersInfo& lanControllersInfo() const { return m_lanControllersInfo; }
		LanControllersInfo& lanControllersInfo() { return m_lanControllersInfo; }

		void appendAssociatedSignal(E::LanControllerType lanType, const QString& signalID);
		void clearAssociatedSignals(E::LanControllerType lanType);
		const QStringList& associatedSignals(E::LanControllerType lanType) const;
		void appendSwCalcSignal(const QString& appSignalID);

		QString profile() const { return m_profile; }
		void setProfile(QString profile) { m_profile = profile; }

		virtual quint32 getExpectedDataUID() const { return 0; }

		//

		void writeToXml(XmlWriteHelper& xml) const;
		bool readFromXml(XmlReadHelper& xml);

		virtual void writeAdditionalSectionsToXml(XmlWriteHelper&) const;
		virtual bool readAdditionalSectionsFromXml(XmlReadHelper&);

		bool saveToProto(Network::DataSourceInfo* protoInfo) const;
		bool loadFromProto(const Network::DataSourceInfo& proto);

		static QString getTimeStr(qint64 timeMs);
		static QString getTimeStr(const Rup::TimeStamp& ts);

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

		LanControllersInfo m_lanControllersInfo;	// array of LanControllerInfo!

		int m_moduleWorkcycle_mcs = 0;				// module workcycle in MICROseconds
		int m_workcycle_ms = 0;

		QString m_profile;
		int m_acquiredSignalsCount = 0;

		QStringList m_appSignals;
		QStringList m_diagSignals;
		QStringList m_tuningSignals;
		QStringList m_swCalcSignals;

		inline static QStringList m_emptyList;
	};


	class DataSourceOnline : public DataSource
	{
	private:
		static constexpr int APP_DATA_SOURCE_TIMEOUT = 1000;

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
						  Rup::Frame& rupFrame,
						  quint32 expectedDataUID);

		bool updateStatistics_500ms(int oneSecond, QString& logStr);

		// Functions used by data processing thread
		//
		bool parseNextBuffer();
		virtual bool parseBuffer(ParsingBuffer& readBuffer);

		void timeCorrection(const ParsingBuffer& readBuffer);

		void checkPlantTime(const Rup::TimeStamp& plantTimeStamp);

		bool takeProcessingOwnership() noexcept;
		bool releaseProcessingOwnership() noexcept;

		//

		QString stateStr() const;

		bool dataProcessingEnabled() const { return m_dataProcessingEnabled; }
		bool receivesData() const { return m_receivesData; }
		qint64 uptime() const { return m_uptime; }
		double dataReceivingSpeed() const { return m_dataReceivingSpeed; }
		qint64 receivedDataSize() const { return m_receivedDataSize; }
		qint64 receivedFramesCount() const { return m_receivedFramesCount; }
		qint64 receivedPacketCount() const { return m_receivedPacketCount; }
		quint32 receivedDataID() const { return m_receivedDataID; }
		qint64 rupFramePlantTime() const { return m_lmTime; }
		QString rupFramePlantTimeStr() const;
		quint16 rupFrameNumerator() const { return static_cast<quint16>(m_rupFrameNumerator); }
		qint64 lostPacketCount() const { return m_lostPacketCount; }

		qint64 errorProtocolVersion() const { return m_errorProtocolVersion; }
		qint64 errorFramesQuantity() const { return m_errorFramesQuantity; }
		qint64 errorFrameNo() const { return m_errorFrameNo; }

		qint64 errorFrameCRC() const { return m_errorFrameCRC; }
		void incErrorFrameCRC() { m_errorFrameCRC++; }

		qint64 errorDataID() const { return m_errorDataID; }
		qint64 errorDuplicatePlantTime() const { return m_errorDuplicatePlantTime; }
		qint64 errorNonmonotonicPlantTime() const { return m_errorNonmonotonicPlantTime; }
		qint64 errorPlantTimeFormat() const { return m_errorPlantTimeFormat; }

		// Used by PacketViewer
		//
		void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
		const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

		void setTimeErrLog(CircularLoggerShared timeErrLog) { m_timeErrLog = timeErrLog; }

	private:
		void clearStatistics();

		bool moveToNextWriteBuffer();
		bool moveToNextReadBuffer();

	protected:
		// static information
		//
		QVector<int> m_relatedSignalIndexes;
		CircularLoggerShared m_timeErrLog;

		// dynamic state information
		//
		bool m_dataProcessingEnabled = true;
		bool m_receivesData = false;

		qint64 m_uptime = 0;										// in seconds!
		quint32 m_receivedDataID = 0;

		qint64 m_lmTime = 0;
		qint64 m_rupFrameNumerator = -1;			// qint64 is Ok!

		std::atomic<double> m_dataReceivingSpeed = {0};
		std::atomic<qint64> m_receivedDataSize = {0};
		std::atomic<qint64> m_prevReceivedDataSize = {0};
		std::atomic<qint64> m_receivedFramesCount = {0};
		std::atomic<qint64> m_receivedPacketCount = {0};
		std::atomic<qint64> m_lostPacketCount = {0};

		//

		// this values can be changed from AppDataReceiver thread!
		//
		std::atomic<qint64> m_errorProtocolVersion = {0};
		std::atomic<qint64> m_errorFramesQuantity = {0};
		std::atomic<qint64> m_errorFrameNo = {0};
		std::atomic<qint64> m_errorFrameCRC = {0};
		std::atomic<qint64> m_errorDataID = {0};

		//

		qint64 m_errorDuplicatePlantTime = 0;
		qint64 m_errorNonmonotonicPlantTime = 0;
		qint64 m_errorPlantTimeFormat = 0;

		//

		qint64 m_firstPacketServerTime = 0;
		qint64 m_lastPacketServerTime = 0;
		qint32 m_correctionsCount = 0;

		//

		std::atomic<quintptr> m_processingOwner = {0};

		//

		static constexpr int PARSING_BUFFERS_COUNT = 5;

		std::vector<ParsingBuffer*> m_parsingBuffers;

		QueueIndex m_writeBufferIndex = 0;		// modified by packet Receiver only in pushRupFrame
		QueueIndex m_readBufferIndex = 0;		// modified only by ProcessingThread

		SpinLock m_parsingBuffersMutex;		// locks only while m_writeBufferIndex and m_readBufferIndex modyfied

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

		for (const TYPE& ds : dataSources)
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

		for (int i = 0; i < count; i++)
		{
			result &= (*dataSources)[i].readFromXml(xml);
		}

		return result;
	}
} // namespace OnlineLib
