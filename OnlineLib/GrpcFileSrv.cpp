#include "GrpcFileSrv.h"

// -------------------------------------------------------------------------------------
//
// GrpcFileSrv class implementation
//
// -------------------------------------------------------------------------------------

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
	return GrpcServer::handshake(context, request, reply);
}

grpc::Status GrpcFileSrv::Ping(grpc::ServerContext* context,
							const Grpc::PingRequest* request,
							Grpc::PingReply* reply)
{
	return GrpcServer::ping(context, request, reply);
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

	std::string authToken;

	if (m_sessionGuard.extractAndValidateAuthToken(context, &authToken) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	Q_ASSERT(authToken.empty() == false);

	//

	WriteReplyPtr<Grpc::GetFileReply> writeReply = &WriteReplyFunc<Grpc::GetFileReply>;
	grpc::Status writeStatus;
	Grpc::GetFileReply reply;

	const QString swEquipmentID = m_sessionGuard.getSoftwareEquipmentID(authToken);

	constexpr int FILE_CHUNK_SIZE = 1024 * 1024;		// 1 Mb
	constexpr int FILE_MAX_SIZE = 100 * 1024 * 1024;	// 100 Mb

	//

	reply.set_filename(request->filename());

	QString fileName = QString::fromStdString(request->filename());

	QString cleanFileName = getCleanFileName(m_rootFolder, fileName);

	if (!cleanFileName.startsWith(m_rootFolder))
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::FileIsNotAccessible));

		logErr(QString("Access denied to file %1 (request from %2)").
							 arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	//

	QFile file(cleanFileName);
	QFileInfo fi(file);

	if (file.exists() == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::RemoteFileIsNotExists));

		logErr(QString("File %1 not exists (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (fi.size() > FILE_MAX_SIZE)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::FileTooBig));

		logErr(QString("File %1 too big (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus,getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::CantOpenRemoteFile));

		logErr(QString("Can't open file %1 (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	QByteArray fileData = file.readAll();

	file.close();

	if (fi.size() != fileData.size())
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::CantReadRemoteFile));

		logErr(QString("Can't read file %1 (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	QString md5;

	if (checkFile(fileName, fileData, md5) == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::FileDataCorrupted));

		logErr(QString("File %1 check error (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	qsizetype sendDataSize = 0;

	int totalParts = 1;
	int curPart = 1;

	if (fileData.size() > 0)
	{
		totalParts = fileData.size() / FILE_CHUNK_SIZE + (fileData.size() % FILE_CHUNK_SIZE ? 1 : 0);
	}

	do
	{
		reply.set_filename(request->filename());
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::Ok));
		reply.set_filesize(fileData.size());
		reply.set_currentpart(curPart);
		reply.set_totalparts(totalParts);
		reply.set_md5(md5.toStdString());

		int partSize = 0;

		if (fileData.size() - sendDataSize > FILE_CHUNK_SIZE)
		{
			partSize = FILE_CHUNK_SIZE;
		}
		else
		{
			partSize = fileData.size() - sendDataSize;
		}

		if (fileData.size() == 0)
		{
			reply.set_filedata("", 0);
		}
		else
		{
			reply.set_filedata(fileData.constData() + sendDataSize, partSize);
		}

		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
		{
			return writeStatus;
		}

		curPart++;
		sendDataSize += partSize;
	}
	while(sendDataSize < fileData.size());

	logMsg(QString("File %1 sent to %2").arg(fileName).arg(swEquipmentID));

	return grpc::Status::OK;
}

grpc::Status GrpcFileSrv::GetSessionParams(grpc::ServerContext* context,
											const Grpc::GetSessionParamsRequest* request,
											Grpc::GetSessionParamsReply* reply)
{
	if (context == nullptr ||
		request == nullptr ||
		reply == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	std::string authToken;

	if (m_sessionGuard.extractAndValidateAuthToken(context, &authToken) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	getSessionParams(reply->mutable_sessionparams());

	return grpc::Status::OK;
}

QString GrpcFileSrv::rootFolder() const
{
	return m_rootFolder;
}

bool GrpcFileSrv::checkFile(const QString& pathFileName, const QByteArray& fileData, QString& md5) const
{
	Q_UNUSED(pathFileName);

	md5 = Md5Hash::hashStr(fileData);

	return true;
}

void GrpcFileSrv::getSessionParams(Network::SessionParams* params) const
{
	SessionParams sp;

	sp.currentSettingsProfile = SettingsProfile::DEFAULT;
	sp.softwareRunMode = E::SoftwareRunMode::Normal;

	sp.saveTo(params);
}

grpc::Service* GrpcFileSrv::getGrpcService()
{
	return this;
}

QString GrpcFileSrv::serviceName() const
{
	return QStringLiteral("GrpcFileSrv");
}

QString GrpcFileSrv::getCleanFileName(const QString& rootFolder, const QString& fileName)
{
	QString filePathName = rootFolder + fileName;

	return QDir::cleanPath(filePathName);
}
