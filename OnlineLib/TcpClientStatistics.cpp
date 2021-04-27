#ifndef ONLINE_LIB_DOMAIN
#error Don't include this file in the project! Link OnlineLib instead.
#endif

#include "TcpClientStatistics.h"

//
// TcpClientInstance
//

QMutex TcpClientStatistics::s_mutex;
std::set<Tcp::Client*> TcpClientStatistics::s_clients;


TcpClientStatistics::TcpClientStatistics(Tcp::Client* client) :
	m_client(client)
{
	Q_ASSERT(m_client);

	QMutexLocker l(&s_mutex);
	s_clients.insert(m_client);

	return;
}

TcpClientStatistics::~TcpClientStatistics()
{
	QMutexLocker l(&s_mutex);

	Q_ASSERT(s_clients.count(m_client) == 1);
	s_clients.erase(m_client);

	return;
}

std::vector<TcpClientStatistics::Statisctics> TcpClientStatistics::statistics()
{
	std::vector<Statisctics> result;

	QMutexLocker l(&s_mutex);
	result.reserve(s_clients.size());

	for (Tcp::Client* tcpClient : s_clients)
	{
		if (tcpClient == nullptr)
		{
			Q_ASSERT(tcpClient);
			return {};
		}

		result.emplace_back(reinterpret_cast<size_t>(tcpClient),
							tcpClient->objectName(),
							tcpClient->getConnectionState());
	}

	return result;
}

void TcpClientStatistics::reconnect(size_t id)
{
	QMutexLocker l(&s_mutex);

	Tcp::Client* ptr = reinterpret_cast<Tcp::Client*>(id);
	if (s_clients.count(ptr) == 0)
	{
		return;
	}

	Tcp::Client* tcpClient = dynamic_cast<Tcp::Client*>(ptr);
	if (tcpClient == nullptr)
	{
		Q_ASSERT(tcpClient);
		return;
	}

	tcpClient->setServers(tcpClient->serverAddressPort1(), tcpClient->serverAddressPort2(), true);		// this will reconnect

	return;
}


