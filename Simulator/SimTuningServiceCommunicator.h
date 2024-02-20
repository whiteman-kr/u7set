#pragma once
#include "SimScopedLog.h"
#include "../CommonLib/Times.h"
#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../HardwareLib/LogicModulesInfo.h"
#include "SimRam.h"
#include "SimTuningRecord.h"

namespace Sim
{
	class Simulator;
	class TuningRequestsProcessingThread;


	class TuningServiceCommunicator : public QObject
	{
		Q_OBJECT

	public:
		TuningServiceCommunicator(Simulator* simulator, const QString& tuningServiceEquipmentID);
		virtual ~TuningServiceCommunicator();

	public:
		bool startSimulation(QString profileName);
		bool stopSimulation();

		Simulator* simulator() const;

		// This function is called by Simulator to provide current RAM state of Tuning memory area
		// if sLM is in TuningMode and Tuning is enabled.
		// Data is in LogicMoudule's native endianness (BE).
		//
		bool updateTuningRam(const QString& lmEquipmentId,
							 const QString& portEquipmentId,
							 const RamArea& ramArea,
							 bool setSorChassisState,
							 TimeStamp timeStamp);

		// This function is called by Simulator to provide confiramtion about writing data to RAM
		//
		void writeConfirmation(qint64 confirmedRecordID,
							   const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   bool setSorChassisState,
							   TimeStamp timeStamp);	// timeStamp can be the same with following updateTuningRam call (writeConfiramtion is called before workcycle, updateTuningRam after workcyle, both already have the same timestamp)

		// These functions are called by Simulator when module enters or leaves tuning mode
		//
		void tuningModeEntered(const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   bool setSorChassisState,
							   TimeStamp timeStamp);

		void tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId);

		QString tuningServiceEquipmentID() const;

