#pragma once
#include "SimScopedLog.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/TimeStamp.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataProtocols.h"
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
		TuningServiceCommunicator(Simulator* simulator, const TuningServiceSettings& settings);
		virtual ~TuningServiceCommunicator();

	public:
		bool startSimulation();
		bool stopSimulation();

		Simulator* simulator() const;

		// This function is called by Simulator to provide current RAM state of Tuning memory area
		// if sLM is in TuningMode and Tuning is enabled.
		// Data is in LogicMoudule's native endianness (BE).
		//
		bool updateTuningRam(const QString& lmEquipmentId,
							 const QString& portEquipmentId,
							 const RamArea& ramArea,
							 TimeStamp timeStamp);

		// This function is called by Simulator to provide confiramtion about writing data to RAM
		//
		void writeConfirmation(std::vector<qint64> confirmedRecords,
							   const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   TimeStamp timeStamp);	// timeStamp can be the same with following updateTuningRam call (writeConfiramtion is called before workcycle, updateTuningRam after workcyle, both already have the same timestamp)

		// These functions are called by Simulator when module enters or leaves tuning mode
		//
		void tuningModeEntered(const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   TimeStamp timeStamp);

		void tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId);

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

		void startProcessingThread();
		void stopProcessingThread();

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool softwareEnabled() const;			// Global enable for all LogicModules AppData LANs

		// Data Section
		//
	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		::TuningServiceSettings m_settings;

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
		TuningRequestsProcessingThread(TuningServiceCommunicator* tsCommunicator, const TuningServiceSettings& settings);
		virtual ~TuningRequestsProcessingThread() override;

		void updateTuningData(const QString& lmEquipmentID,
							  const QString& portEquipmentID,
							  const RamArea& data,
							  TimeStamp timeStamp);

		void tuningModeEntered(const QString& lmEquipmentId,
							   const QString& portEquipmentId,
							   const RamArea& ramArea,
							   TimeStamp timeStamp);

		void tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId);

	private:
		virtual void run() override;

		void initTuningSourcesHandlers(const TuningServiceSettings& settings);

		bool tryCreateAndBindSocket();
		void closeSocket();

		void receiveRequests();

	private:
		TuningServiceCommunicator* m_tsCommunicator = nullptr;
		Simulator* m_sim = nullptr;
		ScopedLog& m_log;

		QString m_tuningServiceEquipmentID;
		HostAddressPort m_tuningRequestsReceivingIP;
		HostAddressPort m_tuningRepliesSendingIP;

		const QThread* m_thisThread = nullptr;
		QUdpSocket* m_socket = nullptr;

		std::map<quint32, std::shared_ptr<TuningSourceHandler>> m_tuningSourcesByIP;
		std::map<std::pair<QString, QString>, std::shared_ptr<TuningSourceHandler>> m_tuningSourcesByEquipmentID;
	};

	class TuningSourceHandler
	{
	public:
		TuningSourceHandler(TuningServiceCommunicator* tsCommunicator,
							  const QString& lmEquipmentID,
							  const QString& portEquipmentID,
							  const HostAddressPort& ip,
							  const ::LogicModuleInfo& logicModuleInfo);

		virtual ~TuningSourceHandler();

		void updateTuningData(const RamArea& data, TimeStamp timeStamp);

		void tuningModeEntered(const RamArea& ramArea, TimeStamp timeStamp);
		void tuningModeLeft();

		bool processRequest(const RupFotipV2& request, RupFotipV2* reply);

		QString lmEquipmentID() const { return m_lmEquipmentID; }

	private:


	private:
		bool checkRequestRupHeader(const Rup::Header& rupHeader);
		bool checkRequestFotipHeader(const FotipV2::Header& fotipHeader, FotipV2::HeaderFlags* replyFlags);

		void processReadRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);
		void processWriteRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);
		void processApplyRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);

		void readFrameData(quint32 startFrameAddrW, FotipV2::Frame* reply);

	private:
		TuningServiceCommunicator* m_tsCommunicator = nullptr;
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

		std::vector<quint8> m_tuningDataReadBuffer;
	};
}


