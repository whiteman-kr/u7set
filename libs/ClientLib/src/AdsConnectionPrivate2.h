#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"
#include <ClientLib/IAppSignalUpdater.h>
#include <ClientLib/IRecentAppSignals.h>
#include <ClientLib/SignalLog.h>


namespace ClientLib
{
	class AdsClientGrpc;

	class AdsConnectionPrivate2 : public QObject
	{
		Q_OBJECT

	private:
		class Connection
		{
		public:
			Connection(const SoftwareInfo& softwareInfo,
					   const SoftwareEndpoint::AppDataService& ads,
					   IAppSignalUpdater& signalUpdater,
					   IRecentAppSignals* recentAppSignals,
					   SignalLog& signalLog,
					   ILogFile& logFile);

			[[nodiscard]] HostAddressPort address() const;
			[[nodiscard]] const SoftwareEndpoint::AppDataService& server() const;

			[[nodiscard]] Tcp::ConnectionState tcpConnectionState() const;

			[[nodiscard]] bool signalParamsLoaded() const;
			[[nodiscard]] bool signalStatesLoaded() const;

		private:
			std::unique_ptr<ClientLib::AdsClientGrpc> m_client;
		};

	public:
		explicit AdsConnectionPrivate2(
			IAppSignalUpdater& signalUpdater,
			IRecentAppSignals* recentAppSignals, // Can be nullptr, then recent state comm thread will not be created.
			ILogFile* logFile);
		virtual ~AdsConnectionPrivate2();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		[[nodiscard]] std::vector<Tcp::ConnectionState> connectionStates() const;

		[[nodiscard]] bool signalParamsLoaded() const;
		[[nodiscard]] bool signalStatesLoaded() const;

		[[nodiscard]] SignalLog& signalLog();
		[[nodiscard]] const SignalLog& signalLog() const;

		// --
		//
	private:
		IAppSignalUpdater& m_signalUpdater;
		IRecentAppSignals* m_recentAppSignals = nullptr; // If nullptr then recent connections are not used
		HasLogFile m_logFile;

		SignalLog m_signalLog;

		mutable QReadWriteLock m_connsMutex;
		std::list<Connection> m_conns;
	};
} // namespace ClientLib
