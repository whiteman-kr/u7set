#pragma once

#include <QReadWriteLock>
#include <queue>
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"

#include <ClientLib/ITuningAuthorization.h>
#include <ClientLib/IRecentAppSignals.h>
#include <ClientLib/ITuningLog.h>
#include <ClientLib/ITuningSignalUpdater.h>
#include <ClientLib/TuningSourceState.h>
#include <ClientLib/TuningWriteCommand.h>

//
//		  OnConnection
//				|
//		TDS_GET_TUNING_SOURCES_INFO
//				|
//				|<----------------------------------------------------------------------o
//              |																		|
//              |																		|
//          Wait 100ms                              									|
//           or until																	|
//			WriteQueue --->----Yes---->---WriteCommand-->---Yes----o					|
//			 has data?				  ^  is WriteValue?            |					|
//				|					  |	      |			TDS_TUNING_SIGNALS_WRITE		|
//			    No					  |	      No				  o-------------------->|
//              |					  |		  |											|
//				|					  |       | 										|
//	 TDS_GET_TUNING_SOURCES_STATES    |       |											|
//				|					  |		  |											|
//				|					  |		  |											|
//			WriteQueue --->----Yes----o		  |											|
//			 has data?				  |		  |											|
//				|					  |		  |											|
//              |<-----------------o  |	      |											|
//				|                  |  |	      |											|
//	 TDS_GET_SIGNALS_STATE_CHANGES |  |	WriteCommand-->---Yes-----o						|
//				|				   |  |	   is Apply?			  |						|
//			 Still more	than	   |  |	      |			TDS_TUNING_SIGNALS_APPLY		|
//			 125 Changes?-->Yes----o  |	      No				  o-------------------->|
//				|					  |		  |											|
//				|					  |		  |											|
//			    |   				  |		  |											|
//			    |   				  |		  |											|
//			WriteQueue --->----Yes--->o		  |											|
//			 has data?				  |		  |											|
//				|					  |		  |											|
//			    No  				  |		  |											|
//				|					  |		  |											|
//				|					  |	 WriteCommand-->---Yes----o						|
//	    TDS_TUNING_SIGNALS_READ 	  |	   is ActivateLM?		  |		     			|
//		(Recent Signal Hashes,		  |	      |		TDS_CHANGE_CONTROLLED_TUNING_SOURCE	|
//			  Max 250)				  |	      |					  o-------------------->|
//				|					  |	      |											|
//				|					  |	      |											|
//			    |       			  |	      |											|
//			    |    				  |  	  |											|
//			WriteQueue --->----Yes----o		  |											|
//			 has data?						  |											|
//				|							  |											|
//			    No							  |											|
//				|							  |											|
//				|						      o----------------ASSERT------------------>|
//		TDS_TUNING_SIGNALS_READ															|
//		(Next Queued Signal Hashes,														|
//			  Max 250)																	|
//				|																		|
//				o---->------------------------------------------------------------------o


namespace ClientLib
{
	class TuningTcpClient : public Tcp::Client, public TcpClientStatistics
	{
		Q_OBJECT

	public:
		TuningTcpClient(const SoftwareInfo& softwareInfo,
						const SoftwareEndpoint::TuningService& tunsInfo,
						ITuningSignalUpdater& signalUpdater,
						IRecentAppSignals& recentTuningSignals,
						ITuningAuthorization& tuningAuthorization,
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

		bool signalStatesLoaded() const;

	private:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

		// Sending requests and processing replies functions
		//
		void continueRequestLoop();

		[[nodiscard]] bool sendWriteRequest(int waitTimeMs);

		void requestTuningSourcesInfo();
		void processTuningSourcesInfo(const QByteArray& data);

		void requestTuningSourcesState();
		void processTuningSourcesState(const QByteArray& data);

		void requestActivateTuningSource(Hash equipmentHash, bool enableControl, bool forceTakeControl);
		void processActivateTuningSource(const QByteArray& data);

		void requestReadRecentTuningSignals();
		void processReadRecentTuningSignals(const QByteArray& data);

		void requestReadTuningSignals();
		void processReadTuningSignals(const QByteArray& data);

		void requestReadChangedTuningSignals();
		void processReadChangedTuningSignals(const QByteArray& data);

		void requestWriteTuningSignals(std::queue<TuningWriteCommand> writeQueue);
		void processWriteTuningSignals(const QByteArray& data);

		void requestApplyTuningSignals();
		void processApplyTuningSignals(const QByteArray& data);

		[[nodiscard]] bool processTuningSignalsReadReply(const QByteArray& data);
		[[nodiscard]] bool processTuningSignalStateMessage(const ::Network::TuningSignalState& stateMessage, std::vector<TuningSignalState>& arrivedStates);

	signals:
		void tuningSourcesInfoArrived();

		// Properties
		//
	public:
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

	public:
		inline static const int MaxStateRequestCount = TDS_TUNING_MAX_READ_STATES / 4;  // 250 signals per TDS_TUNING_MAX_READ_STATES
		inline static const int MaxStateWriteCount = TDS_TUNING_MAX_WRITE_RECORDS / 4;  // 250 signals per TDS_TUNING_MAX_WRITE_RECORDS

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
		
		std::set<Hash> m_signalStatesSet;					// Signal hash is added here when signal state is received

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
		IRecentAppSignals& m_recentTuningSignals;
		
		ITuningAuthorization& m_tuningAuthorization;

		// Write processing
		//
		std::mutex m_writeQueueMutex;				// For access to m_writeQueue
		std::condition_variable m_writeQueueCondition;

		std::queue<TuningWriteCommand> m_writeQueue;

		// Reading processing
		//
		enum class ReadRequestType
		{
			SourceState,
			Changed,
			Recent,
			Generic
		};
		ReadRequestType m_lastReadRequestType{ReadRequestType::Generic};

		int m_readTuningSignalIndex = 0;
		int m_readTuningSignalCount = 0;

		std::atomic<bool> m_signalStatesLoaded{false};

		// Active client processing
		//
		mutable QReadWriteLock m_activeClientMutex;				// For access to m_activeClientId, m_activeClientIp, m_singleLmControlMode, m_currentClientIsActive
		QString m_activeClientId;
		QString m_activeClientIp;
		bool m_currentClientIsActive = false;

		// Cached protobuf messages
		//
		//::Network::GetTuningSourcesStates m_getTuningSourcesStates;
		//::Network::GetTuningSourcesStatesReply m_tuningSourcesStatesReply;

		//::Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
		//::Network::GetTuningSourcesInfoReply m_tuningSourcesInfoReply;

		//::Network::ChangeConrolledTuningSourceRequest m_activateTuningSource;
		//::Network::ChangeConrolledTuningSourceReply m_activateTuningSourceReply;

		//::Network::TuningSignalsRead m_readTuningSignals;
		//::Network::TuningSignalsReadReply m_readTuningSignalsReply;

		//::Network::GetTuningSignalsStateChangesRequest m_readChangedTuningSignals;
		//::Network::GetTuningSignalsStateChangesReply m_readChangedTuningSignalsReply;

		//::Network::TuningSignalsWrite m_writeTuningSignals;
		//::Network::TuningSignalsWriteReply m_writeTuningSignalsReply;

		//::Network::TuningSignalsApply m_applyTuningSignals;
		//::Network::TuningSignalsApplyReply m_applyTuningSignalsReply;
	};

}
