#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "AdsConnectionPrivate.h"
#include "TcpSignalClient.h"
#include "TcpSignalRecents.h"
#include "../UtilsLib/SimpleThread.h"


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

	AdsConnectionPrivate::AdsConnectionPrivate(IAppSignalUpdater& signalUpdater, 
											   IRecentAppSignals* recentAppSignals, 
											   ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_logFile{logFile, "AdsConnectionPrivate"}
	{
		return;
	}

	AdsConnectionPrivate::~AdsConnectionPrivate()
	{
		qDebug() << "~AdsConnectionPrivate()";
		m_logFile.writeMessage("~AdsConnectionPrivate()");
	}

	void AdsConnectionPrivate::updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		m_logFile.writeMessage("updateConnections()");

		m_conns.clear();	// it will stop all connection threads and destroy them
		m_signalUpdater.reset();

		for (const auto& ads : appDataServices)
		{
			auto it = std::find_if(m_conns.begin(), m_conns.end(), [&ads](const Connection& c)
								   {
									   return c.address() == ads.address;
								   });

			if (it != m_conns.end())
			{
				// Such connection already exists
				//
				continue;
			}

			m_conns.emplace_back(softwareInfo, ads, m_signalUpdater, m_recentAppSignals, m_logFile.logFile());
		}

		return;
	}

	std::vector<Tcp::ConnectionState> AdsConnectionPrivate::tcpSignalConnStates() const
	{
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
		return std::all_of(m_conns.begin(), m_conns.end(), [](const Connection& c) { return c.signalParamsLoaded(); });
	}

	bool AdsConnectionPrivate::signalStatesLoaded() const
	{
		return std::all_of(m_conns.begin(), m_conns.end(), [](const Connection& c) { return c.signalStatesLoaded(); });
	}

}	// namespace
