#pragma once

#include "ServiceConnectionState.h"

#include <mutex>
#include <set>
#include <string>
#include <type_traits>

class ClientConnStatsStd
{
protected:
	ClientConnStatsStd();
	virtual ~ClientConnStatsStd();

	ClientConnStatsStd(const ClientConnStatsStd&) = delete;
	ClientConnStatsStd(ClientConnStatsStd&&) = delete;

	ClientConnStatsStd& operator=(const ClientConnStatsStd&) = delete;
	ClientConnStatsStd& operator=(ClientConnStatsStd&&) = delete;

public:
	virtual void statsReconnect() = 0;
	virtual std::string statsObjectName() = 0;
	virtual std::string statsServerId() = 0;
	virtual ServiceConnectionState statsConnectionState() = 0;

public:
	struct Statistics
	{
		Statistics(uintptr_t id, std::string objectName, std::string serverId, const ServiceConnectionState& state) :
			id{id},
			objectName{std::move(objectName)},
			serverId{std::move(serverId)},
			state{state}
		{
		}

		uintptr_t id{}; // is a pointer to TcpClientInstance
		std::string objectName{};
		std::string serverId{};
		ServiceConnectionState state{};
	};

	static std::vector<Statistics> statistics();
	static void reconnectClient(uintptr_t id);

private:
	static std::mutex s_mutex;
	static std::set<ClientConnStatsStd*> s_clients;
};