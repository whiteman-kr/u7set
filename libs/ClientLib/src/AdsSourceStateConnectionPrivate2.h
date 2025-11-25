#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../UtilsLib/ILogFile.h"
#include <ClientLib/AppDataSourceState.h>

#include <list>
#include <vector>


namespace ClientLib
{
	class AdsSourceStateClientGrpc;


	// Get application data sources states from AppDataServices(s)
	//
	class AdsSourceStateConnectionPrivate2
	{
	private:
		class Connection
		{
		public:
			Connection(const SoftwareInfo& softwareInfo, const SoftwareEndpoint::AppDataService& ads, ILogFile& logFile);

			[[nodiscard]] HostAddressPort address() const;
			[[nodiscard]] const SoftwareEndpoint::AppDataService& server() const;

			[[nodiscard]] Tcp::ConnectionState tcpConnectionState() const;

			std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

		private:
			std::unique_ptr<ClientLib::AdsSourceStateClientGrpc> m_client;
		};

	public:
		explicit AdsSourceStateConnectionPrivate2(ILogFile* logFile);
		~AdsSourceStateConnectionPrivate2();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService);

		[[nodiscard]] std::vector<Tcp::ConnectionState> adsConnectionStates() const;
		[[nodiscard]] std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

		// --
		//
	private:
		HasLogFile m_logFile;
		std::list<Connection> m_conns;
	};

} // namespace ClientLib
