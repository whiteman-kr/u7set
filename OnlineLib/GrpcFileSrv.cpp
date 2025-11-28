#include "GrpcFileSrv.h"
#include "TcpFileTransfer.h"

GrpcFileSrv::GrpcFileSrv(const SoftwareInfo& serverSwInfo,
						 bool allowAllClients,
						 const std::vector<ClientInfo>& clients,
						 bool checkHostName,
						 const HostAddressPort& listenIP,
						 const QString& rootFolder,
						 CircularLoggerShared log) :
	GrpcServer(serverSwInfo, allowAllClients, clients, checkHostName, listenIP, log),
	m_rootFolder(rootFolder)
{
	start();
}

GrpcFileSrv::~GrpcFileSrv()
{
	stop();
}

grpc::Status GrpcFileSrv::Handshake(grpc::ServerContext* context,
									const Grpc::HandshakeRequest* request,
									Grpc::HandshakeReply* reply)
{
	Q_UNUSED(context);

	return m_sessionGuard.handshake(request, reply);
}

grpc::Status GrpcFileSrv::GetFile(	grpc::ServerContext* context,
									const Grpc::GetFileRequest* request,
									grpc::ServerWriter<Grpc::GetFileReply>* writer)
{
	if (context == nullptr ||
		request == nullptr ||
		writer == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	//

	WriteReplyPtr<Grpc::GetFileReply> writeReply = &WriteReplyFunc<Grpc::GetFileReply>;
	grpc::Status writeStatus;
	Grpc::GetFileReply reply;

	constexpr int FILE_CHUNK_SIZE = 1024 * 1024;		// 1 Mb

	//

	QString fileName = QString::fromStdString(request->filename());

	QFile file;

	file.setFileName(m_rootFolder + fileName);

	if (file.exists() == false)
	{
		reply.set_errorcode(static_cast<int>(Tcp::FileTransferResult::RemoteFileIsNotExists));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		reply.set_errorcode(static_cast<int>(Tcp::FileTransferResult::CantOpenRemoteFile));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	QFileInfo fi(file);

	reply.set_filesize(fi.size());

	int totalParts = ROUND_TO(fi.size(), FILE_CHUNK_SIZE);

	reply.set_totalparts(totalParts);

	QByteArray fileData = file.readAll();

	file.close();

	if (reply.filesize() != 0 && fileData.size() == 0)
	{
		// file reading error!
		reply.set_errorcode(static_cast<int>(Tcp::FileTransferResult::CantReadRemoteFile));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (checkFile(fileName, fileData) == false)
	{
		// file reading error!
		reply.set_errorcode(static_cast<int>(Tcp::FileTransferResult::FileDataCorrupted));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	// while()
	// {

	// }

	return grpc::Status::OK;
}

bool GrpcFileSrv::checkFile(const QString& pathFileName, const QByteArray& fileData) const
{
	Q_UNUSED(pathFileName);
	Q_UNUSED(fileData);
	return true;
}

grpc::Service* GrpcFileSrv::getGrpcService()
{
	return this;
}

QString GrpcFileSrv::serviceName() const
{
	return QStringLiteral("GrpcFileSrv");
}

