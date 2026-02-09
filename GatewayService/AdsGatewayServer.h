#pragma once

#include <limits>
#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <memory>
#include <thread>
#include <set>
#include <atomic>

#include <asio.hpp>
#include <AdsGatewayLib/AdsGwProtocol.hpp>
#include <AdsGatewayLib/GwCrc32.hpp>

#include <CommonLib/HostAddressPort.h>
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"

using asio::ip::tcp;
namespace AGL = AdsGatewayLib;

using TcpSocketShared = std::shared_ptr<tcp::socket>;

class AdsGatewayServer : public LogWrapper
{
public:
	struct Session
	{
		TcpSocketShared socket;
		std::thread thread;
		std::atomic_bool finished {false};
		std::atomic_bool closing { false };

		//

		bool handshakeCompleted = false;
		QString clientName;
		std::atomic_bool connectedToAppDataSrv {false};
		std::vector<char> payloadData;
		size_t errCount = 0;
	};

	using SessionShared = std::shared_ptr<Session>;

	static constexpr size_t MAX_SESSION_ERRORS = 100;

public:
	AdsGatewayServer(const HostAddressPort& listenIP, const AppSignals& appSignals, CircularLoggerShared log);
	virtual ~AdsGatewayServer();

	void run();
	void stop();

	void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply);
	void processStateChanges(const Network::GetAppSignalStateChangesReply& getStateChangesReply);

	void setConnectedToAppDataSrv(bool connected);

private:
	void runAcceptLoop();
	void sessionThread(SessionShared stc);
	void reapFinishedSessions();
	void closeSocket(SessionShared stc);
	void requestCloseSession(SessionShared stc);
	void closeSessions();
	void joinAllSessions();

	void processRequest(SessionShared stc, char* recvBuf, size_t& recvBufSize);

	bool processHandshakeRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalListStartRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalListNextRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalParamStartRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalParamNextRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalStateRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										 const char* recvBuf, const size_t requestSize);
	bool processSignalStateChangesRequest(SessionShared stc, const AGL::GwMessageHeader& header,
										 const char* recvBuf, const size_t requestSize);

	bool checkPayloadSize(const AGL::GwMessageHeader& header, const char* recvBuf, const size_t recvBufSize, AGL::GwErrorCode& errCode);
	[[nodiscard]] size_t skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize);

	void sendErrReply(SessionShared stc, const AdsGatewayLib::GwMessageHeader& requestHeader, AGL::GwErrorCode errCode);
	void sendOkReply(SessionShared stc, const AGL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize);
	void sendReply(SessionShared stc,
				   uint32_t requestID, AGL::GwErrorCode errCode,
				   const char* payloadData, size_t payloadSize);

	bool checkNullTerminated(const char* str, size_t size) const;
	QString getIpPortStr(const tcp::socket& socket) const;

	void copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const;
	uint8_t channelChar(E::Channel ch) const;

	void updateSignalStatesByChanges(const Network::GetAppSignalStateChangesReply& getStateChangesReply);

private:
	HostAddressPort m_listenIP;
	const AppSignals& m_appSignals;

	std::atomic<bool> m_running { false };
	std::thread m_serverThread;

	asio::io_context m_io;

	std::mutex m_acceptorMutex;
	std::shared_ptr<tcp::acceptor> m_acceptor;

	std::mutex m_sessionsMutex;
	std::set<SessionShared> m_sessions;

	std::mutex m_signalStatesMutex;
	std::vector<SimpleAppSignalState> m_signalStates;
	std::unordered_map<Hash, int> m_hashToIndex;

	std::mutex m_signalStateChangesMutex;
	std::deque<AGL::GwAppSignalState> m_signalStateChanges;

	std::atomic_bool m_connectedToAppDataSrv {false};

	static constexpr size_t CONTINUE_RECEIVE = std::numeric_limits<size_t>::max();
	static constexpr int BAD_INDEX = -1;
};
