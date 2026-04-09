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
												 ISignalLogUpdater* signalLogUpdater,
												 ILogFile* logFile)
	{
		tcpSignalClient = new ClientLib::TcpSignalClient{softwareInfo, ads, signalUpdater, signalLogUpdater, logFile};
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

	const SoftwareEndpoint::AppDataService& AdsConnectionPrivate::Connection::server() const
	{
		Q_ASSERT(tcpSignalClient);
		Q_ASSERT(tcpSignalRecents == nullptr || tcpSignalClient->serverAddressPort1() == tcpSignalRecents->serverAddressPort1());

		return tcpSignalClient->server();
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

	AdsConnectionPrivate::AdsConnectionPrivate(IAppSignalUpdater& signalUpdater,
											   IRecentAppSignals* recentAppSignals,
											   ISignalLogUpdater* signalLogUpdater,
											   ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_signalLogUpdater{signalLogUpdater},
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

		// Number of AppDataServices has been changed or any address has been changed
		//
		bool connectionsChanged = (m_conns.size() != appDataServices.size()) ||
								  std::any_of(m_conns.begin(),
											  m_conns.end(),
											  [&appDataServices](const Connection& conn)
											  {
												  return std::none_of(appDataServices.begin(),
																	  appDataServices.end(),
																	  [&conn](const SoftwareEndpoint::AppDataService& ads)
																	  {
																		  return conn.server() == ads;
																	  });
											  });

		if (connectionsChanged == true)
		{
			m_conns.clear();
			m_signalUpdater.reset();

			// Create connections
			//
			for (const auto& ads : appDataServices)
			{
				m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_signalLogUpdater, m_logFile.logFile());
			}
		}

		return;
	}

	std::vector<Tcp::ConnectionState> AdsConnectionPrivate::connectionStates() const
	{
		QReadLocker locker{&m_connsMutex};

		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size() * 2);

		for (const Connection& c : m_conns)
		{
			Q_ASSERT(c.tcpSignalClient);
			states.emplace_back(c.tcpSignalClient->getConnectionState());
		}

		if (m_recentAppSignals != nullptr)
		{
			for (const Connection& c : m_conns)
			{
				Q_ASSERT(c.tcpSignalRecents);
				states.emplace_back(c.tcpSignalRecents->getConnectionState());
			}
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