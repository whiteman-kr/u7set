#pragma once

#include <AdsConnectionLib/ILoggerStd.h>
#include <AdsConnectionLib/ISignalLogUpdater.h>
#include <AdsConnectionLib/ServiceConnectionState.h>
#include <AdsConnectionLib/ServiceEndpoint.h>
#include <AppSignalLibStd/IAppSignalUpdater.h>
#include <AppSignalLibStd/IRecentAppSignals.h>

#include <shared_mutex>


namespace ClientLib
{
	class AdsClientGrpc;

	class AdsConnectionPrivate2
	{
	private:
		class Connection
		{
		public:
			Connection(const ::Network::SoftwareInfo& softwareInfo,
					   const ServiceEndpoint& ads,
					   IAppSignalUpdater& signalUpdater,
					   IRecentAppSignals* recentAppSignals,
					   ISignalLogUpdater* signalLogUpdater,
					   ILoggerStd& logFile);

			~Connection();

			[[nodiscard]] const ServiceEndpoint& server() const;

			[[nodiscard]] ServiceConnectionState tcpConnectionState() const;

			[[nodiscard]] bool signalParamsLoaded() const;
			[[nodiscard]] bool signalStatesLoaded() const;

		private:
			ILoggerStd& m_logFile;
			std::unique_ptr<ClientLib::AdsClientGrpc> m_client;
		};

	public:
		explicit AdsConnectionPrivate2(IAppSignalUpdater& signalUpdater,
									   IRecentAppSignals* recentAppSignals, // Can be nullptr
									   ISignalLogUpdater* signalLogUpdater, // Can be nullptr
									   ILoggerStd& logFile);
		virtual ~AdsConnectionPrivate2();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const Network::SoftwareInfo& softwareInfo, const std::vector<ServiceEndpoint>& appDataServices);

		[[nodiscard]] std::vector<ServiceConnectionState> connectionStates() const;

		[[nodiscard]] bool signalParamsLoaded() const;
		[[nodiscard]] bool signalStatesLoaded() const;

		// --
		//
	private:
		IAppSignalUpdater& m_signalUpdater;
		IRecentAppSignals* m_recentAppSignals = nullptr; // If nullptr then recent connections are not used
		ISignalLogUpdater* m_signalLogUpdater = nullptr;
		ILoggerStd& m_logFile;

		mutable std::shared_mutex m_connsMutex;
		std::list<Connection> m_conns;
	};
} // namespace ClientLib
