#ifndef ONLINE_LIB_DOMAIN
	#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "TcpClientStatistics.h"
#include "../OnlineLib/Tcp.h"

//
// TcpClientInstance
//

QMutex ClientConnectionStatistics::s_mutex;
std::set<ClientConnectionStatistics*> ClientConnectionStatistics::s_clients;


ClientConnectionStatistics::ClientConnectionStatistics()
{
	QMutexLocker l(&s_mutex);

	s_clients.insert(this);

	return;
}

ClientConnectionStatistics::~ClientConnectionStatistics()
{
	QMutexLocker l(&s_mutex);

	Q_ASSERT(s_clients.count(this) == 1);
	s_clients.erase(this);

	return;
}

std::vector<ClientConnectionStatistics::Statistics> ClientConnectionStatistics::statistics()
{
	std::vector<Statistics> result;

	QMutexLocker l(&s_mutex);
	result.reserve(s_clients.size());

	for (ClientConnectionStatistics* tcpClient : s_clients)
	{
		Q_ASSERT(tcpClient);

		result.emplace_back(reinterpret_cast<uintptr_t>(tcpClient),
							tcpClient->statsObjectName(),
							tcpClient->statsServerId(),
							tcpClient->statsConnectionState());
	}

	return result;
}

void ClientConnectionStatistics::reconnectClient(uintptr_t id)
{
	QMutexLocker l(&s_mutex);

	auto ptr = reinterpret_cast<ClientConnectionStatistics*>(id);
	if (s_clients.count(ptr) == 0)
	{
		return;
	}

	auto tcpClient = dynamic_cast<ClientConnectionStatistics*>(ptr);
	if (tcpClient == nullptr)
	{
		Q_ASSERT(tcpClient);
		return;
	}

	tcpClient->statsReconnect();

	return;
}

TcpClientStatistics::TcpClientStatistics(Tcp::Client* client) :
	m_client{client}
{
	Q_ASSERT(m_client);
}

void TcpClientStatistics::statsReconnect()
{
	m_client->setServers(m_client->serverAddressPort1(), m_client->serverAddressPort2(), true); // this will reconnect
}

QString TcpClientStatistics::statsObjectName()
{
	return m_client->objectName();
}

QString TcpClientStatistics::statsServerId()
{
	return m_client->connectToServerID();
}

Tcp::ConnectionState TcpClientStatistics::statsConnectionState()
{
	return m_client->getConnectionState();
}