#pragma once

#include <QUdpSocket>
#include <vector>
#include <queue>

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../AppSignalLib/TuningDataStorage.h"

#include "TuningSource.h"
#include "TuningMemory.h"
#include "TuningSignal.h"

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
		quint64	sourceID{};					// generate by DataSource::generateID()
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
		std::atomic<qint64> lmTime = 0;

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
		std::atomic<quint64> fotipProcessingNumerator = 0;

		// errors in reply RupFrameHeader
		//
		std::atomic<qint64> errRupProtocolVersion = 0;
		std::atomic<qint64> errTimeStamp = 0;
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
	// SafeTuningValue class declaration
	//
	// ----------------------------------------------------------------------------------

	class SafeTuningValue
	{
	public:
		SafeTuningValue();
		SafeTuningValue(const SafeTuningValue& stv);

		SafeTuningValue& operator = (const TuningValue& tv);
		bool operator == (const SafeTuningValue& stv) const;
		bool operator == (const TuningValue& stv) const;

		TuningValueType type() const;
		void setType(TuningValueType t);

		TuningValue tuningValue() const;

	private:
		mutable SimpleMutex m_mutex;
		TuningValue m_value;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningCommand struct declaration
	//
	// ----------------------------------------------------------------------------------

	struct TuningCommand
	{
		TuningCommand();

		QString clientEquipmentID;
		QString matsUser;

		Fotip::OpCode opCode = Fotip::OpCode::Read;
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

		quint64 commandID() const { return m_commandID; }
		void resetCommandID() { m_commandID = 0; }

	private:
		quint64 m_commandID = 0;

		static std::atomic<quint64> m_globalCommandID;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningCommandQueue class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningCommandQueue
	{
	public:
		void push(const TuningCommand& cmd);
		bool pop(TuningCommand* cmd);

	private:
		QMutex m_mutex;
		std::queue<TuningCommand> m_queue;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningChannelHandler class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceThreadWorker;

	struct TuningChannelInfo
	{
		QString portEquipmentID;
		int channel = 0;
		HostAddressPort tuningDataIP;
		HostAddressPort tuningSimIP;
	};

	class TuningChannelHandler
	{
		//
		// Code of both (all) TuningChannelHandlers of one TuningSource sequantally executing
		// in context of one TuningSourceWorkerThread, so no any Mutexes is required
		// on changing data of TuningSourceWorker
		//

	public:
		TuningChannelHandler(TuningSourceThreadWorker& srcThread,
							int rupVersion,
							int fotipVersion,
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

		void run();

		bool isInitialized() const;
		bool isReply() const;
		bool setSOR() const;
		bool writingDisabled() const;

		void pushReply(const RupFotip& reply);
		void incErrReplySize();

		void getState(Network::TuningSourceState* tuningSourceState);

		void pushTuningCommand(const TuningCommand& tc) { m_tuningCommandQueue.push(tc); }

		const TuningSourceState& state() const { return m_state; }

		void stopCommandProcessing(const TuningCommand& cmd, int srcChannel, bool hasUnappliedParams);

	private:

		bool processWaitReply();
		void processUntimelyReply();
		bool processCommandQueue();
		bool enqueueTuningReadCommand();

		void onNoReply();

		bool prepareFotipRequest(const TuningCommand& tuningCmd, RupFotip& request);
		void sendFotipRequest(SimRupFotip& request, const QString& appSignalID, bool retry);

		bool initRupHeader(Rup::Header& rupHeader);
		bool initFotipFrame(Fotip::Frame& fotipFrame, const TuningCommand& tuningCmd);

		void processReply(RupFotip& reply);
		bool processReadReply(RupFotip& reply);
		bool processWriteReply(RupFotip& reply);
		bool processApplyReply(RupFotip& reply);

		void finalizeWriting(E::NetworkError errCode);

		bool checkRupHeader(const Rup::Header& rupHeader);
		bool checkFotipHeader(const Fotip::Header& fotipHeader);

		void invalidateAllSignals();

		void logTuningRequest(const TuningCommand& cmd, QString* appSignalID, quint16 requestNumerator);
		void logTuningReply(const TuningCommand& cmd, const RupFotip& reply, quint16 requestNumerator);

		TuningSignalShared getTuningSignal(Hash signalHash);

		QString toHex(quint16 v) const { return (QString("%1").arg(v, 4, 16, Latin1Char::ZERO)).toUpper();}

	private:
		TuningSourceThreadWorker& m_sourceThread;
		int m_rupVersion = Rup::V5;
		int m_fotipVersion = Fotip::V2;
		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		bool m_disableModulesTypeChecking = false;

		bool m_isSimulationMode = false;

		int m_channel = CHANNEL_1;
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

		const int REPLY_TIMEOUT_MS = 50;
		const int PAUSE_BEFORE_NEXT_REQUEST_MS = 2; // +2..3ms delay

		const int MAX_RETRY_COUNT = 3;

		bool m_waitReply = false;
		qint64 m_lastRequestTime = 0;
		qint64 m_lastReplyTime = 0;
		int m_retryCount = 0;

		//

		int m_nextFrameToAutoRead = 0;

		QUdpSocket m_socket;

		SimRupFotip m_request;
		QString m_requestAppSignalID;

		RupFotip m_reply;

		FastThreadSafeQueue<RupFotip> m_replyQueue;

		TuningCommandQueue m_tuningCommandQueue;

		TuningCommand m_lastProcessedCommand;
		std::set<quint64> m_alreadyProcessedCommands;

		quint16 m_rupNumerator = 0;
		quint64 m_fotipRequestNumerator = 0;

		TuningSourceState m_state;
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningServiceWorker;

	class TuningSourceThreadWorker : public SimpleThreadWorker
	{
	public:
		TuningSourceThreadWorker(TuningServiceWorker& service,
								const TuningServiceSettings& settings,
								const TuningSource& source,
								E::SoftwareRunMode swRunMode,
								CircularLoggerShared logger,
								CircularLoggerShared tuningLog);

		void onThreadStarted() override;
		void onThreadFinished() override;

		void timerEvent(QTimerEvent* event) override;

		void pushReply(int channel, const RupFotip& reply);
		void incErrReplySize(quint32 channelIP);

		void getSourceState(Network::GetTuningSourcesStatesReply* reply);

		void readSignalState(Network::TuningSignalState* tss) const;

		E::NetworkError writeSignalState(const QString& clientEquipmentID,
										const QString& matsUser,
										Hash signalHash,
										const TuningValue& newValue);

		E::NetworkError applySignalStates(const QString& clientEquipmentID,
										const QString& matsUser);

		QString sourceEquipmentID() const;

		void waitWhileHandlersInitialized() const;

		bool isSourceHandlerExistsForChannel(int channel) const;

		const TuningSource& source() const { return m_source; }

		TuningSignalConstShared getTuningSignal(Hash hash) const;
		TuningSignalShared getTuningSignal(Hash hash);

		bool updateFrameSignalsState(RupFotip& reply);

		Network::DataSourceInfo protoDataSourceInfo() const { return m_protoDataSourceInfo; }

		TuningServiceWorker& service() { return m_service; }

		void stopCommandProcessing(const TuningCommand& cmd, int srcChannel, bool hasUnappliedParams);

	private:
		void initTuningSignals();

		void initHandlers();
		void shutdownHandlers();

		void initTimer();
		void shutdownTimer();

		const TuningChannelHandler* getChannelHandler(int channel) const;
		TuningChannelHandler* getChannelHandler(int channel);
		TuningChannelHandler* getChannelHandlerByIP(quint32 ip);

		void runHandlers();
		void checkChannelsResponse();
		void invalidateAllSignals();

		void checkSetSOR();

		void pushCommandToHandlers(const TuningCommand& cmd, const QString& appSignalID);

		const TuningChannelHandler* privateGetChannelHandler(int channel) const;
		TuningSignalShared privateGetTuningSignal(Hash hash) const;

	private:
		TuningServiceWorker& m_service;

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

		QThread* m_thread = nullptr;

		//

		QBasicTimer* m_timer = nullptr;

		//

		mutable QMutex m_handlersMutex;

		std::vector<TuningChannelHandler*> m_handlers;
		std::map<int, TuningChannelHandler*> m_ch2handlers;			// channel => TuningChannelHandler
		std::map<quint32, TuningChannelHandler*> m_ip2handlers;		// source lan IP => TuningChannelHandler

		bool m_anyChannelReply = false;

		std::atomic<bool> m_setSOR = false;
		std::atomic<bool> m_writingDisabled = false;
		quint64 m_fotipProcessingNumerator = 0;

		//

		std::vector<TuningSignalShared> m_tuningSignals;			//
		std::map<Hash, int> m_hash2SignalIndexMap;
		std::vector<std::vector<int>> m_frameSignals;

		TuningMemory m_tuningMem;
	};

	class TuningSourceThread : public SimpleThread
	{
	public:
		TuningSourceThread(TuningServiceWorker &service,
						   const TuningServiceSettings& settings,
							const TuningSource& source,
							E::SoftwareRunMode swRunMode,
							CircularLoggerShared logger,
							CircularLoggerShared tuningLog);

		void pushReply(int channel, const RupFotip& reply);
		void incErrReplySize(quint32 channelIP);
		void getSourceState(Network::GetTuningSourcesStatesReply* reply);
		void readSignalState(Network::TuningSignalState* tss) const;

		E::NetworkError writeSignalState(const QString& clientEquipmentID,
										const QString& matsUser,
										Hash signalHash,
										const TuningValue& newValue);

		E::NetworkError applySignalStates(	const QString& clientEquipmentID,
										const QString& user);

		QString sourceEquipmentID() const;
		void waitWhileHandlersInitialized() const;
		bool isSourceHandlerExistsForChannel(int channel) const;

	private:
		TuningSourceThreadWorker* m_worker = nullptr;
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
	public:
		TuningSocketListener(TuningServiceWorker& service,
							 const HostAddressPort& listenIP,
							 int channel,
							 bool simulationMode,
							 std::shared_ptr<CircularLogger> logger);
		~TuningSocketListener();

	private:
		void onThreadStarted() override;
		void onThreadFinished() override;

		void timerEvent(QTimerEvent* event) override;

		void initTimer();
		void shutdownTimer();

		void createSocket();
		void closeSocket();
		bool readSocket();

		void pushReplyToTuningSource(const QHostAddress& tuningSourceIP, const RupFotip& reply);
		void incErrReplySizeOfTuningSource(const QHostAddress& tuningSourceIP);

	private:
		TuningServiceWorker& m_service;
		HostAddressPort m_listenIP;
		int m_channel = CHANNEL_1;
		bool m_simMode = false;

		std::shared_ptr<CircularLogger> m_logger;

		//

		QBasicTimer* m_timer = nullptr;
		QUdpSocket* m_socket = nullptr;
		qint64 m_socketCreateLastTime = 0;

		// statistics
		//
		qint64 m_errReplySize = 0;
		qint64 m_errReadSocket = 0;
		qint64 m_errUnknownTuningSource = 0;
		qint64 m_errSimVersion = 0;
		qint64 m_errNotExpectedSimPacket = 0;
	};

	class TuningSocketListenerThread : public SimpleThread
	{
	public:
		TuningSocketListenerThread(TuningServiceWorker& service,
								const HostAddressPort& listenIP,
								int channel,
								bool simulationMode,
								std::shared_ptr<CircularLogger> logger);
	};

}
