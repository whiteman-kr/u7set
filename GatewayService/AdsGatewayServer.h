#pragma once

#include <limits>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <memory>
#include <thread>

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

		std::vector<char> payloadData;
	};

	using SessionThreadContextShared = std::shared_ptr<SessionThreadContext>;

public:
	AdsGatewayServer(const HostAddressPort& listenIP, const AppSignals& appSignals, CircularLoggerShared log);
	virtual ~AdsGatewayServer();

	void run();
	void stop();

	void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply);
	void processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply);

private:
	void runAcceptLoop();
	void sessionThread(SessionThreadContextShared stc);
	void removeSessionSocket(const TCP_SOCKET_SHARED& socket);
	void reapFinishedSessions();
	void joinAllSessions();

	void processRequest(SessionThreadContextShared stc, char* recvBuf, size_t& recvBufIndex);

	size_t processHandshakeRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalListStartRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalListNextRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalParamStartRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalParamNextRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalStateRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										 const char* recvBuf, size_t& recvBufIndex);
	size_t processSignalStateChangesRequest(SessionThreadContextShared stc, const AGL::GwMessageHeader& header,
										 const char* recvBuf, size_t& recvBufIndex);

	bool sendErrReply(SessionThreadContextShared stc, const AdsGatewayLib::GwMessageHeader& requestHeader, AGL::GwErrorCode errCode);
	bool sendOkReply(SessionThreadContextShared stc, const AGL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize);
	bool sendReply(SessionThreadContextShared stc,
				   uint32_t requestID, AGL::GwErrorCode errCode,
				   const char* payloadData, size_t payloadSize);

	bool checkNullTerminated(const char* str, size_t size) const;
	QString getIpPortStr(const std::shared_ptr<tcp::socket>& socket) const;

	void copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const;
	uint8_t channelChar(E::Channel ch) const;

private:
	HostAddressPort m_listenIP;
	const AppSignals& m_appSignals;

	std::atomic<bool> m_running { false };
	std::thread m_serverThread;

	asio::io_context m_io;

	std::mutex m_acceptorMutex;
	std::shared_ptr<tcp::acceptor> m_acceptor;

	std::mutex m_sessionsMutex;
	std::vector<TCP_SOCKET_SHARED> m_sessionSockets;

	struct SessionThread
	{
		std::thread thread;
		std::shared_ptr<std::atomic<bool>> finished;
	};

	std::mutex m_threadsMutex;
	std::vector<SessionThread> m_sessionThreads;

	std::mutex m_signalStatesMutex;
	std::vector<SimpleAppSignalState> m_signalStates;
	std::unordered_map<Hash, int> m_hashToIndex;

	std::mutex m_signalStateChangesMutex;
	std::deque<AGL::GwAppSignalState> m_signalStateChanges;

	static constexpr size_t CONTINUE_RECEIVE = std::numeric_limits<size_t>::max();
	static constexpr int BAD_INDEX = -1;
};
