#pragma once

#include "../AppSignalLib/ITuningSignalManager.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"
#include "ITuningLog.h"
#include "ITuningSignalUpdater.h"
#include "TuningSourceState.h"
#include "TuningWriteCommand.h"
#include <AppSignalLibStd/IRecentAppSignals.h>
#include <ClientLib/ITuningAuthorization.h>
#include <ClientLib/ITuningConnection.h>


namespace ClientLib
{
	class TuningConnectionPrivate;


	class TuningConnection : public ITuningConnection
	{
	public:
		explicit TuningConnection(ITuningSignalManager& tuningSignalManager,
								  ITuningSignalUpdater& tuningSignalUpdater,
								  IRecentAppSignals& recentTuningSignals,
								  ITuningAuthorization& tuningAuthorization,
								  ILogFile* logFile,
								  ITuningLog* tuningLog);

		~TuningConnection() override;

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
		///	Activation is Tuning Service feature, LM does not know about it.
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
		bool writeTuningSignals(const std::vector<ClientLib::TuningWriteCommand>& writeCommands);
		virtual bool writeTuningSignal(const QString& appSignalId, const TuningValue& tuningValue) override;
		virtual bool writeTuningSignal(const QString& appSignalId, QVariant value) override;

		bool signalStatesLoaded() const;

		/// Apply functions
		///
		void applyTuningSignals(const std::vector<Hash>& signalHashes);
		virtual void applyTuningSignals() override;

	private:
		std::unique_ptr<TuningConnectionPrivate> m_pimpl;
	};
} // namespace ClientLib
