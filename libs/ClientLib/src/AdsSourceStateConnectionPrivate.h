#pragma once

#include <list>
#include <vector>

#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/ILogFile.h"
#include <ClientLib/AppDataSourceState.h>

class SimpleThread;

namespace ClientLib
{
	class TcpAppSourcesState;

	// Get application data sources states from AppDataServices(s)
	//
	class AdsSourceStateConnectionPrivate : public QObject
	{
		Q_OBJECT

	private:
		struct Connection
		{
			Connection(const SoftwareInfo& softwareInfo, const SoftwareEndpoint::AppDataService& ads, ILogFile* logFile);
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
		explicit AdsSourceStateConnectionPrivate(ILogFile* logFile);
		~AdsSourceStateConnectionPrivate() = default;

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

} // namespace ClientLib
