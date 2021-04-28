#pragma once
#include <set>
#include <vector>
#include "../OnlineLib/Tcp.h"


class TcpClientStatistics
{
protected:
	TcpClientStatistics() = delete;
	TcpClientStatistics(Tcp::Client* client);
	virtual ~TcpClientStatistics();

public:
	struct Statisctics
	{
		Statisctics(size_t _id, QString _objectName, Tcp::ConnectionState _state) :
			id(_id),
			objectName(_objectName),
			state(_state)
		{
		}

		size_t id;		// is a pointer to TcpClientInstance
		QString objectName;
		Tcp::ConnectionState state;
	};

	static std::vector<Statisctics> statistics();
	static void reconnect(uintptr_t id);

private:
	Tcp::Client* m_client = nullptr;

	static QMutex s_mutex;
	static std::set<Tcp::Client*> s_clients;
};


