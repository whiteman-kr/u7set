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