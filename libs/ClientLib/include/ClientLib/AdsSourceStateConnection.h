#pragma once

#include "AppDataSourceState.h"

#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"
#include <AdsConnectionLib/ServiceConnectionState.h>

#include <memory>
#include <vector>


namespace ClientLib
{
	class AdsSourceStateConnectionPrivate;  // Tcp version
	class AdsSourceStateConnectionPrivate2; // gRPC version

	// Get application data sources states from AppDataServices(s)
	//
	class AdsSourceStateConnection
	{
	public:
		explicit AdsSourceStateConnection(ILogFile* logFile);
		~AdsSourceStateConnection();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService);

		std::vector<ServiceConnectionState> adsConnectionStates() const;
		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

	private:
#ifdef USE_GRPC_ADS_CONNECTION
		using AdsConnectionType = ClientLib::AdsSourceStateConnectionPrivate2;
#else
		using AdsConnectionType = ClientLib::AdsSourceStateConnectionPrivate;
#endif
		std::unique_ptr<AdsConnectionType> m_pimpl;
	};
} // namespace ClientLib