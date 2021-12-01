#pragma once

#include <QUdpSocket>
#include <vector>

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/TuningDataStorage.h"

#include "TuningSource.h"
#include "TuningMemory.h"

namespace Tuning
{
	class TuningSources;

	// ----------------------------------------------------------------------------------
	//
	// SourceStatistics struct declaration
	//
	// ----------------------------------------------------------------------------------

	struct TuningSourceState
	{
		// Tuning Source channel identification
		// only once initialized fields that not require std::atomic<>
		//
		quint64	sourceID;					// generate by DataSource::generateID()
		std::string lanEquipmentID;
		int channel = CHANNEL_1;

		// Tuning Source processing states
		//
		std::atomic<bool> isReply = false;
		std::atomic<qint64> requestCount = 0;
		std::atomic<qint64> replyCount = 0;
		std::atomic<qint32> commandQueueSize = 0;
		std::atomic<bool> controlIsActive = false;
		std::atomic<bool> setSOR = false;
		std::atomic<bool> writingDisabled = false;
		std::atomic<bool> hasUnappliedParams = false;

		// flags reported by LM in reply FotipHeader.flags
		//
		std::atomic<qint64> fotipFlagBoundsCheckSuccess = 0;
		std::atomic<qint64> fotipFlagWriteSuccess = 0;
		std::atomic<qint64> fotipFlagDataTypeErr = 0;
		std::atomic<qint64> fotipFlagOpCodeErr = 0;
		std::atomic<qint64> fotipFlagStartAddrErr = 0;
		std::atomic<qint64> fotipFlagRomSizeErr = 0;
		std::atomic<qint64> fotipFlagRomFrameSizeErr = 0;
		std::atomic<qint64> fotipFlagFrameSizeErr = 0;
		std::atomic<qint64> fotipFlagProtocolVersionErr = 0;
		std::atomic<qint64> fotipFlagSubsystemKeyErr = 0;
		std::atomic<qint64> fotipFlagUniueIDErr = 0;
		std::atomic<qint64> fotipFlagOffsetErr = 0;
		std::atomic<qint64> fotipFlagApplySuccess = 0;
		std::atomic<qint64> fotipFlagSetSOR = 0;
		std::atomic<qint64> fotipFlagWritingDisabled = 0;

		// errors in reply RupFrameHeader
		//
		std::atomic<qint64> errRupProtocolVersion = 0;
		std::atomic<qint64> errRupFrameSize = 0;
		std::atomic<qint64> errRupNonTuningData = 0;
		std::atomic<qint64> errRupModuleType = 0;
		std::atomic<qint64> errRupFramesQuantity = 0;
		std::atomic<qint64> errRupFrameNumber = 0;
		std::atomic<qint64> errRupCRC = 0;

		// errors in reply FotipHeader
		//
		std::atomic<qint64> errFotipProtocolVersion = 0;
		std::atomic<qint64> errFotipUniqueID = 0;
		std::atomic<qint64> errFotipLmNumber = 0;
		std::atomic<qint64> errFotipSubsystemCode = 0;
		std::atomic<qint64> errFotipOperationCode = 0;
		std::atomic<qint64> errFotipFrameSize = 0;
		std::atomic<qint64> errFotipRomSize = 0;
		std::atomic<qint64> errFotipRomFrameSize = 0;
		std::atomic<qint64> errAnalogLowBoundCheck = 0;
		std::atomic<qint64> errAnalogHighBoundCheck = 0;

		// Tuning Source processing errors
		//
		std::atomic<qint64> errUntimelyReplay = 0;
		std::atomic<qint64> errSent = 0;
		std::atomic<qint64> errPartialSent = 0;
		std::atomic<qint64> errReplySize = 0;
		std::atomic<qint64> errNoReply = 0;
		std::atomic<qint64> errTuningFrameUpdate = 0;

