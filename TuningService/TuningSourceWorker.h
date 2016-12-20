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

		qint64 errAnalogLowBoundCheck = 0;
		qint64 errAnalogHighBoundCheck = 0;

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

		struct TuningCommand
		{
			FotipV2::OpCode opCode = FotipV2::OpCode::Read;

			struct
			{
				quint32 frame = 0;
			} read;

			struct
			{
				qint32 signalIndex = 0;
				float value = 0;
			} write;
		};

		class TuningSignal
		{
		public:
			void init(const Signal* s, int index, int tuningRomFraeSizeW);

			bool valid() const { return m_valid; }
			float value() const { return m_value; }
			float defaultValue() const { return m_defaultValue; }
			float readLowBound() const { return m_readLowBound; }
			float readHighBound() const { return m_readHighBound; }

			int offset() const { return m_offset; }
			int bit() const { return m_bit; }
			int frameNo() const { return m_frameNo; }

			FotipV2::DataType type() const { return m_type; }

			void setState(bool valid, float value);
			void setReadLowBound(float readLowBound);
			void setReadHighBound(float readHighBound);

		private:
			FotipV2::DataType getTuningSignalType(const Signal* s);

		private:
			// state fields
			//
			bool m_valid = false;
			float m_value = 0;

			// static fields
			//
			const Signal* m_signal = nullptr;
			Hash m_signalHash = 0;
			FotipV2::DataType m_type = FotipV2::DataType::Discrete;
			int m_index = -1;

			int m_offset = -1;
			int m_bit = -1;
			int m_frameNo = -1;

			// signal properties from RPCT Databse
			//
			float m_lowBound = 0;
			float m_highBoud = 0;
			float m_defaultValue = 0;

			// signal properties read from LM
			//
			float m_readLowBound = 0;
			float m_readHighBound = 0;
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

		void readSignalState(Network::TuningSignalState& tss);
		void writeSignalState(Hash signalHash, float value, Network::TuningSignalWriteResult& writeResult);
		void applySignalStates();

	signals:
		void replyReady();

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

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
		void processApplyReply(RupFotipV2&);

		bool checkRupHeader(const Rup::Header& rupHeader);
		bool checkFotipHeader(const FotipV2::Header& fotipHeader);

		void restartTimer();

		void invalidateAllSignals();

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

		const int MAX_WAIT_REPLY_COUNTER = 2;

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
