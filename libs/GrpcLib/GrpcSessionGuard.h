#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <QtGlobal>
#include <CommonLib/Types.h>

#include <grpcpp/grpcpp.h>
#include <GrpcAppDataSrv.grpc.pb.h>

#include "../../OnlineLib/SoftwareInfo.h"
#include "../../OnlineLib/SoftwareSettings.h"

class GrpcSessionGuard
{
public:
	GrpcSessionGuard(const SoftwareInfo& severSwInfo,
					 const std::vector<ClientInfo>& clients,
					 bool checkHostName);
	virtual ~GrpcSessionGuard();

	//

	void start();
	void stop() noexcept;

	bool handshake(const Grpc::HandshakeRequest* request,
					Grpc::HandshakeReply* reply);

	bool extractAndValidateAuthToken(grpc::ServerContext* context);

	void setAllowAllClients(bool allowAll);

private:
	std::string extractAuthTokenFromMetadata(grpc::ServerContext* context) const;
	bool validateAuthToken(const std::string& authToken);

	bool isValidClient(const Grpc::HandshakeRequest* request) const;
	void sessionGuardLoop(std::stop_token stopToken) noexcept;

private:
	const SoftwareInfo m_serverSwInfo;
	const std::vector<ClientInfo> m_clients;
	const bool m_checkHostName = false;

	static constexpr int SESSION_CHECK_PERIOD_SEC = 2;
	static constexpr int SESSION_TIMEOUT_SEC = 2 * 60;
	//
	std::atomic_bool m_allowAllClients = false;

	std::jthread m_sessionGuardThread;

	using TimePoint = std::chrono::steady_clock::time_point;

	std::mutex m_sessionsMutex;
	std::unordered_map<std::string, TimePoint> m_sessionExpirations;	// session authToken -> expire time
	std::unordered_map<std::string, SoftwareInfo> m_clientsInfo;		// session authToken -> client SoftwareInfo
};
