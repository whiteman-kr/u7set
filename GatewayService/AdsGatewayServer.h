#pragma once

#include <mutex>

#include <asio.hpp>

#include <CommonLib/HostAddressPort.h>
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"

using asio::ip::tcp;

class AdsGatewayServer : public LogWrapper
{
public:
	AdsGatewayServer(const HostAddressPort& listenIP, CircularLoggerShared log);
	virtual ~AdsGatewayServer();

	void run();
	void stop();

private:
	void runAcceptLoop();
	void sessionThread(const std::shared_ptr<tcp::socket>& socket);
	void removeSessionSocket(const std::shared_ptr<tcp::socket>& socket);
	void reapFinishedSessions();
	void joinAllSessions();

	QString getIpPortStr(const std::shared_ptr<tcp::socket>& socket);

private:
	HostAddressPort m_listenIP;

	std::atomic<bool> m_running { false };
	std::thread m_serverThread;

	std::mutex m_acceptorMutex;
	tcp::acceptor* m_acceptor = nullptr;

	std::mutex m_sessionsMutex;
	std::vector<std::shared_ptr<tcp::socket>> m_sessionSockets;

	struct SessionThread
	{
		std::thread thread;
		std::shared_ptr<std::atomic<bool>> finished;
	};

	std::mutex m_threadsMutex;
	std::vector<SessionThread> m_sessionThreads;

	std::mutex m_signalsStatesMutex;
	std::unordered_map<Hash, SimpleAppSignalState> m_signalsStates;
};
