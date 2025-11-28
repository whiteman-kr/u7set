#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcFileSrv.grpc.pb.h>
#include "../OnlineLib/GrpcSessionGuard.h"
#include "../OnlineLib/GrpcServer.h"

#include <CommonLib/HostAddressPort.h>

#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/SoftwareSettings.h"

class AppDataServiceSettings;

class GrpcFileSrv final : public GrpcServer, public Grpc::FileSrv::Service
{
public:
	explicit GrpcFileSrv(const SoftwareInfo& serverSwInfo,
							bool allowAllClients,
							const std::vector<ClientInfo>& clients,
							bool checkHostName,
							const HostAddressPort& listenIP,
							const QString& rootFolder,
							CircularLoggerShared log);

	GrpcFileSrv(const GrpcFileSrv&) = delete;
	GrpcFileSrv& operator=(const GrpcFileSrv&) = delete;
	GrpcFileSrv(GrpcFileSrv&&) = delete;
	GrpcFileSrv& operator=(GrpcFileSrv&&) = delete;

	~GrpcFileSrv();

	grpc::Status Handshake(grpc::ServerContext* context,
						   const Grpc::HandshakeRequest* request,
						   Grpc::HandshakeReply* reply) override;

	grpc::Status GetFile(grpc::ServerContext* context,
						const Grpc::GetFileRequest* request,
						grpc::ServerWriter<Grpc::GetFileReply>* writer) override;
protected:
	virtual bool checkFile(const QString& pathFileName, const QByteArray& fileData) const;

private:
	grpc::Service* getGrpcService() override;
	virtual QString serviceName() const override;

private:
	const QString m_rootFolder;

};
