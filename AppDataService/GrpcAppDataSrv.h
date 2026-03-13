#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcAppDataSrv.grpc.pb.h>
#include "../OnlineLib/GrpcSessionGuard.h"
#include "../OnlineLib/GrpcServer.h"

#include <CommonLib/HostAddressPort.h>

#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "DynamicAppSignalState.h"
#include "AppDataSource.h"
#include "AppDataReceiver.h"
#include "DiscretesLog.h"

class AppDataServiceSettings;

class GrpcAppDataSrv final : public GrpcServer, public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const HostAddressPort& listenIP,
							const AppDataSources& appDataSources,
							AppDataReceiver* appDataReceiver,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							std::shared_ptr<DiscretesLogWriter> dsLogWriter,
							CircularLoggerShared log);

	GrpcAppDataSrv(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv& operator=(const GrpcAppDataSrv&) = delete;
	GrpcAppDataSrv(GrpcAppDataSrv&&) = delete;
	GrpcAppDataSrv& operator=(GrpcAppDataSrv&&) = delete;

	~GrpcAppDataSrv();

	grpc::Status Handshake(grpc::ServerContext* context,
						const Grpc::HandshakeRequest* request,
						Grpc::HandshakeReply* reply) override;

	grpc::Status Ping(grpc::ServerContext* context,
					const Grpc::PingRequest* request,
					Grpc::PingReply* reply);

	grpc::Status GetAppSignalList(grpc::ServerContext* context,
								const Grpc::GetAppSignalListRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalListReply>* writer) override;

	grpc::Status GetAppSignalParam(grpc::ServerContext* context,
								const Grpc::GetAppSignalParamRequest* request,
								grpc::ServerWriter<Grpc::GetAppSignalParamReply>* writer) override;

	grpc::Status GetAppSignalState(grpc::ServerContext* context,
								const Grpc::GetAppSignalStateRequest* request,
								Grpc::GetAppSignalStateReply* reply) override;

	grpc::Status GetAppSignalStateConstSize(grpc::ServerContext* context,
								   const Grpc::GetAppSignalStateRequest* request,
								   Grpc::GetAppSignalStateReply* reply) override;

	grpc::Status GetAppSignalStateChanges(grpc::ServerContext* context,
								   const Grpc::GetAppSignalStateChangesRequest* request,
								   grpc::ServerWriter<Grpc::GetAppSignalStateChangesReply>* writer) override;

	grpc::Status GetDiscretesLog(grpc::ServerContext* context,
								const Grpc::GetDiscretesLogRequest* request,
								grpc::ServerWriter<Grpc::GetDiscretesLogReply>* writer) override;

	grpc::Status AckDiscretesLog(grpc::ServerContext* context,
								 const Grpc::AckDiscretesLogRequest* request,
								 Grpc::AckDiscretesLogReply* reply) override;

	// grpc::Status GetRtTrendsData(grpc::ServerContext* context,
	// 							 grpc::ServerReaderWriter<Grpc::GetRtTrendsDataReply,
	// 													  Grpc::GetRtTrendsDataRequest>* stream);

	grpc::Status GetAppDataSourcesInfo(grpc::ServerContext* context,
									   const Grpc::GetAppDataSourcesInfoRequest* request,
									   Grpc::GetAppDataSourcesInfoReply* reply) override;

	grpc::Status GetAppDataSourcesState(grpc::ServerContext* context,
										const Grpc::GetAppDataSourcesStateRequest* request,
										Grpc::GetAppDataSourcesStateReply* reply) override;

	grpc::Status GetServerTime(grpc::ServerContext* context,
								const Grpc::GetServerTimeRequest* request,
								Grpc::GetServerTimeReply* reply) override;
private:
	grpc::Status getAppSignalState(grpc::ServerContext* context,
								   const Grpc::GetAppSignalStateRequest* request,
								   Grpc::GetAppSignalStateReply* reply,
								   bool constSize);

	grpc::Service* getGrpcService() override;
	virtual QString serviceName() const override;

private:
	AppDataReceiver* m_appDataReceiver = nullptr;
	const AppDataSources& m_appDataSources;
	const AppSignals& m_appSignals;
	const DynamicAppSignalStates& m_signalStates;
	std::shared_ptr<DiscretesLogWriter> m_dsLogWriter;
};
