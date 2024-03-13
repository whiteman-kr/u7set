#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "./include/ClientLib/AdsConnection.h"
#include "./include/ClientLib/TcpSignalClient.h"
#include "./include/ClientLib/TcpSignalRecents.h"
#include "../UtilsLib/SimpleThread.h"


namespace ClientLib
{

	AdsConnection::Connection::Connection(const SoftwareInfo& softwareInfo,
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

	AdsConnection::Connection::~Connection()
	{
		stopAndDestroy();
		return;
	}

	void AdsConnection::Connection::stopAndDestroy()
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

	HostAddressPort AdsConnection::Connection::address() const
	{
		Q_ASSERT(tcpSignalClient);
		Q_ASSERT(tcpSignalRecents == nullptr || tcpSignalClient->serverAddressPort1() == tcpSignalRecents->serverAddressPort1());

		return tcpSignalClient->serverAddressPort1();
	}

	bool AdsConnection::Connection::signalParamsLoaded() const
	{
		Q_ASSERT(tcpSignalClient);
		return tcpSignalClient->signalParamsLoaded();
	}

	bool AdsConnection::Connection::signalStatesLoaded() const
	{
		Q_ASSERT(tcpSignalClient);
		return tcpSignalClient->signalStatesLoaded();
	}

	AdsConnection::AdsConnection(IAppSignalUpdater& signalUpdater,
								 IRecentAppSignals* recentAppSignals,
								 ILogFile* logFile) :
		m_signalUpdater{signalUpdater},
		m_recentAppSignals{recentAppSignals},
		m_logFile{logFile, "AdsConnection"}
	{
		return;
	}

	AdsConnection::~AdsConnection()
	{
		qDebug() << "~AdsConnection()";
		m_logFile.writeMessage("~AdsConnection()");
	}

	void AdsConnection::updateConnections(const SoftwareInfo& softwareInfo, const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
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

	std::vector<Tcp::ConnectionState> AdsConnection::tcpSignalConnStates() const
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

	std::vector<Tcp::ConnectionState> AdsConnection::recentSignalConnStates() const
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

	bool AdsConnection::signalParamsLoaded() const
	{
		return std::all_of(m_conns.begin(), m_conns.end(), [](const Connection& c) { return c.signalParamsLoaded(); });
	}

	bool AdsConnection::signalStatesLoaded() const
	{
		return std::all_of(m_conns.begin(), m_conns.end(), [](const Connection& c) { return c.signalStatesLoaded(); });
	}

}	// namespace
