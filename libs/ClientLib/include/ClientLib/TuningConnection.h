#pragma once

#include "../AppSignalLib/ITuningSignalManager.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../UtilsLib/ILogFile.h"
#include "../UtilsLib/SimpleThread.h"
#include "../lib/Tuning/ITuningAuthorization.h"
#include "../lib/Tuning/ITuningConnection.h"
#include "IRecentAppSignals.h"
#include "ITuningSignalUpdater.h"
#include "ITuningLog.h"
#include "TuningSourceState.h"
#include "TuningWriteCommand.h"


class SimpleThread;
class TuningSignalManager;

namespace ClientLib
{
	class TuningTcpClient;

	class TuningConnection : public QObject, public ITuningConnection
	{
		Q_OBJECT

	protected:
		struct Connection
		{
			Connection(const SoftwareInfo& softwareInfo,
					   const SoftwareEndpoint::TuningService& tuns,
					   bool autoApply,
					   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
					   ITuningSignalUpdater& signalUpdater,
					   IRecentAppSignals& recentTuningSignals,
					   ITuningAuthorization& tuningAuthorization,
					   ILogFile* logFile,
					   ITuningLog* tuningLog);
			Connection(const Connection&) = delete;
			Connection(Connection&& src) = delete;
			~Connection();

			Connection& operator=(const Connection&) = delete;
			Connection& operator=(Connection&& src) = delete;

			void stopAndDestroy();
			HostAddressPort address() const;

			bool signalStatesLoaded() const;

			// --
			//
			ClientLib::TuningTcpClient* tcpTuningClient = nullptr;
			SimpleThread* tcpClientThread = nullptr;
		};

	public:
		explicit TuningConnection(ITuningSignalManager& tuningSignalManager,
								  ITuningSignalUpdater& tuningSignalUpdater,
								  IRecentAppSignals& recentTuningSignals,
								  ITuningAuthorization& tuningAuthorization,
								  ILogFile* logFile,
								  ITuningLog* tuningLog);

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
							   bool autoApply,
							   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode);

		/// Statistics functions
		///
		[[nodiscard]] std::vector<Tcp::ConnectionState> tcpTuningConnStates() const;

		/// Tuning sources functions
		///
		[[nodiscard]] std::vector<TuningSource> tuningSourcesInfo() const;
		[[nodiscard]] std::vector<TuningSource> tuningSourceInfo(Hash sourceHash) const;

		/// Returns number of communication channels for tuning source (LogicModule). Now 1 or 2.
		/// (Used only Single LM Control is turned on)
		///
		[[nodiscard]] int tuningSourceStatesCount(Hash sourceHash) const;

		/// Returns number of activated communication channels for tuning source (LogicModule). Now 0, 1 or 2.
		///	Activation is Tusning Service feature, LM does not know about it.
		/// (Used only Single LM Control is turned on)
		///
		[[nodiscard]] int activatedTuningSourceStatesCount(Hash sourceHash) const;

		/// Command to TuningServices to (de)activate communication channel to source (LogicModule).
		/// (Used only Single LM Control is turned on)
		///
		[[nodiscard]] bool activateTuningSource(Hash sourceHash, bool activate) const;

		/// Get information about current state of client control. Used for printing info in status bars, etc.
		/// (Used only Single LM Control is turned on)
		///
		QString clientControlInfo() const;

		/// Make this client active for TuningServices that control the specified sources.
		/// (Used only Single LM Control is turned on)
		///
		[[nodiscard]] bool takeClientControl(Hash sourceHash) const;

		/// Tuning signals functions
		///
		bool writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands);
		virtual bool writeTuningSignal(const QString& appSignalId, const TuningValue& tuningValue) override;
		virtual bool writeTuningSignal(const QString& appSignalId, QVariant value) override;

		bool signalStatesLoaded() const;

		/// Apply functions
		///
		void applyTuningSignals(const std::vector<Hash>& signalHashes);
		virtual void applyTuningSignals() override;

		// --
		//
	protected:
		std::list<Connection> m_conns;

	private:
		ITuningSignalManager& m_tuningSignalManager;
		ITuningSignalUpdater& m_tuningSignalUpdater;
		IRecentAppSignals& m_recentTuningSignals;

		HasLogFile m_logFile;
		ITuningAuthorization& m_tuningAuthorization;
		ITuningLog* m_tuningLog = nullptr;
	};
}

