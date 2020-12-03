#pragma once
#include "SimScopedLog.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/TimeStamp.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataProtocols.h"
#include "../lib/LogicModulesInfo.h"
#include "SimRam.h"

namespace Sim
{
	class Simulator;

	struct TuningRecord
	{
		QString lmEquipmentId;
		QString portEquipmentId;

		union
		{
			struct
			{
				quint16 value;
				quint16 mask;
			} word;
			quint32 dword;
			qint32 signedInt32;
			float float32;
		} data;

		E::ByteOrder dataByteOrder;

		static TuningRecord createWord(const QString& lmEquipmentId,
									   const QString& portEquipmentId,
									   quint16 value,
									   quint16 mask)
		{
			TuningRecord r{lmEquipmentId, portEquipmentId, {}, E::ByteOrder::BigEndian};
			r.data.word.value = value;
			r.data.word.mask = mask;
			return r;
		}

		static TuningRecord createDword(const QString& lmEquipmentId,
										const QString& portEquipmentId,
										quint32 value)
		{
			TuningRecord r{lmEquipmentId, portEquipmentId, {}, E::ByteOrder::BigEndian};
			r.data.dword = value;
			return r;
		}

		static TuningRecord createSignedInt32(const QString& lmEquipmentId,
											  const QString& portEquipmentId,
											  qint32 value)
		{
			TuningRecord r{lmEquipmentId, portEquipmentId, {}, E::ByteOrder::BigEndian};
			r.data.signedInt32 = value;
			return r;
		}

		static TuningRecord createFloat(const QString& lmEquipmentId,
											  const QString& portEquipmentId,
											  float value)
		{
			TuningRecord r{lmEquipmentId, portEquipmentId, {}, E::ByteOrder::BigEndian};
			r.data.float32 = value;
			return r;
		}
	};

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
		//
		// Data is in LogicMoudule's native endianness (BE).
		//
		bool updateTuningRam(const QString& lmEquipmentId,
							 const QString& portEquipmentId,
							 const RamArea& ramArea,
							 TimeStamp timeStamp);

		// This function is called by Simulator when module's tuning mode has changed to or from TuningMode
		//
		void tuningModeChanged(const QString& lmEquipmentId, bool tuningMode);

	public:
		void writeTuningWord(const QString& lmEquipmentId, const QString& portEquipmentId, quint16 data, quint16 mask);
		void writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 data);
		void writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, qint32 data);
		void writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, float data);

		std::queue<TuningRecord> fetchWriteTuningQueue(const QString& lmEquipmentId);
	private:
		void writeTuningRecord(TuningRecord&& r);

		void startProcessingThread();
		void stopProcessingThread();

	protected slots:
		void projectUpdated();					// Project was loaded or cleared

	public:
		bool enabled() const;
		void setEnabled(bool value);

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

		void tuningModeChanged(const QString& lmEquipmentId, bool tuningEnabled);

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
		std::map<QString, std::shared_ptr<TuningSourceHandler>> m_tuningSourcesByEquipmentID;
	};

	class TuningSourceHandler
	{
	public:
		TuningSourceHandler(TuningServiceCommunicator* tsCommunicator,
							  const QString& equipmentID,
							  const HostAddressPort& ip,
							  const ::LogicModuleInfo& logicModuleInfo);

		virtual ~TuningSourceHandler();

		void updateTuningData(const RamArea& data, TimeStamp timeStamp);

		void tuningModeChanged(bool tuningEnabled);

		bool processRequest(const RupFotipV2& request, RupFotipV2* reply);

	private:
		bool checkRequestRupHeader(const Rup::Header& rupHeader);
		bool checkRequestFotipHeader(const FotipV2::Header& fotipHeader, FotipV2::HeaderFlags* replyFlags);

		void processReadRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);
		void processWriteRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);
		void processApplyRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags);

	private:
		TuningServiceCommunicator* m_tsCommunicator = nullptr;
		QString m_equipmentID;
		HostAddressPort m_tuningSourceIP;
		int m_moduleType = 0;
		int m_lmNumber = -1;
		int m_subsystemKey = -1;
		quint64 m_lmUniqueID = 0;

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


