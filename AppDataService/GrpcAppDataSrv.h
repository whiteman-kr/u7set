#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>

#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/CircularLogger.h"

class AppDataServiceSettings;

class GrpcAppDataSrv final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const AppDataServiceSettings& settings,
							const AppSignals& appSignals,
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

private:
	const AppSignals& m_appSignals;
	CircularLoggerShared m_log;
	std::unique_ptr<grpc::Server> m_server;
	std::jthread m_thread;
};
