#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>
#include <GrpcSessionGuard.h>

#include <CommonLib/HostAddressPort.h>

#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "DynamicAppSignalState.h"

class AppDataServiceSettings;

class GrpcAppDataSrv final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const std::vector<HostAddressPort>& listenIPs,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							CircularLoggerShared log);

	explicit GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const HostAddressPort& listenIP,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							CircularLoggerShared log);

	GrpcAppDataSrv(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv& operator=(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv(GrpcAppDataSrv&&) = delete;
	GrpcAppDataSrv& operator=(GrpcAppDataSrv&&) = delete;

	~GrpcAppDataSrv();

	void setSessionTimeout(int seconds);

	grpc::Status Handshake(grpc::ServerContext* context,
						const Grpc::HandshakeRequest* request,
						Grpc::HandshakeReply* reply) override;

	grpc::Status GetAppSignalList(grpc::ServerContext* context,
								const Grpc::GetAppSignalListRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalListReply>* writer) override;

	grpc::Status GetAppSignalParam(grpc::ServerContext* context,
								const Grpc::GetAppSignalParamRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalParamReply>* writer) override;

	grpc::Status GetAppSignalState(grpc::ServerContext* context,
								const Grpc::GetAppSignalStateRequest* request,
								Grpc::GetAppSignalStateReply* reply) override;

	grpc::Status GetAppSignalStateChanges(grpc::ServerContext* context,
								   const Grpc::GetAppSignalStateChangesRequest* request,
								   grpc::ServerWriter<Grpc::GetAppSignalStateChangesReply>* writer) override;
private:
	void initService(const std::vector<HostAddressPort>& listenIPs);

private:
	const AppSignals& m_appSignals;
	const DynamicAppSignalStates& m_signalStates;
	CircularLoggerShared m_log;
	std::unique_ptr<grpc::Server> m_server;
	std::jthread m_thread;

	GrpcSessionGuard m_sessionGuard;
};
