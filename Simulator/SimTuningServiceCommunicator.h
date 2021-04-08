#pragma once
#include "SimScopedLog.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/TimeStamp.h"
#include "../lib/SimpleThread.h"
#include "../OnlineLib/DataProtocols.h"
#include "../lib/LogicModulesInfo.h"
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
		void writeConfirmation(std::vector<qint64> confirmedRecords,
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

		TuningRequestsProcessingThread* m_processingThread = nullptr;

		// Queue to write data to LogicModule
		// Key is LM EquipmentID
		//
		QMutex m_qmutex;
		std::map<QString, std::queue<TuningRecord>> m_writeTuningQueue;
	};

	class TuningSourceHandler;

	class TuningRequestsProcessingThread : public RunOverrideThread
	{
	public:
		TuningRequestsProcessingThread(TuningServiceCommunicator& tsCommunicator,
									   const QString& curProfileName,
									   ScopedLog& log);

		virtual ~TuningRequestsProcessingThread() override;

		void updateTuningData(const QString& lmEquipmentID,
							  const QString& portEquipmentID,
							  const RamArea& data,
							  bool setSorChassisState,
							  TimeStamp timeStamp);

		void writeConfirmation(const QString& lmEquipmentID,
							   const QString& portEquipmentID,
							   const std::vector<qint64>& confirmedRecords,
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

		void finalizeAndSendReply(quint32 tuningSourceIP, SimRupFotipV2 &reply);

		void cancelTuningSourceHandlersOperations();

	private:
		struct WriteConfirmation
		{
			QString lmEquipmentID;
			QString portEquipmentID;
			std::vector<qint64> confirmedRecordsIDs;

			WriteConfirmation()
			{
			}

			WriteConfirmation(const QString& lmID, const QString& portID, const std::vector<qint64>& ids) :
				lmEquipmentID(lmID),
				portEquipmentID(portID),
				confirmedRecordsIDs(ids)
			{
			}
		};

	private:
		TuningServiceCommunicator& m_tsCommunicator;
		QString m_curProfileName;
		Simulator& m_sim;
		ScopedLog& m_log;

		HostAddressPort m_tuningRequestsReceivingIP;
		HostAddressPort m_tuningRepliesSendingIP;

		const QThread* m_thisThread = nullptr;
		QUdpSocket* m_socket = nullptr;

		qint64 m_lastRequestTime = 0;
		SimRupFotipV2 m_request;
		SimRupFotipV2 m_reply;

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
		bool writeConfirmation(const std::vector<qint64>& confirmationIDs, RupFotipV2* reply);

		void tuningModeEntered(const RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp);
		void tuningModeLeft();

		bool processRequest(const RupFotipV2& request, RupFotipV2* nowReply);

		void cancelOperations();

		QString lmEquipmentID() const { return m_lmEquipmentID; }
		quint32 tuningSourceIP() const { return m_tuningSourceIP.address32(); }

	private:
		bool checkRequestRupHeader(const Rup::Header& rupHeader);
		bool checkRequestFotipHeader(const FotipV2::Header& requestFotipHeader, FotipV2::HeaderFlags* replyFlags);

		void processReadRequest(const FotipV2::Frame& request,
								FotipV2::Frame* reply, bool*
								sendReplyImmediately);

		void processWriteRequest(const FotipV2::Frame& request,
								 FotipV2::Frame* reply,
								 bool* sendReplyImmediately);

		void processApplyRequest(bool* sendReplyImmediately);

		void readFrameData(quint32 startFrameAddrW, FotipV2::Frame* reply);

	private:
		TuningServiceCommunicator& m_tsCommunicator;
		QString m_lmEquipmentID;
		QString m_portEquipmentID;
		HostAddressPort m_tuningSourceIP;
		int m_moduleType = 0;
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

		RupFotipV2 m_delayedReply;
	};
}


