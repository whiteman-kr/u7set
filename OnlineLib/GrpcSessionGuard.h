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

#include "SoftwareInfo.h"
#include "SoftwareSettings.h"

class GrpcSessionGuard
{
public:
	GrpcSessionGuard(const SoftwareInfo& severSwInfo,
					 bool allowAllClients,
					 const std::vector<ClientInfo>& clients,
					 bool checkHostName);
	virtual ~GrpcSessionGuard();

	//

	void start();
	void stop() noexcept;

	grpc::Status handshake(const Grpc::HandshakeRequest* request,
							Grpc::HandshakeReply* reply);

	bool extractAndValidateAuthToken(grpc::ServerContext* context, std::string* authToken = nullptr);
	std::string extractAuthTokenFromMetadata(grpc::ServerContext* context) const;

	void setSessionTimeout(int seconds);

	SoftwareInfo getSoftwareInfo(const std::string& authToken);
	QString getSoftwareEquipmentID(const std::string& authToken);

private:
bool validateAuthToken(const std::string& authToken);

	bool isValidClient(const Grpc::HandshakeRequest* request, std::string& errMsg) const;
	void sessionGuardLoop(std::stop_token stopToken) noexcept;

private:
	const SoftwareInfo m_serverSwInfo;
	std::atomic_bool m_allowAllClients = false;
	const std::vector<ClientInfo> m_clients;
	const bool m_checkHostName = false;

	static constexpr int SESSION_CHECK_PERIOD_SEC = 2;
	static constexpr int SESSION_TIMEOUT_SEC = 2 * 60;

	std::atomic<int> m_sessionTimeout = SESSION_TIMEOUT_SEC;

	//

	std::jthread m_sessionGuardThread;

	using TimePoint = std::chrono::steady_clock::time_point;

	std::mutex m_sessionsMutex;
	std::unordered_map<std::string, TimePoint> m_sessionExpirations;	// session authToken -> expire time
	std::unordered_map<std::string, SoftwareInfo> m_clientsInfo;		// session authToken -> client SoftwareInfo
};

inline const char* grpcStatusCodeToString(grpc::StatusCode code)
{
	switch (code)
	{
	case grpc::StatusCode::OK: return "OK";
	case grpc::StatusCode::CANCELLED: return "CANCELLED";
	case grpc::StatusCode::UNKNOWN: return "UNKNOWN";
	case grpc::StatusCode::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
	case grpc::StatusCode::DEADLINE_EXCEEDED: return "DEADLINE_EXCEEDED";
	case grpc::StatusCode::NOT_FOUND: return "NOT_FOUND";
	case grpc::StatusCode::ALREADY_EXISTS: return "ALREADY_EXISTS";
	case grpc::StatusCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
	case grpc::StatusCode::UNAUTHENTICATED: return "UNAUTHENTICATED";
	case grpc::StatusCode::RESOURCE_EXHAUSTED: return "RESOURCE_EXHAUSTED";
	case grpc::StatusCode::FAILED_PRECONDITION: return "FAILED_PRECONDITION";
	case grpc::StatusCode::ABORTED: return "ABORTED";
	case grpc::StatusCode::OUT_OF_RANGE: return "OUT_OF_RANGE";
	case grpc::StatusCode::UNIMPLEMENTED: return "UNIMPLEMENTED";
	case grpc::StatusCode::INTERNAL: return "INTERNAL";
	case grpc::StatusCode::UNAVAILABLE: return "UNAVAILABLE";
	case grpc::StatusCode::DATA_LOSS: return "DATA_LOSS";
	default: return "UNKNOWN_CODE";
	}
}
