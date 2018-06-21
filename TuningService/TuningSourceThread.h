#pragma once

#include <QUdpSocket>

#include "../lib/SimpleThread.h"
#include "../lib/ServiceSettings.h"
#include "../lib/CircularLogger.h"

#include "TuningSource.h"
#include "TuningMemory.h"

namespace Tuning
{
	class TuningSources;

	struct SourceStatistics
	{
		quint64 dataSourceID;		// generate by DataSource::generateID()

		bool isReply = false;

		qint64 requestCount = 0;
		qint64 replyCount = 0;

		qint32 commandQueueSize = 0;

		qint64 errUntimelyReplay = 0;
		qint64 errSent = 0;
		qint64 errPartialSent = 0;
		qint64 errReplySize = 0;
		qint64 errNoReply = 0;
		qint64 errRupCRC = 0;

		// errors in reply RupFrameHeader
		//
		qint64 errRupProtocolVersion = 0;
		qint64 errRupFrameSize = 0;
		qint64 errRupNonTuningData = 0;
		qint64 errRupModuleType = 0;
		qint64 errRupFramesQuantity = 0;
		qint64 errRupFrameNumber = 0;

		// errors in reply FotipHeader
		//
		qint64 errFotipProtocolVersion = 0;
		qint64 errFotipUniqueID = 0;
		qint64 errFotipLmNumber = 0;
		qint64 errFotipSubsystemCode = 0;
		qint64 errFotipOperationCode = 0;
		qint64 errFotipFrameSize = 0;
		qint64 errFotipRomSize = 0;
		qint64 errFotipRomFrameSize = 0;

		// errors reported by LM in reply FotipHeader.flags
		//
		qint64 fotipFlagBoundsCheckSuccess = 0;
		qint64 fotipFlagWriteSuccess = 0;
		qint64 fotipFlagDataTypeErr = 0;
		qint64 fotipFlagOpCodeErr = 0;
		qint64 fotipFlagStartAddrErr = 0;
		qint64 fotipFlagRomSizeErr = 0;
		qint64 fotipFlagRomFrameSizeErr = 0;
		qint64 fotipFlagFrameSizeErr = 0;
		qint64 fotipFlagProtocolVersionErr = 0;
		qint64 fotipFlagSubsystemKeyErr = 0;
		qint64 fotipFlagUniueIDErr = 0;
		qint64 fotipFlagOffsetErr = 0;
		qint64 fotipFlagApplySuccess = 0;
		qint64 fotipFlagSetSOR = 0;

		qint64 errAnalogLowBoundCheck = 0;
		qint64 errAnalogHighBoundCheck = 0;

		bool controlIsActive = false;
		bool setSOR = false;
		bool hasUnappliedParams = false;

