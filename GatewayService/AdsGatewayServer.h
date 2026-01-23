#pragma once

#include <mutex>
#include <limits>

#include <asio.hpp>
#include <AdsGatewayLib/AdsGwProtocol.hpp>
#include <AdsGatewayLib/GwCrc32.hpp>

#include <CommonLib/HostAddressPort.h>
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"

using asio::ip::tcp;
namespace AGL = AdsGatewayLib;

using TCP_SOCKET_SHARED = std::shared_ptr<tcp::socket>;

class AdsGatewayServer : public LogWrapper
{
public:
	struct SessionThreadContext
	{
		TCP_SOCKET_SHARED socket;
		bool handshakeCompleted = false;
		QString clientName;
		int signalListIndex = 0;
	};

	using SessionThreadContextShared = std::shared_ptr<SessionThreadContext>;

public:
	AdsGatewayServer(const HostAddressPort& listenIP, const AppSignals& appSignals, CircularLoggerShared log);
	virtual ~AdsGatewayServer();

	void run();
	void stop();

private:
	void runAcceptLoop();
	void sessionThread(SessionThreadContextShared stc);
	void removeSessionSocket(const TCP_SOCKET_SHARED& socket);
	void reapFinishedSessions();
	void joinAllSessions();

	void processRequest(SessionThreadContextShared stc, char* recvBuf, std::size_t& recvBufIndex);

	std::size_t processHandshakeRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, std::size_t& recvBufIndex);
	std::size_t processSignalListStartRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
											const char* recvBuf, std::size_t& recvBufIndex);
	std::size_t processSignalListNextRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
											  const char* recvBuf, std::size_t& recvBufIndex);

	bool sendErrReply(SessionThreadContextShared stc, const AdsGatewayLib::GwMessageHeader& requestHeader, AGL::GwErrorCode errCode);
	bool sendOkReply(SessionThreadContextShared stc, const AGL::GwMessageHeader& requestHeader, const char* payloadData, std::size_t payloadSize);
	bool sendReply(SessionThreadContextShared stc,
				   uint32_t requestID, AGL::GwErrorCode errCode,
				   const char* payloadData, std::size_t payloadSize);

	bool checkNullTerminated(const char* str, std::size_t size);
	QString getIpPortStr(const std::shared_ptr<tcp::socket>& socket);

private:
	HostAddressPort m_listenIP;
	const AppSignals& m_appSignals;

	std::atomic<bool> m_running { false };
	std::thread m_serverThread;

	std::mutex m_acceptorMutex;
	tcp::acceptor* m_acceptor = nullptr;

	std::mutex m_sessionsMutex;
	std::vector<TCP_SOCKET_SHARED> m_sessionSockets;

	struct SessionThread
	{
		std::thread thread;
		std::shared_ptr<std::atomic<bool>> finished;
	};

	std::mutex m_threadsMutex;
	std::vector<SessionThread> m_sessionThreads;

	std::mutex m_signalsStatesMutex;
	std::unordered_map<Hash, SimpleAppSignalState> m_signalsStates;

	static constexpr std::size_t CONTINUE_RECEIVE = std::numeric_limits<std::size_t>::max();
};


