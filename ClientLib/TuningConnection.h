#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../UtilsLib/ILogFile.h"
#include "../UtilsLib/SimpleThread.h"
#include "../lib/Tuning/ITuningConnection.h"
#include "TuningTcpClient.h"

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
					   TuningSignalManager& tuningSignalManager,
					   ILogFile* logFile,
					   TuningLog::TuningLog* tuningLog);
			Connection(const Connection&) = delete;
			Connection(Connection&& src) noexcept;
			~Connection();

			Connection& operator=(const Connection&) = delete;
			Connection& operator=(Connection&& src) noexcept;

			void stopAndDestroy();
			HostAddressPort address() const;

			// --
			//

			ClientLib::TuningTcpClient* tcpTuningClient = nullptr;
			SimpleThread* tcpClientThread = nullptr;
		};

	public:
		explicit TuningConnection(TuningSignalManager& tuningSignalManager, ILogFile* logFile, TuningLog::TuningLog* tuningLog);

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

		[[nodiscard]] bool tuningSourceIsActive(Hash sourceHash) const;
		[[nodiscard]] bool tuningSourceIsInactive(Hash sourceHash) const;
		[[nodiscard]] bool activateTuningSource(Hash sourceHash, bool activate) const;

		/// Client Control functions
		///
		QString clientControlInfo() const;

		[[nodiscard]] bool takeClientControl(const std::set<Hash>& sourceHashes) const;

		/// Reading channel-specific signal states from all connections
		///
		std::vector<std::pair<QString, TuningSignalState>> states(Hash appSignalHash) const;	// First is Tuning Service ID, Second is State

		/// Tuning signals functions
		///
		void writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands);

		virtual bool hasTuningSignal(QString appSignalId) const override;
		virtual bool writeTuningSignal(QString appSignalId, TuningValue tuningValue) override;

		/// Apply functions
		///
		void applyTuningSignals(const std::vector<Hash>& signalHashes);
		virtual void applyTuningSignals() override;

		// --
		//
	protected:
		std::list<Connection> m_conns;

	private:
		TuningSignalManager& m_tuningSignalManager;

		HasLogFile m_logFile;
		TuningLog::TuningLog* m_tuningLog = nullptr;
	};
}