		void get(Network::TuningSourceState& tss);
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceHandler class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceHandler
	{
	private:

		struct TuningCommand
		{
			QString clientEquipmentID;
			QString user;

			FotipV2::OpCode opCode = FotipV2::OpCode::Read;
			bool autoCommand = false;

			struct
			{
				quint32 frame = 0;
			} read;

			struct
			{
				qint32 signalIndex = 0;

				TuningValue newTuningValue;
			} write;
		};

		class TuningSignal
		{
		public:
			void init(const Signal* s, int index, int tuningDataFrameSizeW);

			QString appSignalID() const { return m_appSignalID; }

			bool valid() const { return m_valid; }

			E::SignalType signalType() const { return m_signalType; }

			TuningValueType tuningValueType() const { return m_tuningValueType; }

			TuningValue currentValue() const { return m_currentValue; }
			TuningValue readLowBound() const { return m_readLowBound; }
			TuningValue readHighBound() const { return m_readHighBound; }

			TuningValue defaultValue() const { return m_defaultValue; }
			TuningValue lowBound() const { return m_lowBound; }
			TuningValue highBound() const { return m_highBound; }

			int offset() const { return m_offset; }
			int bit() const { return m_bit; }
			int frameNo() const { return m_frameNo; }

			void updateCurrentValue(bool valid, const TuningValue& value, qint64 time);

			void setCurrentValue(bool valid, const TuningValue& value);
			void setReadLowBound(const TuningValue& value);
			void setReadHighBound(const TuningValue& value);
			void invalidate();

			bool writeInProgress() const { return m_writeInProgress; }

			void setWriteClient(const QString& clientEquipmentID) { m_writeClient = calcHash(clientEquipmentID); }
			void setWriteInProgress(bool inProgress) { m_writeInProgress = inProgress; }
			void setWriteRequestTime(qint64 writeRequestTime) { m_writeRequestTime = writeRequestTime; }
			void setSuccessfulWriteTime(qint64 writeTime) { m_successfulWriteTime = writeTime; }
			void setUnsuccessfulWriteTime(qint64 writeTime) { m_unsuccessfulWriteTime = writeTime; }

			qint64 successfulReadTime() const { return m_successfulReadTime; }
			qint64 writeRequestTime() const { return m_writeRequestTime; }
			qint64 successfulWriteTime() const { return m_successfulWriteTime; }
			qint64 unsuccessfulWriteTime() const { return m_unsuccessfulWriteTime; }

			Hash writeClient() const { return m_writeClient; }

			NetworkError writeErrorCode() const { return m_writeErrorCode; }
			void setWriteErrorCode(NetworkError errCode) { m_writeErrorCode = errCode; }
			void resetWriteErrorCode() { setWriteErrorCode(NetworkError::Success); }

			FotipV2::DataType fotipV2DataType();

		private:
			void updateTuningValuesType(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat);

		private:
			QString m_appSignalID;
			Hash m_signalHash = 0;
			E::SignalType m_signalType = E::SignalType::Discrete;
			E::AnalogAppSignalFormat m_analogFormat = E::AnalogAppSignalFormat::SignedInt32;

			int m_index = -1;

			int m_offset = -1;
			int m_bit = -1;
			int m_frameNo = -1;

			TuningValueType m_tuningValueType = TuningValueType::Discrete;

			// signal properties from RPCT Databse
			//
			TuningValue m_lowBound;
			TuningValue m_highBound;
			TuningValue m_defaultValue;

			// tuning signal state and bounds from LM
			//
			bool m_valid = false;

			TuningValue m_currentValue;
			TuningValue m_readLowBound;
			TuningValue m_readHighBound;

			bool m_writeInProgress = false;

			qint64 m_successfulReadTime = 0;		// time of last succesfull signal reading (UTC), in normal should be permanently update
			qint64 m_writeRequestTime = 0;			// time of last write request (UTC)
			qint64 m_successfulWriteTime = 0;		// time of last succesfull signal writing (UTC), usually should be near m_writeRequestTime
			qint64 m_unsuccessfulWriteTime = 0;		// time of last unsuccesfull signal writing (UTC), usually should be near m_writeRequestTime

			Hash m_writeClient = 0;									// last write client's EquipmentID hash
			NetworkError m_writeErrorCode = NetworkError::Success;	// last write error code, NetworkError:  Success, TuningValueOutOfRange, TuningNoReply
		};

	public:
		TuningSourceHandler(const TuningServiceSettings& settings,
						   const TuningSource& source,
						   CircularLoggerShared logger,
						   CircularLoggerShared tuningLog,
							QObject* parent);
		~TuningSourceHandler();

		quint32 sourceIP() const;
		QString sourceEquipmentID() const;

		TuningSourceHandler* handler() { return this; }

		void pushReply(const Rup::Frame& reply);
		void incErrReplySize();

		void getState(Network::TuningSourceState& tuningSourceState);

		void readSignalState(Network::TuningSignalState* tss) const;

		NetworkError writeSignalState(	const QString& clientEquipmentID,
										const QString& user,
										Hash signalHash,
										const TuningValue& newValue);

		NetworkError applySignalStates(	const QString& clientEquipmentID,
										const QString& user);
	protected:
		void initHandler();
		void shutdownHandler();

		Queue<Rup::Frame>& replyQueue() { return m_replyQueue; }

		virtual void restartTimer() {}

		void periodicProcessing();
		bool processReplyQueue();

	private:
		void initTuningSignals(const TuningData* td);

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

		void onNoReply();

		bool prepareFotipRequest(const TuningCommand& tuningCmd, RupFotipV2& request);
		void sendFotipRequest(RupFotipV2& request);

		bool initRupHeader(Rup::Header& rupHeader);
		bool initFotipFrame(FotipV2::Frame& fotipFrame, const TuningCommand& tuningCmd);

		void processReply(RupFotipV2& reply);
		void processReadReply(RupFotipV2& reply);
		void processWriteReply(RupFotipV2& reply);
		void processApplyReply(RupFotipV2& reply);

		void updateFrameSignalsState(RupFotipV2& reply);
		void finalizeWriting(NetworkError errCode);

		bool checkRupHeader(const Rup::Header& rupHeader);
		bool checkFotipHeader(const FotipV2::Header& fotipHeader);

		void invalidateAllSignals();

		void logTuningRequest(const TuningCommand& cmd);
		void logTuningReply(const TuningCommand& cmd, const RupFotipV2& reply);

	private:
		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		bool m_disableModulesTypeChecking = false;

		// data from tuning source
		//
		QString m_sourceEquipmentID;
		HostAddressPort m_sourceIP;

		quint64 m_sourceUniqueID = 0;
		quint16 m_lmNumber = 0;
		quint16 m_lmModuleType = 0;
		quint16 m_subsystemCode = 0;

		int m_tuningFlashSizeB = 0;
		int m_tuningFlashFramePayloadB = 0;

		int m_tuningDataOffsetW = 0;
		int m_tuningDataFrameCount = 0;
		int m_tuningDataFramePayloadW = 0;
		int m_tuningUsedFramesCount = 0;

		//

		QVector<TuningSignal> m_tuningSignals;
		QHash<Hash, int> m_hash2SignalIndexMap;
		QVector<QVector<int>> m_frameSignals;

		//

		bool m_waitReply = false;

		const int MAX_WAIT_REPLY_COUNTER = 2;

		int m_waitReplyCounter = 0;

		int m_nextFrameToAutoRead = 0;

		QUdpSocket m_socket;

		RupFotipV2 m_request;
		RupFotipV2 m_reply;

		int m_retryCount = 0;

		const int MAX_RETRY_COUNT = 3;

		Queue<Rup::Frame> m_replyQueue;

		QueueOnList<TuningCommand> m_tuningCommandQueue;

		TuningCommand m_lastProcessedCommand;

		quint16 m_rupNumerator = 0;

		TuningMemory m_tuningMem;

		// statisticts
		//
		SourceStatistics m_stat;

		// debug vars
		//
		qint64 m_timerCount = 0;
		qint64 m_waitReplyFalseCount = 0;
		qint64 m_waitReplyTrueCount = 0;
		qint64 m_processIdleTrueCount = 0;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceThread class declaration (QThread::run override)
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceRunOverrideThread : public RunOverrideThread, public TuningSourceHandler
	{
	public:
		TuningSourceRunOverrideThread(	const TuningServiceSettings& settings,
										const TuningSource& source,
										CircularLoggerShared logger,
										CircularLoggerShared tuningLog);
	private:
		void run() override;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker class declaration (signal-slot, event driven)
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceWorker : public SimpleThreadWorker, public TuningSourceHandler
	{
		Q_OBJECT

	public:
		TuningSourceWorker(const TuningServiceSettings& settings,
						   const TuningSource& source,
						   CircularLoggerShared logger,
						   CircularLoggerShared tuningLog);
	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		virtual void restartTimer() override;

	private slots:
		void onTimer();
		void onReplyReady();

	private:
		int m_timerInterval = 10;
		QTimer m_timer;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorkerThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceWorkerThread : public SimpleThread
	{
	public:
		TuningSourceWorkerThread(const TuningServiceSettings& settings,
								 const TuningSource& source,
								 CircularLoggerShared logger,
								 CircularLoggerShared tuningLog);

		~TuningSourceWorkerThread();

		quint32 sourceIP();

		TuningSourceWorker* worker();

		void pushReply(const Rup::Frame& reply);
		void incErrReplySize();

		TuningSourceHandler* handler();

	private:
		TuningSourceWorker* m_sourceWorker = nullptr;
	};

	//
	// Different implementations of TuningSourceThread
	//

	//	typedef TuningSourceWorkerThread TuningSourceThread;

	typedef TuningSourceRunOverrideThread TuningSourceThread;

	//

	typedef QHash<quint32, TuningSourceThread*> TuningSourceThreadMap;

	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListener class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListener : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSocketListener(const HostAddressPort& listenIP,
							 TuningSourceThreadMap& sourceWorkerMap,
							 std::shared_ptr<CircularLogger> logger);
		~TuningSocketListener();

	signals:

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void createSocket();
		void closeSocket();

		void startTimer();

	private slots:
		void onTimer();

		void onSocketReadyRead();

		void pushReplyToTuningSourceWorker(const QHostAddress& tuningSourceIP, const Rup::Frame& reply);
		void incSourceWorkerErrReplySize(const QHostAddress& tuningSourceIP);

	private:
		HostAddressPort m_listenIP;
		TuningSourceThreadMap& m_sourceThreadMap;

		std::shared_ptr<CircularLogger> m_logger;

		QTimer m_timer;

		QUdpSocket* m_socket = nullptr;

		// statistics
		//
		qint64 m_errReplySize = 0;
		qint64 m_errReadSocket = 0;
		qint64 m_errUnknownTuningSource = 0;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListenerThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListenerThread : public SimpleThread
	{
	public:
		TuningSocketListenerThread(const HostAddressPort& listenIP,
								   TuningSourceThreadMap& sourceWorkerMap,
								   std::shared_ptr<CircularLogger> logger);
		~TuningSocketListenerThread();

	private:
		TuningSocketListener* m_socketListener = nullptr;
	};
}
