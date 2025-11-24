#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"

#include "IAppSignalUpdater.h"
#include "IRecentAppSignals.h"
#include "ISignalLogUpdater.h"


namespace ClientLib
{
	class AdsConnectionPrivate;
	class AdsConnectionPrivate2;


	class AdsConnection
	{
	public:
		explicit AdsConnection(IAppSignalUpdater& signalUpdater,
							   IRecentAppSignals* recentAppSignals, // Can be nullptr
							   ISignalLogUpdater* signalLogUpdater, // Can be nullptr
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

										  // --
		//
	private:
#ifdef USE_GRPC_ADS_CONNECTION
		using AdsConnectionType = ClientLib::AdsConnectionPrivate2; // Grpc-based connection
#else
		using AdsConnectionType = ClientLib::AdsConnectionPrivate; // Tcp-based connection
#endif

		bool m_hasRecentAppSignals = false;
		std::unique_ptr<AdsConnectionType> m_pimpl;
	};
} // namespace ClientLib
