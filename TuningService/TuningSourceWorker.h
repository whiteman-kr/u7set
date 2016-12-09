#pragma once

#include <QUdpSocket>

#include "../lib/SimpleThread.h"
#include "../lib/ServiceSettings.h"

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

		void get(Network::TuningSourceState& tss);
	};

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	private:

		struct Read
		{
			quint32 frame;
		};

		struct Write
		{
			quint32 frame;
		};

		struct Apply
		{
			quint32 frame;
		};

		struct TuningCommand
		{
			FotipV2::OpCode opCode = FotipV2::OpCode::Read;

			Read read;
			Write write;
			Apply apply;
		};

		enum class TuningSignalType
		{
			AnalogFloat = 0,
			AnalogInt = 1,
			Discrete = 2,
		};

		struct TuningSignal
		{
			// static fields
			//
			bool valid = false;
			double value = 0;

			// static fields
			//
			Signal* signal = nullptr;
			Hash signalHash = 0;
			TuningSignalType type = TuningSignalType::Discrete;
			int index = -1;

			int offset = -1;
			int bit = -1;
			int frameNo = -1;

			// signal properties from RPCT Databse
			//
			double lowBound = 0;
			double highBoud = 0;
			double defaultValue = 0;

			// signal properties read from LM
			//
			double readLowBound = 0;
			double readHighBound = 0;
		};

	public:
		TuningSourceWorker(const TuningServiceSettings& settings,
						   const TuningSource& source);
		~TuningSourceWorker();

		quint32 sourceIP() const;
		QString sourceEquipmentID() const;

		void pushReply(const Rup::Frame& reply);
		void incErrReplySize();

		void getState(Network::TuningSourceState& tuningSourceState);
		void getSignalState(Network::TuningSignalState& tss);

	signals:
		void replyReady();

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void initTuningSignals(const TuningData* td);
		TuningSignalType getTuningSignalType(Signal* s) const;

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

		void prepareFOTIPRequest(const TuningCommand& tuningCmd, RupFotipV2& request);
		void sendFOTIPRequest(RupFotipV2& request);

		void initRupHeader(Rup::Header& rupHeader);
		void initFotipHeader(FotipV2::Header& fotipHeader, const TuningCommand& tuningCmd);
		void initFotipData(FotipV2::Frame& fotip, const TuningCommand& tuningCmd);

		void processReply(RupFotipV2& reply);
		void processReadReply(RupFotipV2& reply);
		void processWriteReply(RupFotipV2& reply);
		void processApplyReply(RupFotipV2& reply);

		bool checkRupHeader(const Rup::Header& rupHeader);
		bool checkFotipHeader(const FotipV2::Header& fotipHeader);

		void restartTimer();

	private slots:
		void onTimer();
		void onReplyReady();

	private:

		// data from tuning source
		//
		QString m_sourceEquipmentID;
		HostAddressPort m_sourceIP;

		quint64 m_sourceUniqueID = 0;
		quint16 m_lmNumber = 0;
		quint16 m_subsystemCode = 0;

		int m_tuningRomStartAddrW = 0;
		int m_tuningRomFrameCount = 0;
		int m_tuningRomFrameSizeW = 0;
		int m_tuningRomSizeW = 0;

		//

		QVector<TuningSignal> m_tuningSignals;
		QHash<Hash, int> m_hash2SignalIndexMap;

		//

		bool m_waitReply = false;

		const int MAX_WAIT_REPLY_COUNTER = 3;

		int m_waitReplyCounter = 0;

		int m_nextFrameToAutoRead = 0;

		int m_timerInterval = 10;
		QTimer m_timer;

		QUdpSocket m_socket;

		RupFotipV2 m_request;
		RupFotipV2 m_reply;

		int m_retryCount = 0;

		const int MAX_RETRY_COUNT = 3;

		Queue<Rup::Frame> m_replyQueue;

		Queue<TuningCommand> m_tuningCommandQueue;

		quint16 m_rupNumerator = 0;

		TuningMemory m_tuningMem;

		// statisticts
		//
		SourceStatistics m_stat;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorkerThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceWorkerThread : public SimpleThread
	{
	public:
		TuningSourceWorkerThread(const TuningServiceSettings& settings, const TuningSource& source);
		~TuningSourceWorkerThread();

		quint32 sourceIP();

		TuningSourceWorker* worker();

		void pushReply(const Rup::Frame& reply);
		void incErrReplySize();

	private:
		TuningSourceWorker* m_sourceWorker = nullptr;
	};


	typedef QHash<quint32, TuningSourceWorkerThread*> TuningSourceWorkerThreadMap;


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListener class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListener : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSocketListener(const HostAddressPort& listenIP, TuningSourceWorkerThreadMap& sourceWorkerMap);
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
		TuningSourceWorkerThreadMap& m_sourceWorkerMap;

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
		TuningSocketListenerThread(const HostAddressPort& listenIP, TuningSourceWorkerThreadMap& sourceWorkerMap);
		~TuningSocketListenerThread();

	private:
		TuningSocketListener* m_socketListener = nullptr;
	};
}
