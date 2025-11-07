#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>

#include <CommonLib/HostAddressPort.h>

#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicAppSignalState.h"

class AppDataServiceSettings;

class GrpcAppDataSrv final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const std::vector<HostAddressPort>& listenIPs,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							CircularLoggerShared log);

	explicit GrpcAppDataSrv(const HostAddressPort& listenIP,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							CircularLoggerShared log);

	GrpcAppDataSrv(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv& operator=(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv(GrpcAppDataSrv&&) = delete;
	GrpcAppDataSrv& operator=(GrpcAppDataSrv&&) = delete;

	~GrpcAppDataSrv();

	grpc::Status GetAppSignalList(grpc::ServerContext* context,
								const Grpc::GetAppSignalListRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalListReply>* writer) override;

	grpc::Status GetAppSignalParam(grpc::ServerContext* context,
								const Grpc::GetAppSignalParamRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalParamReply>* writer) override;

	grpc::Status GetAppSignalState(grpc::ServerContext* context,
								const Grpc::GetAppSignalStateRequest* request,
								Grpc::GetAppSignalStateReply* reply) override;

private:
	void initService(const std::vector<HostAddressPort>& listenIPs);
private:
	const AppSignals& m_appSignals;
	const DynamicAppSignalStates& m_signalStates;
	CircularLoggerShared m_log;
	std::unique_ptr<grpc::Server> m_server;
	std::jthread m_thread;
};
