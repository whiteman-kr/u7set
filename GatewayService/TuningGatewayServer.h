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
#include <GatewayClientLib/TuningGwProtocol.hpp>
#include <GatewayClientLib/GwCrc32.hpp>

#include <CommonLib/HostAddressPort.h>
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"
#include <GrpcAppDataSrv.pb.h>
#include "AsyncTcpServer.h"
#include "TuningSrvClient.h"

using asio::ip::tcp;
namespace GCL = GatewayClientLib;

class TuningGatewaySession : public AsyncTcpSession
{
private:
	enum class WaitResult
	{
		QuitRequested,
		Timeout,
		DataReady
	};

public:
	TuningGatewaySession(const SoftwareInfo& swInfo, 
						 const AppSignals& appSignals,
						 const std::vector<HostAddressPort>& serviceAddresses, 
						 asio::ip::tcp::socket socket, 
						 CircularLoggerShared log);

	void setConnectedToTuningSrv(bool connected);

	void setReplyData(const char* data, size_t size);

protected:	
	virtual void onStarted();
	virtual void onStopped();

	virtual bool checkRequestID(uint32_t requestID) override;
	virtual bool isHandshakeRequest(uint32_t requestID) override;
	virtual bool checkPayloadSize(const GCL::GwMessageHeader& header,
								const char* recvBuf,
								const size_t recvBufSize,
								GCL::GwErrorCode& errCode) override;

	virtual bool processRequest(const GCL::GwMessageHeader& header, char* recvBuf, size_t requestSize) override;

private:
	bool processHandshakeRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processGetTuningSourcesStartRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processGetTuningSourcesNextRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processGetTuningSourceStatesRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processTuningSignalsReadRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processTuningSignalsWriteRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processTuningSignalsApplyRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processChangeControlledSourceRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);

	void startTuningSrvClient();
	void stopTuningSrvClient();

	WaitResult waitForOrQuit(const int64_t timeoutMs);

	void clearReplyData();

private:
	std::unique_ptr<TuningSrvClientThread> m_tunSrvClientThread;
	std::atomic_bool m_connectedToTuningSrv {false};

	std::vector<char> m_payload;
	quint64 m_readRequestID = 0;
	quint64 m_writeRequestID = 0;

	std::mutex m_condVarMutex;
	std::condition_variable m_condVar;
	std::vector<char> m_replyData;
};

using TcpSocketShared = std::shared_ptr<tcp::socket>;

struct TgsSession
{
	TcpSocketShared socket;
	std::thread thread;
	std::atomic_bool finished {false};
	std::atomic_bool closing { false };

	//

	bool handshakeCompleted = false;
	QString clientName;
	std::atomic_bool connectedToTuningSrv {false};
	std::vector<char> payloadData;
	size_t errCount = 0;

	std::unique_ptr<TuningSrvClientThread> tunSrvClientThread;

	//

	std::mutex condVarMutex;
	std::condition_variable condVar;
	std::vector<char> replyData;
};

using TgsSessionShared = std::shared_ptr<TgsSession>;

class TuningGatewayServer : public LogWrapper
{
public:
	static constexpr size_t MAX_SESSION_ERRORS = 100;

private:
	enum class WaitResult
	{
		QuitRequested,
		Timeout,
		DataReady
	};

public:
	TuningGatewayServer(const SoftwareInfo& swInfo,
						const HostAddressPort& listenIP,
						const HostAddressPort& tunSrvIP1,
						const HostAddressPort& tunSrvIP2,
						const AppSignals& appSignals,
						CircularLoggerShared log);
	virtual ~TuningGatewayServer();

	void start();
	void stop();

	void setConnectedToTuningSrv(bool connected);

private:
	void runAcceptLoop();
	void sessionThread(TgsSessionShared stc);
	void reapFinishedSessions();
	void closeSocket(TgsSessionShared stc);
	void requestCloseSession(TgsSessionShared stc);
	void closeSessions();
	void joinAllSessions();
	void resetAcceptor();

	void startTuningSrvClient(TgsSessionShared stc);
	void stopTuningSrvClient(TgsSessionShared stc);

	void processRequest(TgsSessionShared stc, char* recvBuf, size_t& recvBufSize);

	bool processHandshakeRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);

	bool processGetTuningSourcesStartRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
											 const char* recvBuf, const size_t requestSize);

	bool processGetTuningSourcesNextRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
											 const char* recvBuf, const size_t requestSize);

	bool processGetTuningSourceStatesRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
											const char* recvBuf, const size_t requestSize);

	bool processTuningSignalsReadRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
											 const char* recvBuf, const size_t requestSize);

	bool processTuningSignalsWriteRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
										 const char* recvBuf, const size_t requestSize);

	bool processTuningSignalsApplyRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
										  const char* recvBuf, const size_t requestSize);

	bool processChangeControlledSourceRequest(TgsSessionShared stc, const GCL::GwMessageHeader& header,
										  const char* recvBuf, const size_t requestSize);

	bool checkPayloadSize(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t recvBufSize, GCL::GwErrorCode& errCode);
	[[nodiscard]] size_t skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize);

	void sendErrReply(TgsSessionShared stc, const GatewayClientLib::GwMessageHeader& requestHeader, GCL::GwErrorCode errCode);
	void sendOkReply(TgsSessionShared stc, const GCL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize);
	void sendReply(TgsSessionShared stc,
				   uint32_t requestID, GCL::GwErrorCode errCode,
				   const char* payloadData, size_t payloadSize);

	bool checkNullTerminated(const char* str, size_t size) const;
	QString getIpPortStr(const tcp::socket& socket) const;

	void copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const;
	void copyStr(char* toStr, size_t toStrLen, const std::string& fromStr) const;

	uint8_t channelChar(E::Channel ch) const;

	WaitResult waitForOrQuit(TgsSessionShared stc, const int64_t timeoutMs);

	bool isQuitRequested(TgsSessionShared stc) const;

private:
	SoftwareInfo m_swInfo;
	HostAddressPort m_listenIP;
	HostAddressPort m_tunSrvIP1;
	HostAddressPort m_tunSrvIP2;
	const AppSignals& m_appSignals;

	std::atomic<bool> m_running { false };
	std::thread m_serverThread;

	asio::io_context m_io;

	std::mutex m_acceptorMutex;
	std::shared_ptr<tcp::acceptor> m_acceptor;

	std::mutex m_sessionsMutex;
	std::set<TgsSessionShared> m_sessions;

	std::atomic_bool m_connectedToTuningSrv {false};

	static constexpr size_t CONTINUE_RECEIVE = std::numeric_limits<size_t>::max();
	static constexpr int BAD_INDEX = -1;
};
