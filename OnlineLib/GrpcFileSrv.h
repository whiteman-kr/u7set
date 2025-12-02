#pragma once

#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>

#include <GrpcFileSrv.grpc.pb.h>
#include "../OnlineLib/GrpcSessionGuard.h"
#include "../OnlineLib/GrpcServer.h"
#include "TcpFileTransfer.h"

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

struct FileReady
{
	QString fileName;
	Tcp::FileTransferResult errorCode;
	QByteArray fileData;
};

class GrpcFileClient
{
public:
	GrpcFileClient(const SoftwareInfo& softwareInfo,
				   const std::vector<HostAddressPort>& serverAddress,
				   const QString& rootFolder,
				   const QString& clientDescription,
				   CircularLoggerShared log);

	virtual ~GrpcFileClient();

	void downloadFile(const QString& fileName);
	bool waitFileReady(FileReady* fileReady);

	void setRootFolder(const QString& rootFolder);

	virtual void onEndFileDownload(const QString fileName,
								   Tcp::FileTransferResult errorCode,
								   const QString md5);

	bool isTransferInProgress();

protected:
	QString getErrorStr(Tcp::FileTransferResult errorCode) const;

private:
	void run();

	void createStubAndHandshake(grpc::Status* status = nullptr);
	Tcp::FileTransferResult privateDownloadFile(const QString& fileName);

private:
	SoftwareInfo m_swInfo;
	std::vector<HostAddressPort> m_serverAddress;
	CircularLoggerShared m_log;

	int m_srvAddrIndex = -1;			// !

	std::mutex m_procMutex;
	std::condition_variable m_procCondition;
	QStringList m_downloadFileQueue;
	std::atomic_bool m_quitRequested {false};
	std::atomic_bool m_transferInProgress {false};

	std::mutex m_mutex;
	QString m_rootFolder;
	std::queue<FileReady> m_fileReadyQueue;

	std::thread m_thread;
	std::unique_ptr<Grpc::FileSrv::Stub> m_stub;
	std::string m_authToken;
};