		void saveToProto(Network::TuningSourceState* tss) const;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSignal class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSignal
	{
	public:
		void init(const AppSignal* s, int index, int tuningDataFrameSizeW);

		QString appSignalID() const { return m_appSignalID; }

		bool valid() const { return m_valid; }

		E::SignalType signalType() const { return m_signalType; }

		TuningValueType tuningValueType() const { return m_tuningValueType; }
		QString tuningValueTypeStr() const;

		TuningValue currentValue() const { return m_currentValue; }
		TuningValue readLowBound() const { return m_readLowBound; }
		TuningValue readHighBound() const { return m_readHighBound; }
		bool isTuningDefault() const { return m_tuningDefaultFlag; }

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

		FotipV2::DataType fotipV2DataType() const;

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

		// signal state read from LM
		//
		bool m_valid = false;

		TuningValue m_currentValue;
		TuningValue m_readLowBound;
		TuningValue m_readHighBound;

		bool m_tuningDefaultFlag = false;

		//

		bool m_writeInProgress = false;

		qint64 m_successfulReadTime = 0;		// time of last succesfull signal reading (UTC), in normal should be permanently update
		qint64 m_writeRequestTime = 0;			// time of last write request (UTC)
		qint64 m_successfulWriteTime = 0;		// time of last succesfull signal writing (UTC), usually should be near m_writeRequestTime
		qint64 m_unsuccessfulWriteTime = 0;		// time of last unsuccesfull signal writing (UTC), usually should be near m_writeRequestTime

		Hash m_writeClient = 0;									// last write client's EquipmentID hash
		NetworkError m_writeErrorCode = NetworkError::Success;	// last write error code, NetworkError:  Success, TuningValueOutOfRange, TuningNoReply
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningCommand struct declaration
	//
	// ----------------------------------------------------------------------------------

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
			Hash signalHash = 0;

			TuningValue newTuningValue;
		} write;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningCommandQueue class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningCommandQueue : private QList<TuningCommand>
	{
	public:
		void push(const TuningCommand& cmd);
		bool pop(TuningCommand* cmd);

	private:
		QMutex m_mutex;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningChannelHandler class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceThread;

	struct TuningChannelInfo
	{
		QString portEquipmentID;
		int channel = 0;
		HostAddressPort tuningDataIP;
		HostAddressPort tuningSimIP;
	};

	class TuningChannelHandler
	{
	public:
		TuningChannelHandler(TuningSourceThread& srcThread,
							const TuningChannelInfo& channelInfo,
							bool disableModulesTypeChecking,
							E::SoftwareRunMode swRunMode,
							CircularLoggerShared logger,
							CircularLoggerShared tuningLog);
		~TuningChannelHandler();

		HostAddressPort sourceIP() const;
		QString sourceEquipmentID() const;
		int channel() const;

		void startHandler();
		void stopHandler();

		bool isInitialized() const;
		bool isReply() const;
		bool setSOR() const;
		bool writingDisabled() const;

		void periodicProcessing();
		bool processReplyQueue();

		void pushReply(const RupFotipV2& reply);
		void incErrReplySize();

		void getState(Network::TuningSourceState* tuningSourceState);

		void pushTuningCommand(const TuningCommand& tc) { m_tuningCommandQueue.push(tc); }

		const TuningSourceState& state() const { return m_state; }

	private:
		void initTuningSignals(TuningDataSharedConst td);

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

		void onNoReply();

		bool prepareFotipRequest(const TuningCommand& tuningCmd, RupFotipV2& request);
		void sendFotipRequest(SimRupFotipV2& request, const QString& appSignalID);

		bool initRupHeader(Rup::Header& rupHeader);
		bool initFotipFrame(FotipV2::Frame& fotipFrame, const TuningCommand& tuningCmd);

		void processReply(RupFotipV2& reply);
		void processReadReply(RupFotipV2& reply);
		void processWriteReply(RupFotipV2& reply);
		void processApplyReply(RupFotipV2& reply);

		void finalizeWriting(NetworkError errCode);

		bool checkRupHeader(const Rup::Header& rupHeader);
		bool checkFotipHeader(const FotipV2::Header& fotipHeader);

		void invalidateAllSignals();

		void logTuningRequest(const TuningCommand& cmd, QString* appSignalID);
		void logTuningReply(const TuningCommand& cmd, const RupFotipV2& reply);

		TuningSignal* getTuningSignal(Hash signalHash);

	private:
		TuningSourceThread& m_sourceThread;
		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		bool m_disableModulesTypeChecking = false;

		bool m_isSimulationMode = false;

		int m_channel = 0;
		HostAddressPort m_tuningSimIP;

		// data from tuning source
		//
		QString m_moduleEquipmentID;
		QString m_portEquipmentID;
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

		std::atomic<bool> m_isInitialized = {false};

		//

		bool m_waitReply = false;

		const int MAX_WAIT_REPLY_COUNTER = 2;

		int m_waitReplyCounter = 0;

		int m_nextFrameToAutoRead = 0;

		QUdpSocket m_socket;

		SimRupFotipV2 m_request;
		QString m_requestAppSignalID;

		RupFotipV2 m_reply;

		int m_retryCount = 0;

		const int MAX_RETRY_COUNT = 3;

		Queue<RupFotipV2> m_replyQueue;

		TuningCommandQueue m_tuningCommandQueue;

		TuningCommand m_lastProcessedCommand;

		quint16 m_rupNumerator = 0;

		TuningSourceState m_state;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceThread : public RunOverrideThread
	{
	public:
		TuningSourceThread(	const TuningServiceSettings& settings,
							const TuningSource& source,
							E::SoftwareRunMode swRunMode,
							CircularLoggerShared logger,
							CircularLoggerShared tuningLog);

		void pushReply(int channel, const RupFotipV2& reply);
		void incErrReplySize(quint32 channelIP);

		void getSourceState(std::vector<Network::TuningSourceState>* tuningSourcesStates);

		void readSignalState(Network::TuningSignalState* tss) const;

		NetworkError writeSignalState(const QString& clientEquipmentID,
										const QString& user,
										Hash signalHash,
										const TuningValue& newValue);

		NetworkError applySignalStates(	const QString& clientEquipmentID,
										const QString& user);

		QString sourceEquipmentID() const;

		void waitWhileHandlersInitialized() const;

		const TuningSource& source() const { return m_source; }

		const TuningSignal* getTuningSignal(Hash hash) const;
		TuningSignal* getTuningSignal(Hash hash);

		bool updateFrameSignalsState(RupFotipV2& reply);

		bool isSourceHandlerExistsForChannel(int channel) const;

		Network::DataSourceInfo protoDataSourceInfo() const { return m_protoDataSourceInfo; }

	private:
		void run() override;

		void initTuningSignals();
		void initHandlers();
		void shutdownHandlers();

		const TuningChannelHandler* getChannelHandler(int channel) const;
		TuningChannelHandler* getChannelHandler(int channel);
		TuningChannelHandler* getChannelHandlerByIP(quint32 ip);

		void checkChannelsResponse();
		void invalidateAllSignals();

		void checkSetSOR();

		void pushCommandToHandlers(const TuningCommand& cmd, const QString& appSignalID);

		const TuningChannelHandler* privateGetChannelHandler(int channel) const;
		const TuningSignal* privateGetTuningSignal(Hash hash) const;

	private:
		std::vector<TuningChannelInfo> m_tuningChannelsInfo;
		const TuningSource& m_source;
		Network::DataSourceInfo m_protoDataSourceInfo;
		bool m_disableModulesTypeChecking = false;
		E::SoftwareRunMode m_swRunMode = E::SoftwareRunMode::Normal;
		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		TuningDataSharedConst m_tuningData;
		int m_tuningDataOffsetW = 0;
		int m_tuningDataFramePayloadW = 0;
		int m_tuningDataFrameCount = 0;

		//

		mutable QMutex m_handlersMutex;

		std::vector<TuningChannelHandler*> m_handlers;
		std::map<int, TuningChannelHandler*> m_ch2handlers;			// channel => TuningChannelHandler
		std::map<quint32, TuningChannelHandler*> m_ip2handlers;		// source lan IP => TuningChannelHandler

		bool m_anyChannelReply = false;

		std::atomic<bool> m_setSOR = false;
		std::atomic<bool> m_writingDisabled = false;

		//

		std::vector<TuningSignal> m_tuningSignals;
		std::map<Hash, int> m_hash2SignalIndexMap;
		std::vector<std::vector<int>> m_frameSignals;

		TuningMemory m_tuningMem;
	};

	using TuningSourceThreadShared = std::shared_ptr<TuningSourceThread>;

	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListener class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningServiceWorker;

	class TuningSocketListener : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSocketListener(TuningServiceWorker& service,
							 const HostAddressPort& listenIP,
							 int channel,
							 bool simulationMode,
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

		void pushReplyToTuningSource(const QHostAddress& tuningSourceIP, const RupFotipV2& reply);
		void incErrReplySizeOfTuningSource(const QHostAddress& tuningSourceIP);

	private:
		TuningServiceWorker& m_service;
		HostAddressPort m_listenIP;
		int m_channel = 0;
		bool m_simMode = false;

		std::shared_ptr<CircularLogger> m_logger;

		QTimer m_timer;

		QUdpSocket* m_socket = nullptr;

		// statistics
		//
		qint64 m_errReplySize = 0;
		qint64 m_errReadSocket = 0;
		qint64 m_errUnknownTuningSource = 0;
		qint64 m_errSimVersion = 0;
		qint64 m_errNotExpectedSimPacket = 0;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListenerThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListenerThread : public SimpleThread
	{
	public:
		TuningSocketListenerThread(TuningServiceWorker& service,
								   const HostAddressPort& listenIP,
								   int channel,
								   bool simulationMode,
								   std::shared_ptr<CircularLogger> logger);
		~TuningSocketListenerThread();

	private:
		int m_channel = 0;
		TuningSocketListener* m_socketListener = nullptr;
	};
}
