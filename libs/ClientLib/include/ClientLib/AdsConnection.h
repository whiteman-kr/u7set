#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"

#include "IAppSignalUpdater.h"
#include "IRecentAppSignals.h"
#include "SignalLog.h"


namespace ClientLib
{
	class AdsConnectionPrivate;
	class AdsConnectionPrivate2;


	class AdsConnection
	{
	public:
		explicit AdsConnection(IAppSignalUpdater& signalUpdater,
							   IRecentAppSignals* recentAppSignals, // Can be nullptr, then recent state comm thread will not be created.
							   ILogFile* logFile);

		AdsConnection(const AdsConnection&) = delete;
		AdsConnection(AdsConnection&&) = default;

		AdsConnection& operator=(const AdsConnection&) = delete;
		AdsConnection& operator=(AdsConnection&&) = default;

		virtual ~AdsConnection();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		std::vector<Tcp::ConnectionState> connectionStates() const;

		int connectionsPerServer() const; // Returns 2 for TcpConnection and 1 for Grpc.

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

		ClientLib::SignalLog& signalLog();
		const ClientLib::SignalLog& signalLog() const;

		// --
		//
	private:
#if 0
		// Tcp-based connection
		//
		using AdsConnectionType = ClientLib::AdsConnectionPrivate;
#else
		// Grpc-based connection
		//
		using AdsConnectionType = ClientLib::AdsConnectionPrivate2;
#endif

		bool m_hasRecentAppSignals = false;
		std::unique_ptr<AdsConnectionType> m_pimpl;
	};
} // namespace ClientLib
