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
#include <GatewayClientLib/AdsGwProtocol.hpp>
#include <GatewayClientLib/GwCrc32.hpp>

#include <CommonLib/HostAddressPort.h>
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/GrpcAdsClient.h"
#include "AsyncTcpServer.h"

using asio::ip::tcp;
namespace GCL = GatewayClientLib;

class AdsGatewaySession;

class AdsGatewaySessionAppSignalStateUpdater : public IAppSignalStateUpdater
{
public:
	AdsGatewaySessionAppSignalStateUpdater(AdsGatewaySession& session);

	virtual void adsConnected() override;
	virtual void adsDisconnected() override;

	virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override;
	virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override;
	virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override;

private:
	AdsGatewaySession& m_session;
};

class AdsGatewaySession : public AsyncTcpSession
{
public:
	explicit AdsGatewaySession(const SoftwareInfo& swInfo,
							 const AppSignals& appSignals,
							 const std::vector<HostAddressPort>& serviceAddresses,
							 asio::ip::tcp::socket socket,
							 CircularLoggerShared log);

	virtual ~AdsGatewaySession();

	void setConnectedToAppDataSrv(bool connected);

	void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply);
	void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply);
	void invalidateSignals();

protected:
	void startGrpcAppDataSrvClient(); 
	void stopGrpcAppDataSrvClient(); 

	virtual void onStarted() override;
	virtual void onStopped() override;
	virtual bool checkRequestID(uint32_t requestID) override;
	virtual bool isHandshakeRequest(uint32_t requestID) override;
	virtual bool checkPayloadSize(const GCL::GwMessageHeader& header,
								const char* recvBuf,
								const size_t recvBufSize,
								GCL::GwErrorCode& errCode) override;

	virtual bool processRequest(const GCL::GwMessageHeader& header, char* recvBuf, size_t requestSize) override;

	bool processHandshakeRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processSignalListStartRequest(const GCL::GwMessageHeader& header,const char* recvBuf, const size_t requestSize);
	bool processSignalListNextRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processSignalParamStartRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processSignalParamNextRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processSignalStateRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);
	bool processSignalStateChangesRequest(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t requestSize);

private:
	void updateSignalStatesByChanges(const Grpc::GetAppSignalStateChangesReply& reply);

private:
	std::atomic_bool m_connectedToAppDataSrv{false};

	std::unique_ptr<GrpcAdsClient > m_adsClient;

	std::mutex m_signalStatesMutex;
	std::vector<SimpleAppSignalState> m_signalStates;
	std::unordered_map<Hash, int> m_hashToIndex;
	std::vector<int> m_indexes;

	std::mutex m_signalStateChangesMutex;
	std::deque<GCL::GwAppSignalState> m_signalStateChanges;

	std::vector<char> m_payload;
};

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

	void start();
	void stop();

	void updateSignalStates(const Grpc::GetAppSignalStateReply& getStatesReply);
	void processStateChanges(const Grpc::GetAppSignalStateChangesReply& getStateChangesReply);
	void invalidateSignals();

	void setConnectedToAppDataSrv(bool connected);

private:
	void runAcceptLoop();
	void sessionThread(SessionShared stc);
	void reapFinishedSessions();
	void closeSocket(SessionShared stc);
	void requestCloseSession(SessionShared stc);
	void closeSessions();
	void joinAllSessions();
	void resetAcceptor();

	void processRequest(SessionShared stc, char* recvBuf, size_t& recvBufSize);

	bool processHandshakeRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalListStartRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalListNextRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalParamStartRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalParamNextRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										const char* recvBuf, const size_t requestSize);
	bool processSignalStateRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										 const char* recvBuf, const size_t requestSize);
	bool processSignalStateChangesRequest(SessionShared stc, const GCL::GwMessageHeader& header,
										 const char* recvBuf, const size_t requestSize);

	bool checkPayloadSize(const GCL::GwMessageHeader& header, const char* recvBuf, const size_t recvBufSize, GCL::GwErrorCode& errCode);
	[[nodiscard]] size_t skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize);

	void sendErrReply(SessionShared stc, const GatewayClientLib::GwMessageHeader& requestHeader, GCL::GwErrorCode errCode);
	void sendOkReply(SessionShared stc, const GCL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize);
	void sendReply(SessionShared stc,
				   uint32_t requestID, GCL::GwErrorCode errCode,
				   const char* payloadData, size_t payloadSize);

	bool checkNullTerminated(const char* str, size_t size) const;
	QString getIpPortStr(const tcp::socket& socket) const;

	void copyStr(char* toStr, size_t toStrLen, const QString& fromStr) const;

	uint8_t channelChar(E::Channel ch) const;

	void updateSignalStatesByChanges(const Grpc::GetAppSignalStateChangesReply& getStateChangesReply);

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
	std::deque<GCL::GwAppSignalState> m_signalStateChanges;

	std::atomic_bool m_connectedToAppDataSrv {false};

	static constexpr size_t CONTINUE_RECEIVE = std::numeric_limits<size_t>::max();
	static constexpr int BAD_INDEX = -1;
};
