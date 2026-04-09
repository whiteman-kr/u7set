#pragma once

#include <memory>
#include <thread>
#include <queue>
#include <chrono>
#include <mutex>
#include <grpcpp/grpcpp.h>

#include <QString>

#include <GrpcFileSrv.grpc.pb.h>

#include <CommonLib/HostAddressPort.h>
#include <CommonLib/Hash.h>

#include "GrpcSessionGuard.h"
#include "GrpcServer.h"
#include "GrpcClient.h"
#include "CircularLogger.h"
#include "SoftwareSettings.h"
#include "TcpFileTransfer.h"

// -------------------------------------------------------------------------------------
//
// GrpcFileSrv class declaration
//
// -------------------------------------------------------------------------------------

class GrpcFileSrv : public GrpcServer, public Grpc::FileSrv::Service
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

	grpc::Status Ping(grpc::ServerContext* context,
					const Grpc::PingRequest* request,
					Grpc::PingReply* reply) override;

	grpc::Status GetFile(grpc::ServerContext* context,
						const Grpc::GetFileRequest* request,
						grpc::ServerWriter<Grpc::GetFileReply>* writer) override;

	grpc::Status GetSessionParams(grpc::ServerContext* context,
								const Grpc::GetSessionParamsRequest* request,
								Grpc::GetSessionParamsReply* reply);

	QString rootFolder() const;

protected:
	virtual bool checkFile(const QString& pathFileName, const QByteArray& fileData, QString& md5) const;
	virtual void getSessionParams(Network::SessionParams* params) const;

private:
	grpc::Service* getGrpcService() override;
	virtual QString serviceName() const override;

	static QString getCleanFileName(const QString& rootFolder, const QString& fileName);

private:
	QString m_rootFolder;
};
