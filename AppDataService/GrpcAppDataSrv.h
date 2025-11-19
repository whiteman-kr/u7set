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
#include "AppDataReceiver.h"
#include "DiscretesLog.h"

class AppDataServiceSettings;

class GrpcAppDataSrv final : public Grpc::AppDataSrv::Service
{
public:
	explicit GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const std::vector<HostAddressPort>& listenIPs,
							AppDataReceiver* appDataReceiver,
							const AppSignals& appSignals,
							const DynamicAppSignalStates& signalStates,
							std::shared_ptr<DiscretesLogWriter> dsLogWriter,
							CircularLoggerShared log);

	explicit GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const HostAddressPort& listenIP,
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

	grpc::Status GetDiscretesLog(grpc::ServerContext* context,
								const Grpc::GetDiscretesLogRequest* request,
								grpc::ServerWriter<Grpc::GetDiscretesLogReply>* writer) override;

	grpc::Status AckDiscretesLog(grpc::ServerContext* context,
								 const Grpc::AckDiscretesLogRequest* request,
								 Grpc::AckDiscretesLogReply* reply) override;

private:
	void initService(const std::vector<HostAddressPort>& listenIPs);

private:
	AppDataReceiver* m_appDataReceiver = nullptr;
	const AppSignals& m_appSignals;
	const DynamicAppSignalStates& m_signalStates;
	std::shared_ptr<DiscretesLogWriter> m_dsLogWriter;
	CircularLoggerShared m_log;
	std::unique_ptr<grpc::Server> m_server;
	std::jthread m_thread;

	GrpcSessionGuard m_sessionGuard;
};
