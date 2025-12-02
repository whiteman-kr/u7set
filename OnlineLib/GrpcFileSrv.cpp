#include "GrpcFileSrv.h"

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

	//

	reply.set_filename(request->filename());

	QString fileName = QString::fromStdString(request->filename());

	QFile file;

	file.setFileName(m_rootFolder + fileName);

	if (file.exists() == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::RemoteFileIsNotExists));

		DEBUG_LOG_ERR(m_log, QString("File %1 not exists (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::CantOpenRemoteFile));

		DEBUG_LOG_ERR(m_log, QString("Can't open file %1 (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	QFileInfo fi(file);

	QByteArray fileData = file.readAll();

	file.close();

	if (fi.size() != fileData.size())
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::CantReadRemoteFile));

		DEBUG_LOG_ERR(m_log, QString("Can't read file %1 (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	if (checkFile(fileName, fileData) == false)
	{
		reply.set_errorcode(TO_INT(Tcp::FileTransferResult::FileDataCorrupted));

		DEBUG_LOG_ERR(m_log, QString("File %1 check error (request from %2)").arg(fileName).arg(swEquipmentID));

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		return grpc::Status::OK;
	}

	QCryptographicHash md5Gen(QCryptographicHash::Md5);

	md5Gen.addData(QByteArrayView(fileData.constData(), fileData.size()));

	QByteArray md5 = md5Gen.result();

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
		reply.set_md5(md5.constData(), md5.size());

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

		if (writeReply(context, writer, reply, writeStatus, m_log) == false)
		{
			return writeStatus;
		}

		curPart++;
		sendDataSize += partSize;
	}
	while(sendDataSize < fileData.size());

	DEBUG_LOG_MSG(m_log, QString("File %1 sent to %2").arg(fileName).arg(swEquipmentID));

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

//

GrpcFileClient::GrpcFileClient(const SoftwareInfo& softwareInfo,
								const std::vector<HostAddressPort>& serverAddress,
								const QString& rootFolder,
								const QString& clientDescription,
								CircularLoggerShared log) :
	m_swInfo(softwareInfo),
	m_serverAddress(serverAddress),
	m_rootFolder(rootFolder),
	m_log(log)
{
	Q_UNUSED(clientDescription);

	if (m_serverAddress.size() == 0)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread = std::thread(&GrpcFileClient::run, this);
}

GrpcFileClient::~GrpcFileClient()
{
	m_quitRequested.store(true, std::memory_order::relaxed);
	m_procCondition.notify_one();

	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void GrpcFileClient::downloadFile(const QString& fileName)
{
	{
		std::lock_guard lg(m_procMutex);
		m_downloadFileQueue.append(fileName);
	}

	m_procCondition.notify_one();
}

bool GrpcFileClient::waitFileReady(FileReady* fileReady)
{
	TEST_PTR_RETURN_FALSE(fileReady);

	while(true)
	{
		{
			std::lock_guard lg(m_mutex);

			if (m_fileReadyQueue.empty() == false)
			{
				FileReady& fr = m_fileReadyQueue.front();

				fileReady->errorCode = fr.errorCode;
				fr.fileName.swap(fileReady->fileName);
				fr.fileData.swap(fileReady->fileData);

				m_fileReadyQueue.pop();

				return true;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// check wait time
	}

	return false;
}

void GrpcFileClient::setRootFolder(const QString& rootFolder)
{
	std::lock_guard lg(m_mutex);
	m_rootFolder = rootFolder;
}

void GrpcFileClient::onEndFileDownload(const QString fileName,
									   Tcp::FileTransferResult errorCode,
									   const QString md5)
{
}

bool GrpcFileClient::isTransferInProgress()
{
	return m_transferInProgress.load(std::memory_order::relaxed);
}

void GrpcFileClient::run()
{
	std::unique_lock ul(m_procMutex, std::defer_lock);

	while(m_quitRequested == false)
	{
		if (m_authToken.empty())
		{
			createStubAndHandshake();

			if (m_authToken.empty())
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
		}

		ul.lock();

		m_procCondition.wait(ul, [this]() -> bool
						   {
							   return m_quitRequested || !m_downloadFileQueue.empty();
						   });

		if (m_quitRequested == true)
		{
			ul.unlock();
			break;
		}

		if (m_downloadFileQueue.empty())
		{
			ul.unlock();
			continue;
		}

		QString fileName = m_downloadFileQueue.first();
		m_downloadFileQueue.removeFirst();

		ul.unlock();

		privateDownloadFile(fileName);
	}

	if (m_stub != nullptr)
	{
		m_stub.reset();
	}
}

void GrpcFileClient::createStubAndHandshake(grpc::Status* status)
{
	m_srvAddrIndex++;

	if (m_srvAddrIndex >= m_serverAddress.size())
	{
		m_srvAddrIndex = 0;
	}

	const std::string endpoint = m_serverAddress[m_srvAddrIndex].addressPortStr().toStdString();

	auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
	m_stub = Grpc::FileSrv::NewStub(channel);

	grpc::ClientContext handshakeCtx;

	Grpc::HandshakeRequest req;
	Grpc::HandshakeReply rep;

	m_swInfo.serializeTo(req.mutable_clientsoftwareinfo());

	grpc::Status st = m_stub->Handshake(&handshakeCtx, req, &rep);

	if (status != nullptr)
	{
		*status = st;
	}

	if (st.ok())
	{
		m_authToken = rep.authtoken();
		return;
	}

	m_authToken.clear();
	m_stub.reset();
}

Tcp::FileTransferResult GrpcFileClient::privateDownloadFile(const QString& fileName)
{
	Tcp::FileTransferResult result = Tcp::FileTransferResult::Ok;

	m_transferInProgress.store(true, std::memory_order::relaxed);

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, m_authToken);

	Grpc::GetFileRequest req;

	req.set_filename(fileName.toStdString());

	auto reader = m_stub->GetFile(&ctx, req);

	Grpc::GetFileReply reply;
	QByteArray fileData;

	while(reader->Read(&reply))
	{
		if (reply.errorcode() != TO_INT(Tcp::FileTransferResult::Ok))
		{
			result = static_cast<Tcp::FileTransferResult>(reply.errorcode());
			break;
		}

		if (fileData.size() == 0)
		{
			fileData.reserve(reply.filesize());
		}

		const std::string& fData = reply.filedata();

		fileData.append(fData.data(), static_cast<int>(fData.size()));

		if (fileData.size() < reply.filesize())
		{
			reply.Clear();
			continue;
		}

		if (fileData.size() > reply.filesize())
		{
			Q_ASSERT(false);
			result = Tcp::FileTransferResult::InternalError;
			break;
		}

		QCryptographicHash md5Gen(QCryptographicHash::Md5);

		md5Gen.addData(QByteArrayView(fileData.constData(), fileData.size()));

		QByteArray md5 = md5Gen.result();

		const std::string &protoStr = reply.md5();
		QByteArray protoMd5 = QByteArray::fromStdString(protoStr);

		if (md5 != protoMd5)
		{
			result = Tcp::FileTransferResult::FileDataCorrupted;
			break;
		}

		QString rootFolder;

		{
			std::lock_guard lg(m_mutex);

			rootFolder = m_rootFolder;
		}

		QString filePathName = rootFolder + fileName;

		QFile file;

		file.setFileName(filePathName);

		QFileInfo fi(filePathName);

		QDir dir(fi.path());

		if (dir.mkpath(fi.path()) == false)
		{
			result = Tcp::FileTransferResult::CantCreateLocalFolder;
			break;
		}

		bool res = file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate);

		if (res == false)
		{
			result = Tcp::FileTransferResult::CantCreateLocalFile;
			break;
		}

		file.write(fileData);
		file.close();

		QFileInfo fi2(filePathName);

		if (fi2.size() != fileData.size())
		{
			Q_ASSERT(false);
			result = Tcp::FileTransferResult::CantWriteLocalFile;
			break;
		}

		break;
	}

	reader->Finish();

	m_transferInProgress.store(false, std::memory_order::relaxed);

	return result;
}