	public:
		// Write command to LM's RAM
		// Save returned qint64 (RecordIndex) for confirmation
		//
		qint64 applyWrittenChanges(const QString& lmEquipmentId, const QString& portEquipmentId);
		qint64 writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, quint32 data, quint32 mask);
		qint64 writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, qint32 data);
		qint64 writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, float data);

		std::queue<TuningRecord> fetchWriteTuningQueue(const QString& lmEquipmentId);

		ScopedLog& log();

	private:
		qint64 writeTuningRecord(TuningRecord&& r);

		void startProcessingThread(const QString& curProfileName);
		void stopProcessingThread();

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool softwareEnabled() const;			// Global enable for all LogicModules AppData LANs

		// Data Section
		//
	private:
		Simulator* m_simulator = nullptr;
		const QString m_tuningServiceEquipmentID;
		mutable ScopedLog m_log;

		std::atomic<bool> m_enabled{true};		// Allow communication to TuningService

		std::vector<TuningRequestsProcessingThread*> m_processingThreads;

		// Queue to write data to LogicModule
		// Key is LM EquipmentID
		//
		QMutex m_qmutex;
		std::map<QString, std::queue<TuningRecord>> m_writeTuningQueue;
	};

	class FotipProcessingNumeratorsMap
	{
	public:
		void appendNumerator(const QString& lmEquipmentID);
		quint64 getNextFotipProcessingNumerator(const QString& lmEquipmentID);

	private:
		QMutex m_mapMutex;
		std::map<QString, quint64> m_fotipProcessingNumeratorsMap;			// lmEquipmentID -> fotipProcessingNumerator
	};

	class TuningSourceHandler;

	class TuningRequestsProcessingThread : public RunOverrideThread
	{
	public:
		TuningRequestsProcessingThread(TuningServiceCommunicator& tsCommunicator,
									   const QString& curProfileName,
									   std::shared_ptr<const TuningServiceSettings> settings,
									   int channel,
									   ScopedLog& log);

		virtual ~TuningRequestsProcessingThread() override;

		void updateTuningData(const QString& lmEquipmentID,
							  const QString& portEquipmentID,
							  const RamArea& data,
							  bool setSorChassisState,
							  TimeStamp timeStamp);

		void writeConfirmation(const QString& lmEquipmentID,
							   const QString& portEquipmentID,
							   qint64 confirmedRecordID,
							   const RamArea& ramArea,
							   bool setSorChassisState,
							   TimeStamp timeStamp);

		void tuningModeEntered(const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   bool setSorChassisState,
							   TimeStamp timeStamp);

		void tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId);

	private:
		virtual void run() override;

		void initTuningSourcesHandlers(const TuningServiceSettings& settings);

		std::shared_ptr<TuningSourceHandler> getTuningSourceHandler(const QString& lmEquipmentID,
																	const QString& portEquipmentID);

		std::shared_ptr<TuningSourceHandler> getTuningSourceHandler(quint32 tuningSourceIP);

		bool tryCreateAndBindSocket();
		void closeSocket();

		void receiveRequests();

		bool processWriteConfirmations();
		bool processRequests();

		void finalizeAndSendReply(quint32 tuningSourceIP, SimRupFotip &reply);

		void cancelTuningSourceHandlersOperations();

		void logWarningThinned(int codeLine, const QString& warning);

	private:
		struct WriteConfirmation
		{
			QString lmEquipmentID;
			QString portEquipmentID;
			qint64 confirmedRecordID{};

			WriteConfirmation()
			{
			}

			WriteConfirmation(const QString& lmID, const QString& portID, qint64 id) :
				lmEquipmentID(lmID),
				portEquipmentID(portID),
				confirmedRecordID(id)
			{
			}
		};

	private:
		TuningServiceCommunicator& m_tsCommunicator;
		QString m_curProfileName;
		int m_channel = -1;
		Simulator& m_sim;
		ScopedLog& m_log;

		QString m_controllerEquipmentID;
		HostAddressPort m_tuningRequestsReceivingIP;
		HostAddressPort m_tuningRepliesSendingIP;

		const QThread* m_thisThread = nullptr;
		QUdpSocket* m_socket = nullptr;

		qint64 m_lastRequestTime = 0;
		SimRupFotip m_request;
		SimRupFotip m_reply;

		std::map<quint32, std::shared_ptr<TuningSourceHandler>> m_tuningSourcesByIP;
		std::map<std::pair<QString, QString>, std::shared_ptr<TuningSourceHandler>> m_tuningSourcesByEquipmentID;

		QMutex m_queueMutex;
		std::queue<WriteConfirmation> m_writeConfirmationQueue;
	};

	class TuningSourceHandler
	{
	public:
		TuningSourceHandler(TuningServiceCommunicator& tsCommunicator,
							  const QString& lmEquipmentID,
							  const QString& portEquipmentID,
							  const HostAddressPort& ip,
							  const ::LogicModuleInfo& logicModuleInfo);

		virtual ~TuningSourceHandler();

		void updateTuningData(const RamArea& data, bool setSorChassisState, TimeStamp timeStamp);
		bool writeConfirmation(qint64 confirmationID, RupFotip* reply);

		void tuningModeEntered(const RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp);
		void tuningModeLeft();

		bool processRequest(const RupFotip& request, RupFotip* nowReply);

		void cancelOperations();

		QString lmEquipmentID() const { return m_lmEquipmentID; }
		quint32 tuningSourceIP() const { return m_tuningSourceIP.address32(); }

	private:
		bool checkRequestRupHeader(const Rup::Header& rupHeader);
		bool checkRequestFotipHeader(const Fotip::Header& requestFotipHeader, Fotip::HeaderFlags* replyFlags);

		void processReadRequest(const Fotip::Frame& request,
								Fotip::Frame* reply, bool*
								sendReplyImmediately);

		void processWriteRequest(const Fotip::Frame& request,
								 Fotip::Frame* reply,
								 bool* sendReplyImmediately);

		void processApplyRequest(bool* sendReplyImmediately);

		void readFrameData(quint32 startFrameAddrW, Fotip::Frame* reply);

		void setFotipProcessingNumerator(RupFotip* reply);

	private:
		TuningServiceCommunicator& m_tsCommunicator;
		ScopedLog& m_log;
		QString m_lmEquipmentID;
		QString m_portEquipmentID;
		HostAddressPort m_tuningSourceIP;
		int m_moduleType = 0;
		int m_rupVersion = Rup::V5;
		int m_fotipVersion = Fotip::V2;
		int m_lmNumber = -1;
		int m_subsystemKey = -1;
		quint64 m_lmUniqueID = 0;

		quint32 m_tuningFlashSizeB = 0;
		quint32 m_tuningFlashFramePayloadB = 0;

		quint32 m_tuningDataStartAddrW = 0;
		quint32 m_tuningDataSizeW = 0;
		quint32 m_tuningDataSizeB = 0;
		quint32 m_tuningDataFrameSizeW = 0;
		quint32 m_tuningDataFramePayloadW = 0;
		quint32 m_tuningDataFramePayloadB = 0;

		//

		std::atomic<bool> m_tuningEnabled = { false };

		QMutex m_tuningDataMutex;
		std::shared_ptr<RamArea> m_tuningData;
		std::atomic<bool> m_setSorChassisState = { false };

		std::vector<quint8> m_tuningDataReadBuffer;

		// delayed reply processing
		//
		std::optional<qint64> m_waitingConfirmationID;
		int m_receivedConfirmationsCount = 0;

		RupFotip m_delayedReply{};

		static FotipProcessingNumeratorsMap m_processingNumeratorsMap;	// one map for all TuningSourceHandlers
																		// each entry in map according to one real LM
	};
}


