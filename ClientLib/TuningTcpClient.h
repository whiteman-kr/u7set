#pragma once

#include <queue>
#include <QReadWriteLock>
#include "../Proto/network.pb.h"
#include "../AppSignalLib/TuningSignalManager.h"
#include "../CommonLib/Hash.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/LogFile.h"
#include "TuningSourceState.h"
#include "TuningLog.h"

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

		Hash m_appSignalHash = 0;
		Hash m_equipmentHash;		// Used only for activation/deactivation LM
		TuningValue m_value;

		TuningWriteCommandType m_type = TuningWriteCommandType::WriteValue;

		bool m_enableControl = false;
		bool m_forceTakeControl = false;

		// Write constructor
		//
		TuningWriteCommand(const QString& appSignalId, const TuningValue& value) :
			TuningWriteCommand(::calcHash(appSignalId), value)
		{
		}

		TuningWriteCommand(Hash hash, const TuningValue& value)
		{
			m_type = TuningWriteCommandType::WriteValue;

			m_appSignalHash = hash;
			m_value = value;
		}

		// Apply constructor
		//
		TuningWriteCommand(bool apply)
		{
			Q_UNUSED(apply);
			m_type = TuningWriteCommandType::Apply;
		}

		// Activate LM constructor
		//
		TuningWriteCommand(Hash equipmentHash, bool enableControl, bool forceTakeControl)
		{
			m_type = TuningWriteCommandType::ActivateLm;

			m_equipmentHash = equipmentHash;
			m_enableControl = enableControl;
			m_forceTakeControl = forceTakeControl;
		}

		// Serializing

		bool save(Network::TuningWriteCommand* message) const;
		bool load(const Network::TuningWriteCommand& message);
	};


	class TuningTcpClient : public Tcp::Client, public TcpClientStatistics
	{
		Q_OBJECT

		Q_ENUM(NetworkError)

	public:
		TuningTcpClient(const SoftwareInfo& softwareInfo,
						const SoftwareEndpoint::TuningService& tunsInfo,
						TuningSignalManager& signalManager,
						ILogFile* log,
						TuningLog::TuningLog* tuningLog);

		virtual ~TuningTcpClient();

	public:
		// Tuning sources
		//
		std::vector<Hash> tuningSourcesHashes() const;
		std::vector<TuningSource> tuningSourcesInfo() const;
		bool tuningSourceInfo(Hash equipmentHash, TuningSource* result) const;
		bool hasTuningSource(Hash equipmentHash) const;

		bool activateTuningSourceControl(Hash equipmentHash, bool enableControl, bool forceTakeControl);

		// Searching signals
		//
		bool hasTuningSignals(const std::vector<Hash> appSignalHashes) const;	// Returns true if client processes at least one of specified signals
		bool hasTuningSignal(Hash appSignalHash) const;
		bool hasTuningSignal(QString appSignalId) const;

		// Writing states
		//
		void writeTuningSignal(const std::vector<TuningWriteCommand>& data);
		void writeTuningSignal(const TuningWriteCommand& data);

		// ITuningTcpClient implementation
		//
	public:
		virtual bool writeTuningSignal(QString appSignalId, TuningValue value);
		// Apply states
		//
		void applyTuningSignals();

		// Reading state
		//
		TuningSignalState state(Hash hash, bool* found) const;

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

		void writeLogAlert(const QString& message);
		void writeLogError(const QString& message);
		void writeLogWarning(const QString& message);
		void writeLogMessage(const QString& message);

		void writeTuningLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue);
		void writeTuningLogMessage(const QString& message);

	public slots:
		void slot_signalsUpdated();

	signals:
		void tuningSourcesInfoArrived();

	private:
		QString networkErrorStr(NetworkError error);

		// Properties
		//
	public:
		QString instanceId() const;
		void setInstanceId(const QString& instanceId);

		Hash instanceIdHash() const;

		int requestInterval() const;
		void setRequestInterval(int requestInterval);

		bool autoApply() const;
		void setAutoApply(bool value);

		QString tuningServiceId() const;
		void setTuningServiceId(const QString& tuningServiceId);

		// LM Control functions

		bool singleLmControlMode() const;

		bool clientIsActive() const;	// Returns if this client is selected as active in connected TuningService
		QString activeClientId() const;
		QString activeClientIp() const;

		Hash activeTuningSource() const;

		// SOR/Key mode flag

		TuningClientSettings::LmStatusFlagMode lmStatusFlagMode() const;
		void setLmStatusFlagMode(const TuningClientSettings::LmStatusFlagMode& mode);

		// Data
		//
	private:
		HasLogFile m_logFile;
		TuningLog::TuningLog* m_tuningLog = nullptr;

		QString m_instanceId;
		Hash m_instanceIdHash;
		int m_requestInterval = 100;
		bool m_autoApply = true;
		QString m_tuningServiceId;

		TuningClientSettings::LmStatusFlagMode m_lmStatusFlagMode = TuningClientSettings::LmStatusFlagMode::SOR;

		TuningSignalManager& m_signals;

		mutable QReadWriteLock m_statesLocker;				// For access to m_states
		std::map<Hash, TuningSignalState> m_states;

	protected:
		// Tuning sources
		//
		mutable QReadWriteLock m_tuningSourcesLock;				// For access to m_tuningSources, m_equipmentToSignalMap
		std::map<Hash, TuningSource> m_tuningSources;		// Key is hash of EquipmentID

		mutable QReadWriteLock m_signalHashesLock;			// For access to m_signalHashes and m_signalHashesSet
		std::vector<Hash> m_signalHashes;					// SORTED Hash Vector for iterating all processed signals
		std::unordered_set<Hash> m_signalHashesSet;			// Hash Table for fast checking if signal is processed by this client

	private:
		// Processing
		//
		mutable QMutex m_writeQueueMutex;					// For access to m_writeQueue
		std::queue<TuningWriteCommand> m_writeQueue;

		int m_readTuningSignalIndex = 0;
		int m_readTuningSignalCount = 0;

		mutable QReadWriteLock m_activeClientMutex;				// For access to m_activeClientId, m_activeClientIp, m_singleLmControlMode, m_currentClientIsActive
		QString m_activeClientId;
		QString m_activeClientIp;
		bool m_currentClientIsActive = false;

		bool m_singleLmControlMode = false;

	private:
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
