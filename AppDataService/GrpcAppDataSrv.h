#pragma once

#include <grpcpp/grpcpp.h>

#include "../libs/GrpcLib/GrpcAppDataSrv.grpc.pb.h"

#include "../OnlineLib/CircularLogger.h"

using namespace grpc;

class AppDataServiceSettings;

class GrpcAppDataSrv final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const AppDataServiceSettings& settings,
							CircularLoggerShared log);
	~GrpcAppDataSrv();

	Status GetAppSignalList(ServerContext* context,
							 const Grpc::GetAppSignalListRequest* request,
							 ServerWriter<Grpc::GetAppSignalListReply>* writer) override;

	Status GetAppSignalParam(ServerContext* context,
							 const Grpc::GetAppSignalParamRequest* request,
							 ServerWriter<Grpc::GetAppSignalParamReply>* writer) override;

private:
	CircularLoggerShared m_log;
	std::unique_ptr<Server> m_server;
	std::jthread m_thread;
};
