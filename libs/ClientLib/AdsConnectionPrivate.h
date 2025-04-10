#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/TcpConnectionState.h"
#include "../UtilsLib/ILogFile.h"
#include <ClientLib/IAppSignalUpdater.h>
#include <ClientLib/IRecentAppSignals.h>

class SimpleThread;

namespace ClientLib
{
	class TcpSignalClient;
	class TcpSignalRecents;


	class AdsConnectionPrivate : public QObject
	{
		Q_OBJECT

	private:
		struct Connection
		{
			Connection(const SoftwareInfo& softwareInfo,
					   const SoftwareEndpoint::AppDataService& ads,
					   IAppSignalUpdater& signalUpdater,
					   IRecentAppSignals* recentAppSignals,
					   ILogFile* logFile);
			~Connection();

			Connection() = delete;
			Connection(const Connection&) = delete;
			Connection(Connection&& src) = delete;
			Connection& operator=(const Connection&) = delete;
			Connection& operator=(Connection&& src) = delete;

			void stopAndDestroy();

			HostAddressPort address() const;
			const SoftwareEndpoint::AppDataService& server() const;

			bool signalParamsLoaded() const;
			bool signalStatesLoaded() const;

			// --
			//
			ClientLib::TcpSignalClient* tcpSignalClient = nullptr;
			SimpleThread* tcpClientThread = nullptr;

			ClientLib::TcpSignalRecents* tcpSignalRecents = nullptr;
			::SimpleThread* tcpClientRecentThread = nullptr;
		};

	public:
		explicit AdsConnectionPrivate(
			IAppSignalUpdater& signalUpdater,
			IRecentAppSignals* recentAppSignals, // Can be nullptr, then recent state comm thread will not be created.
			ILogFile* logFile);
		virtual ~AdsConnectionPrivate();

	public:
		/// Call this function when the new configuration arrived to recreate communication thread with the new configuration
		///
		void updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		std::vector<Tcp::ConnectionState> tcpSignalConnStates() const;
		std::vector<Tcp::ConnectionState> recentSignalConnStates() const;

		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

		// --
		//
	private:
		IAppSignalUpdater& m_signalUpdater;
		IRecentAppSignals* m_recentAppSignals = nullptr; // If nullptr then recent connections are not used
		HasLogFile m_logFile;

		mutable QReadWriteLock m_connsMutex;
		std::list<Connection> m_conns;
	};
} // namespace ClientLib
