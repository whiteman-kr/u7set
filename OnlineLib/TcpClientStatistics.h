#pragma once

#include "../OnlineLib/TcpConnectionState.h"
#include <QMutex>
#include <set>
#include <vector>

namespace Tcp
{
	class Client;
}

class ClientConnectionStatistics
{
protected:
	ClientConnectionStatistics();
	virtual ~ClientConnectionStatistics();

	Q_DISABLE_COPY_MOVE(ClientConnectionStatistics);

public:
	virtual void statsReconnect() = 0;
	virtual QString statsObjectName() = 0;
	virtual QString statsServerId() = 0;
	virtual Tcp::ConnectionState statsConnectionState() = 0;

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

		uintptr_t id; // is a pointer to TcpClientInstance
		QString objectName;
		QString serverId;
		Tcp::ConnectionState state;
	};

	static std::vector<Statistics> statistics();
	static void reconnectClient(uintptr_t id);

private:
	static QMutex s_mutex;
	static std::set<ClientConnectionStatistics*> s_clients;
};


class TcpClientStatistics : public ClientConnectionStatistics
{
public:
	TcpClientStatistics(Tcp::Client* client);
	virtual ~TcpClientStatistics() = default;

public:
	virtual void statsReconnect() override;
	virtual QString statsObjectName() override;
	virtual QString statsServerId() override;
	virtual Tcp::ConnectionState statsConnectionState() override;

private:
	Tcp::Client* m_client = nullptr;
};