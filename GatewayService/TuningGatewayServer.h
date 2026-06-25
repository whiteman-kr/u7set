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
