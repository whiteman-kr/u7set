#pragma once
#include <set>
#include <vector>
#include "../OnlineLib/TcpConnectionState.h"


namespace Tcp
{
	class Client;
}


class TcpClientStatistics
{
protected:
	TcpClientStatistics() = delete;
	TcpClientStatistics(Tcp::Client* client);
	virtual ~TcpClientStatistics();

public:
	struct Statistics
	{
		Statistics(uintptr_t id, QString objectName, QString serverId, const Tcp::ConnectionState& state) :
			id{id},
			objectName{objectName},
			serverId{serverId},
			state{state}
		{
		}

		uintptr_t id;		// is a pointer to TcpClientInstance
		QString objectName;
		QString serverId;
		Tcp::ConnectionState state;
	};

	static std::vector<Statistics> statistics();
	static void reconnect(uintptr_t id);

private:
	Tcp::Client* m_client = nullptr;

	static QMutex s_mutex;
	static std::set<Tcp::Client*> s_clients;
};


