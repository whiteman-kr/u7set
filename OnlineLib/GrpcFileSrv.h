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

// -------------------------------------------------------------------------------------
//
// GrpcFileBase class declaration
//
// -------------------------------------------------------------------------------------

class GrpcFileBase
{
public:
	GrpcFileBase(const QString& rootFolder);

	void setRootFolder(const QString& rootFolder);
	QString rootFolder() const;

	static QByteArray getMd5(const QByteArray& data);

protected:
	static QString getCleanRoot(const QString& rootFolder);
	static QString getCleanFileName(const QString& rootFolder, const QString& fileName);

private:
	mutable std::mutex m_mutex;
	QString m_rootFolder;
};

// -------------------------------------------------------------------------------------
//
// GrpcFileSrv class declaration
//
// -------------------------------------------------------------------------------------

class GrpcFileSrv : public GrpcFileBase, public GrpcServer, public Grpc::FileSrv::Service
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
};

struct FileReady
{
	QString fileName;
	Tcp::FileTransferResult errorCode;
	QByteArray fileData;
	QByteArray md5;
};

// -------------------------------------------------------------------------------------
//
// GrpcFileClient class declaration
//
// -------------------------------------------------------------------------------------

class GrpcFileClient : public GrpcFileBase
{
public:
	GrpcFileClient(const SoftwareInfo& softwareInfo,
				   const std::vector<HostAddressPort>& serverAddress,
				   const QString& rootFolder,
				   const QString& clientDescription,
				   CircularLoggerShared log);

	GrpcFileClient(const GrpcFileClient&) = delete;
	GrpcFileClient& operator=(const GrpcFileClient&) = delete;
	GrpcFileClient(GrpcFileClient&&) = delete;
	GrpcFileClient& operator=(GrpcFileClient&&) = delete;

	virtual ~GrpcFileClient();

	void downloadFile(const QString& fileName);
	bool waitFileReady(FileReady* fileReady);

	bool downloadFileBlocked(const QString& fileName, FileReady* fileReady);

	bool isTransferInProgress();

protected:
	QString getErrorStr(Tcp::FileTransferResult errorCode) const;

private:
	void run();

	void createStubAndHandshake(grpc::Status* status = nullptr);
	Tcp::FileTransferResult privateDownloadFile(const QString& fileName);

	void pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode);
	void pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode,
					   QByteArray& fileData, QByteArray& md5);	// not const - OK!

private:
	SoftwareInfo m_swInfo;
	std::vector<HostAddressPort> m_serverAddress;
	CircularLoggerShared m_log;

	int m_srvAddrIndex = -1;			// !

	std::mutex m_procMutex;
	std::condition_variable m_procCond;
	QStringList m_downloadFileQueue;
	std::atomic_bool m_quitRequested {false};
	std::atomic_bool m_transferInProgress {false};

	std::mutex m_fileReadyMutex;
	std::condition_variable m_fileReadyCond;
	std::queue<FileReady> m_fileReadyQueue;

	std::thread m_thread;
	std::unique_ptr<Grpc::FileSrv::Stub> m_stub;
	std::string m_authToken;
};
