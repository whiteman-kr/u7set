#pragma once

#include <vector>
#include <list>

#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/ILogFile.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "TcpAppSourcesState.h"


class SimpleThread;


namespace ClientLib
{
	// Get application data sources states from AppDataServices(s)
	//
	class AdsSourceStateConnection : public QObject
	{
		Q_OBJECT

	private:
		struct Connection
		{
			Connection(const SoftwareInfo& softwareInfo,
					   const SoftwareEndpoint::AppDataService& ads,
					   ILogFile* logFile);
			Connection(const Connection&) = delete;
			Connection(Connection&& src) = delete;
			~Connection();

			Connection& operator=(const Connection&) = delete;
			Connection& operator=(Connection&& src) = delete;

			void stopAndDestroy();

			[[nodiscard]] HostAddressPort address() const;

			// --
			//
			TcpAppSourcesState* tcpAppSourceStateClient = nullptr;
			SimpleThread* tcpAppSourceStateThread = nullptr;
		};

	public:
		explicit AdsSourceStateConnection(ILogFile* logFile);
		~AdsSourceStateConnection() = default;

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService);

		std::vector<Tcp::ConnectionState> adsConnectionStates() const;
		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

	private:
		void createAndStart(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataService);

		// --
		//
	private:
		HasLogFile m_logFile;
		std::list<Connection> m_conns;
	};

}
