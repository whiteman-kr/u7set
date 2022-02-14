#include "AdsConnection.h"

AdsConnection::Connection::Connection(MonitorConfigController& configController,
									  const MonitorSettings::AppDataService& ads,
									  MonitorSignalManager& signalManager,
									  ILogFile* logFile)
{
	tcpSignalClient = new TcpSignalClient{configController, ads, signalManager, logFile};
	tcpClientThread = new SimpleThread{tcpSignalClient};
	tcpClientThread->start();

	tcpSignalRecents = new TcpSignalRecents{configController, ads, signalManager, logFile};
	tcpClientRecentThread = new SimpleThread{tcpSignalRecents};
	tcpClientRecentThread->start();

	connect(&signalManager, &MonitorSignalManager::addSignalToPriorityList, tcpSignalRecents, &TcpSignalRecents::addSignal, Qt::QueuedConnection);
	connect(&signalManager, &MonitorSignalManager::addSignalsToPriorityList, tcpSignalRecents, &TcpSignalRecents::addSignals, Qt::QueuedConnection);

	return;
}

AdsConnection::Connection::~Connection()
{
	stopAndDestroy();
	return;
}

AdsConnection::Connection::Connection(Connection&& src) noexcept
{
	operator=(std::move(src));
	return;
}

AdsConnection::Connection& AdsConnection::Connection::operator=(Connection&& src) noexcept
{
	if (this == &src)
	{
		Q_ASSERT(this != &src);
		return *this;
	}

	tcpSignalClient = src.tcpSignalClient;
	tcpClientThread = src.tcpClientThread;
	tcpSignalRecents = src.tcpSignalRecents;
	tcpClientRecentThread = src.tcpClientRecentThread;

	src.tcpSignalClient = nullptr;
	src.tcpClientThread = nullptr;
	src.tcpSignalRecents = nullptr;
	src.tcpClientRecentThread = nullptr;

	return *this;
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
	Q_ASSERT(tcpSignalRecents);
	Q_ASSERT(tcpSignalClient->serverAddressPort1() == tcpSignalRecents->serverAddressPort1());

	return tcpSignalClient->serverAddressPort1();
}

AdsConnection::AdsConnection(MonitorConfigController& configController,
							 MonitorSignalManager& signalManager,
							 ILogFile* logFile) :
	HasLogFile(logFile, "AdsConnection"),
	m_configController(configController),
	m_signalManager(signalManager)
{

	connect(&m_configController, &MonitorConfigController::configurationArrived, this, &AdsConnection::configurationArrived);

	return;
}

std::vector<Tcp::ConnectionState> AdsConnection::tcpSignalConnStates() const
{
	std::vector<Tcp::ConnectionState> states;
	states.reserve(m_conns.size());

	for (const Connection& c : m_conns)
	{
		states.emplace_back(c.tcpSignalClient->getConnectionState());
	}

	return states;
}

std::vector<Tcp::ConnectionState> AdsConnection::recentSignalConnStates() const
{
	std::vector<Tcp::ConnectionState> states;
	states.reserve(m_conns.size());

	for (const Connection& c : m_conns)
	{
		states.emplace_back(c.tcpSignalRecents->getConnectionState());
	}

	return states;
}

void AdsConnection::configurationArrived(ConfigSettings conf)
{
	m_conns.clear();	// it will stop all connection threads and destroy them
	m_signalManager.reset();

	for (const MonitorSettings::AppDataService& ads : conf.appDataServices)
	{
		m_conns.emplace_back(m_configController, ads, m_signalManager, logFile());
	}

	return;
}
