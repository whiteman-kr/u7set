#include <AdsConnectionLib/ClientConnStatsStd.h>

//
// TcpClientInstance
//
std::mutex ClientConnStatsStd::s_mutex;
std::set<ClientConnStatsStd*> ClientConnStatsStd::s_clients;


ClientConnStatsStd::ClientConnStatsStd()
{
	std::scoped_lock locker{s_mutex};
	s_clients.insert(this);
	return;
}

ClientConnStatsStd::~ClientConnStatsStd()
{
	std::scoped_lock locker{s_mutex};

	assert(s_clients.count(this) == 1);
	s_clients.erase(this);

	return;
}

std::vector<ClientConnStatsStd::Statistics> ClientConnStatsStd::statistics()
{
	std::vector<Statistics> result;

	std::scoped_lock locker{s_mutex};
	result.reserve(s_clients.size());

	for (ClientConnStatsStd* tcpClient : s_clients)
	{
		assert(tcpClient);

		result.emplace_back(reinterpret_cast<uintptr_t>(tcpClient),
							tcpClient->statsObjectName(),
							tcpClient->statsServerId(),
							tcpClient->statsConnectionState());
	}

	return result;
}

void ClientConnStatsStd::reconnectClient(uintptr_t id)
{
	std::scoped_lock locker{s_mutex};

	auto ptr = reinterpret_cast<ClientConnStatsStd*>(id);
	if (s_clients.count(ptr) == 0)
	{
		return;
	}

	auto tcpClient = dynamic_cast<ClientConnStatsStd*>(ptr);
	if (tcpClient == nullptr)
	{
		assert(tcpClient);
		return;
	}

	tcpClient->statsReconnect();

	return;
}
