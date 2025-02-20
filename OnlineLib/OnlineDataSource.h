#pragma once

#include <CommonLib/Hash.h>
#include "DataSource.h"

class BaseOnlineDataSource : public OnlineLib::DataSource
{
private:
	static const int APP_DATA_SOURCE_TIMEOUT = 500;

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
	BaseOnlineDataSource(const DataSource& dataSource, E::LanControllerType srcType);
	virtual ~BaseOnlineDataSource();

	bool initParsingBuffers(int framesQuantity);
	void clearParsingBuffers();

	// Functions used by receiver thread
	//
	bool pushRupFrame(quint32 sourceIP,
					  qint64 serverTime,
					  bool isSimFrame,
					  Rup::Frame& rupFrame,
					  const QThread* thread);

	bool updateStatistics_500ms(bool oneSecond);

	// Functions used by data processing thread
	//
	bool parseNextBuffer(const QThread* thread);

	virtual bool parseRupData(	const Times& time,
								bool isSimPacket,
								quint16 packetNo,
								const char* rupData,
								int rupDataSize,
								const QThread* thread) = 0;

	virtual bool invalidateAllSignals(const QThread* thread) = 0;

	void checkPlantTime(const Rup::TimeStamp& plantTimeStamp);

	bool takeProcessingOwnership(const QThread* processingThread);
	bool releaseProcessingOwnership(const QThread* processingThread);

	//

	E::LanControllerType sourceType() const { return m_sourceType; }
	int acquiredSignalsCount() const { return m_acquiredSignalsCount; }

	QString stateStr() const;

	bool dataProcessingEnabled() const { return m_dataProcessingEnabled; }
	bool receivesData() const { return m_receivesData; }
	qint64 uptime() const { return m_uptime; }
	double dataReceivingSpeed() const { return m_dataReceivingSpeed; }
	qint64 receivedDataSize() const { return m_receivedDataSize; }
	qint64 receivedFramesCount() const { return m_receivedFramesCount; }
	qint64 receivedPacketCount() const { return m_receivedPacketCount; }
	quint32 receivedDataID() const { return m_receivedDataUID; }
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

	static QString getTimeStr(qint64 timeMs);
	static QString getTimeStr(const Rup::TimeStamp& ts);

	// Used by PacketViewer
	//
	void addSignalIndex(int index) { m_relatedSignalIndexes.append(index); }
	const QVector<int>& signalIndexes() const { return m_relatedSignalIndexes; }

	void setTimeErrLog(CircularLoggerShared timeErrLog) { m_timeErrLog = timeErrLog; }

private:
	void clearStatistics();

	bool moveToNextWriteBuffer(const QThread* thread);
	bool moveToNextReadBuffer(const QThread* thread);

protected:
	E::LanControllerType m_sourceType = E::LanControllerType::Unknown;
	quint32 m_expectedDataUID = 0;
	int m_acquiredSignalsCount = 0;

	// static information
	//
	QVector<int> m_relatedSignalIndexes;
	CircularLoggerShared m_timeErrLog;

	// dynamic state information
	//
	bool m_dataProcessingEnabled = true;
	bool m_receivesData = false;

	qint64 m_uptime = 0;										// in seconds!
	quint32 m_receivedDataUID = 0;

	qint64 m_lmTime = 0;
	qint64 m_rupFrameNumerator = -1;			// qint64 is Ok!

	std::atomic<double> m_dataReceivingSpeed = { 0 };
	std::atomic<qint64> m_receivedDataSize = { 0 };
	std::atomic<qint64> m_prevReceivedDataSize = { 0 };
	std::atomic<qint64> m_receivedFramesCount = { 0 };
	std::atomic<qint64> m_receivedPacketCount = { 0 };
	std::atomic<qint64> m_lostPacketCount = { 0 };

	//

	// this values can be changed from AppDataReceiver thread!
	//
	std::atomic<qint64> m_errorProtocolVersion = { 0 };
	std::atomic<qint64> m_errorFramesQuantity = { 0 };
	std::atomic<qint64> m_errorFrameNo = { 0 };
	std::atomic<qint64> m_errorFrameCRC = { 0 };
	std::atomic<qint64> m_errorDataID = { 0 };

	//

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

	QueueIndex m_writeBufferIndex = 0;		// modified by RupReceiver only in pushRupFrame
	QueueIndex m_readBufferIndex = 0;		// modified only by parsing thread

	SimpleMutex m_parsingBuffersMutex;		// locks only while m_writeBufferIndex and m_readBufferIndex modyfied

	// result variables

	Times m_rupTimes;
	Times m_lastRupTimes;
	quint16 m_packetNo = 0;
	int m_rupDataSize = 0;
};

template <typename SIGNAL_STATE>
class OnlineDataSource : public BaseOnlineDataSource
{
protected:
	OnlineDataSource(const DataSource& dataSource, E::LanControllerType srcType) :
		BaseOnlineDataSource(dataSource, srcType),
		m_states(3)
	{
		if (acquiredSignalsCount() > 0)
		{
			m_states.resize(acquiredSignalsCount() * 3);
		}
	}

	void pushSignalState(const SIGNAL_STATE& state, const QThread* thread)
	{
		m_states.push(state, thread);
	}

public:
	int popStates(SIGNAL_STATE* signalStatesBuffer, int bufferSize, const QThread* thread)
	{
		Q_UNUSED(signalStatesBuffer);
		Q_UNUSED(bufferSize);
		Q_UNUSED(thread);
		Q_ASSERT(false);			// to do
		return 0;
	}

private:
	FastThreadSafeQueue<SIGNAL_STATE> m_states;
};


