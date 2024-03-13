#pragma once

#include <vector>

#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"
#include "AppDataSourceState.h"


namespace ClientLib
{
	class AdsSourceStateConnectionPrivate;

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

		std::vector<Tcp::ConnectionState> adsConnectionStates() const;
		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

	private:
		std::unique_ptr<AdsSourceStateConnectionPrivate> m_pimpl;
	};
}