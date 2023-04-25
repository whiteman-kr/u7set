#pragma once

#include <queue>
#include <QReadWriteLock>
#include "../Proto/network.pb.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../CommonLib/Hash.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "TuningSourceState.h"
#include "ITuningLog.h"

//
//		TDS_GET_TUNING_SOURCES_INFO
//				|
//		TDS_GET_TUNING_SOURCES_STATES <-------+
//              |                             |
//		TDS_TUNING_SIGNALS_READ               |
//				|						      |
//		TDS_TUNING_SIGNALS_WRITE?             |
//				|						      |
//		TDS_TUNING_SIGNALS_APPLY?             |
//				|						      |
//		TDS_CHANGE_CONTROLLED_TUNING_SOURCE?  |
//				+-----------------------------+
//

namespace ClientLib
{
	struct TuningWriteCommand
	{
		enum class TuningWriteCommandType
		{
			WriteValue,
			Apply,
			ActivateLm
		};

		// Data

		Hash appSignalHash = 0;
		Hash equipmentHash;		// Used only for activation/deactivation LM
		TuningValue value;

		TuningWriteCommandType type = TuningWriteCommandType::WriteValue;

		bool enableControl = false;
		bool forceTakeControl = false;

		// Write constructor
		//
		TuningWriteCommand(const QString& appSignalId, const TuningValue& value) :
			TuningWriteCommand(::calcHash(appSignalId), value)
		{
		}

		TuningWriteCommand(Hash appSignalHash, const TuningValue& value)
		{
			type = TuningWriteCommandType::WriteValue;
			this->appSignalHash = appSignalHash;
			this->value = value;
		}

		// Apply constructor
		//
		TuningWriteCommand(bool apply)
		{
			Q_UNUSED(apply);
			this->type = TuningWriteCommandType::Apply;
		}

		// Activate LM constructor
		//
		TuningWriteCommand(Hash equipmentHash, bool enableControl, bool forceTakeControl)
		{
			type = TuningWriteCommandType::ActivateLm;
			this->equipmentHash = equipmentHash;
			this->enableControl = enableControl;
			this->forceTakeControl = forceTakeControl;
		}

		// Serializing
		//
		bool toProtoWriteCommand(Network::TuningWriteCommand* message) const;
	};


	class TuningTcpClient : public Tcp::Client, public TcpClientStatistics
	{
		Q_OBJECT

	public:
		TuningTcpClient(const SoftwareInfo& softwareInfo,
						const SoftwareEndpoint::TuningService& tunsInfo,
						ITuningSignalUpdater& signalUpdater,
						ILogFile* log,
						ITuningLog* tuningLog);

		virtual ~TuningTcpClient();

	public:
		Hash tuningServiceHash() const;

		// Tuning sources
		//
		std::vector<Hash> tuningSourcesHashes() const;
		std::vector<TuningSource> tuningSourcesInfo() const;
		bool tuningSourceInfo(Hash equipmentHash, TuningSource* result) const;
		bool hasTuningSource(Hash equipmentHash) const;

		bool activateTuningSourceControl(Hash equipmentHash, bool enableControl, bool forceTakeControl);

		// Searching signals
		//
		bool hasTuningSignals(const std::vector<Hash>& appSignalHashes) const;	// Returns true if client processes at least one of specified signals
		bool hasTuningSignal(Hash appSignalHash) const;

		// Writing states
		//
		void writeTuningSignal(const std::vector<TuningWriteCommand>& data);

		// Apply states
		//
		void applyTuningSignals();

	private:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	protected:
		void resetToGetTuningSources();
		void resetToGetTuningSourcesState();
		void resetToProcessTuningSignals();

		void requestTuningSourcesInfo();
		void processTuningSourcesInfo(const QByteArray& data);

		void requestTuningSourcesState();
		void processTuningSourcesState(const QByteArray& data);

		void requestActivateTuningSource(Hash equipmentHash, bool enableControl, bool forceTakeControl);
		void processActivateTuningSource(const QByteArray& data);

		void requestReadTuningSignals();
		void processReadTuningSignals(const QByteArray& data);

		void requestWriteTuningSignals();
		void processWriteTuningSignals(const QByteArray& data);

		void requestApplyTuningSignals();
		void processApplyTuningSignals(const QByteArray& data);

	public slots:
		void reset();

	signals:
		void tuningSourcesInfoArrived();

		// Properties
		//
	public:
		int requestInterval() const;
		void setRequestInterval(int requestInterval);

		bool autoApply() const;
		void setAutoApply(bool value);

		// LM Control functions

		bool singleLmControlMode() const;

		bool clientIsActive() const;	// Returns if this client is selected as active in connected TuningService
		QString activeClientId() const;
		QString activeClientIp() const;

		Hash activeTuningSource() const;

		// SOR/Key mode flag

		TuningClientSettings::LmStatusFlagMode lmStatusFlagMode() const;
		void setLmStatusFlagMode(const TuningClientSettings::LmStatusFlagMode& mode);

	protected:
		// Tuning sources
		//
		mutable QReadWriteLock m_tuningSourcesLock;				// For access to m_tuningSources, m_equipmentToSignalMap
		std::map<Hash, TuningSource> m_tuningSources;		// Key is hash of EquipmentID

		// Tuning signals hashes
		//
		mutable QReadWriteLock m_signalHashesLock;			// For access to m_signalHashes and m_signalHashesSet
		std::vector<Hash> m_signalHashes;					// SORTED Hash Vector for iterating all processed signals
		std::unordered_set<Hash> m_signalHashesSet;			// Hash Table for fast checking if signal is processed by this client

	private:
		// Data
		//
		HasLogFile m_logFile;
		ITuningLog* m_tuningLog = nullptr;

		const Hash m_tuningServiceHash = UNDEFINED_HASH;

		int m_requestInterval = 100;
		bool m_autoApply = true;
		bool m_singleLmControlMode = false;
		TuningClientSettings::LmStatusFlagMode m_lmStatusFlagMode = TuningClientSettings::LmStatusFlagMode::SOR;

		ITuningSignalUpdater& m_signalUpdater;

		// Write processing
		//
		mutable QMutex m_writeQueueMutex;					// For access to m_writeQueue
		std::queue<TuningWriteCommand> m_writeQueue;

		// Reading processing
		//
		int m_readTuningSignalIndex = 0;
		int m_readTuningSignalCount = 0;

		// Active client processing
		//
		mutable QReadWriteLock m_activeClientMutex;				// For access to m_activeClientId, m_activeClientIp, m_singleLmControlMode, m_currentClientIsActive
		QString m_activeClientId;
		QString m_activeClientIp;
		bool m_currentClientIsActive = false;

		// Cached protobuf messages
		//
		::Network::GetTuningSourcesStates m_getTuningSourcesStates;
		::Network::GetTuningSourcesStatesReply m_tuningSourcesStatesReply;

		::Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
		::Network::GetTuningSourcesInfoReply m_tuningSourcesInfoReply;

		::Network::ChangeConrolledTuningSourceRequest m_activateTuningSource;
		::Network::ChangeConrolledTuningSourceReply m_activateTuningSourceReply;

		::Network::TuningSignalsRead m_readTuningSignals;
		::Network::TuningSignalsReadReply m_readTuningSignalsReply;

		::Network::TuningSignalsWrite m_writeTuningSignals;
		::Network::TuningSignalsWriteReply m_writeTuningSignalsReply;

		::Network::TuningSignalsApply m_applyTuningSignals;
		::Network::TuningSignalsApplyReply m_applyTuningSignalsReply;
	};

}
