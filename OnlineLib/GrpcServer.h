#pragma once

#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>
#include "GrpcSessionGuard.h"

#include <CommonLib/HostAddressPort.h>

#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"

class AppDataServiceSettings;

class GrpcServer final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcServer(const SoftwareInfo& serverSwInfo,
						bool allowAllClients,
						const std::vector<ClientInfo>& clients,
						bool checkHostName,
						const HostAddressPort& listenIP,
						CircularLoggerShared log);

	GrpcServer(const GrpcServer&) = delete;
	GrpcServer& operator=(const GrpcServer&) = delete;
	GrpcServer(GrpcServer&&) = delete;
	GrpcServer& operator=(GrpcServer&&) = delete;

	~GrpcServer();

	void setSessionTimeout(int seconds);
	bool isBinded() const;

	grpc::Status Handshake(grpc::ServerContext* context,
						   const Grpc::HandshakeRequest* request,
						   Grpc::HandshakeReply* reply) override;
private:
	void initService(const HostAddressPort& listenIP);

private:
	CircularLoggerShared m_log;
	std::unique_ptr<grpc::Server> m_server;
	std::atomic_bool m_stopRequested {false};
	std::thread m_thread;
	std::atomic_bool m_binded {false};

	GrpcSessionGuard m_sessionGuard;
};
