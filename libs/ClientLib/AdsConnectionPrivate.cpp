#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "AdsConnectionPrivate.h"
#include "../UtilsLib/SimpleThread.h"
#include "TcpSignalClient.h"
#include "TcpSignalRecents.h"


namespace ClientLib
{
	AdsConnectionPrivate::Connection::Connection(const SoftwareInfo& softwareInfo,
												 const SoftwareEndpoint::AppDataService& ads,
												 IAppSignalUpdater& signalUpdater,
												 IRecentAppSignals* recentAppSignals,
												 ILogFile* logFile)
	{
		tcpSignalClient = new ClientLib::TcpSignalClient{softwareInfo, ads, signalUpdater, logFile};
		tcpClientThread = new ::SimpleThread{tcpSignalClient};
		tcpClientThread->start();

		if (recentAppSignals != nullptr)
		{
			tcpSignalRecents = new ClientLib::TcpSignalRecents{softwareInfo, ads, *recentAppSignals, signalUpdater, logFile};
			tcpClientRecentThread = new SimpleThread{tcpSignalRecents};
			tcpClientRecentThread->start();
		}

		return;
	}

	AdsConnectionPrivate::Connection::~Connection()
	{
		stopAndDestroy();
		return;
	}

	void AdsConnectionPrivate::Connection::closeConnection() 
	{
		if (tcpSignalClient != nullptr && tcpSignalClient->isConnected() == true)
		{
			tcpSignalClient->closeConnection();
		}

		if (tcpSignalRecents != nullptr && tcpSignalRecents->isConnected() == true)
		{
			tcpSignalRecents->closeConnection();
		}
	}

	void AdsConnectionPrivate::Connection::stopAndDestroy()
	{
		if (tcpClientThread != nullptr)
		{
			tcpClientThread->quitAndWait(10000);
			delete tcpClientThread;
		}

		if (tcpClientRecentThread != nullptr)
		{
			tcpClientRecentThread->quitAndWait(10000);
			delete tcpClientRecentThread;
		}

		tcpSignalClient = nullptr;
		tcpClientThread = nullptr;
		tcpSignalRecents = nullptr;
		tcpClientRecentThread = nullptr;

		return;
	}

	HostAddressPort AdsConnectionPrivate::Connection::address() const
	{
		Q_ASSERT(tcpSignalClient);
		Q_ASSERT(tcpSignalRecents == nullptr || tcpSignalClient->serverAddressPort1() == tcpSignalRecents->serverAddressPort1());

		return tcpSignalClient->serverAddressPort1();
	}

	bool AdsConnectionPrivate::Connection::signalParamsLoaded() const
	{
		Q_ASSERT(tcpSignalClient);
		return tcpSignalClient->signalParamsLoaded();
	}

	bool AdsConnectionPrivate::Connection::signalStatesLoaded() const
	{
		Q_ASSERT(tcpSignalClient);
		return tcpSignalClient->signalStatesLoaded();
	}

	AdsConnectionPrivate::AdsConnectionPrivate(IAppSignalUpdater& signalUpdater, IRecentAppSignals* recentAppSignals, ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_logFile{logFile, "AdsConnectionPrivate"}
	{
		return;
	}

	AdsConnectionPrivate::~AdsConnectionPrivate()
	{
		m_logFile.writeMessage("~AdsConnectionPrivate()");
	}

	void AdsConnectionPrivate::updateConnections(const SoftwareInfo& softwareInfo,
												 const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		m_logFile.writeMessage(QString{"updateConnections(), %1 AppDataServices"}.arg(appDataServices.size()));
		for (const auto& ads : appDataServices)
		{
			m_logFile.writeMessage(QString{"updateConnections(),    %1 - %2"}.arg(ads.shortenId).arg(ads.address.toString()));
		}

		QWriteLocker locker{&m_connsMutex};

		m_signalUpdater.reset();

		// Remove connections that are not in the new configuration.
		//
		m_conns.remove_if(
			[&appDataServices](const Connection& c)
			{
				return std::none_of(appDataServices.begin(),
									appDataServices.end(),
									[&c](const SoftwareEndpoint::AppDataService& ads)
									{
										return c.address() == ads.address;
									});
			});

		// Add new connections.
		//
		for (const auto& ads : appDataServices)
		{
			auto it = std::find_if(m_conns.begin(),
								   m_conns.end(),
								   [&ads](const Connection& c)
								   {
									   return c.address() == ads.address;
								   });

			if (it != m_conns.end())
			{
				it->closeConnection();	// Force this connection to reconnect to receive signals again
				continue;
			}

			m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_logFile.logFile());
		}

		return;
	}

	std::vector<Tcp::ConnectionState> AdsConnectionPrivate::tcpSignalConnStates() const
	{
		QReadLocker locker{&m_connsMutex};

		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			Q_ASSERT(c.tcpSignalClient);
			states.emplace_back(c.tcpSignalClient->getConnectionState());
		}

		return states;
	}

	std::vector<Tcp::ConnectionState> AdsConnectionPrivate::recentSignalConnStates() const
	{
		std::vector<Tcp::ConnectionState> states;

		if (m_recentAppSignals == nullptr)
		{
			return states;
		}

		QReadLocker locker{&m_connsMutex};

		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			Q_ASSERT(c.tcpSignalRecents);
			states.emplace_back(c.tcpSignalRecents->getConnectionState());
		}

		return states;
	}

	bool AdsConnectionPrivate::signalParamsLoaded() const
	{
		QReadLocker locker{&m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalParamsLoaded();
						   });
	}

	bool AdsConnectionPrivate::signalStatesLoaded() const
	{
		QReadLocker locker{&m_connsMutex};
		return std::all_of(m_conns.begin(),
						   m_conns.end(),
						   [](const Connection& c)
						   {
							   return c.signalStatesLoaded();
						   });
	}

} // namespace ClientLib