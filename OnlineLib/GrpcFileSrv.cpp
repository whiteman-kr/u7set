#include "GrpcFileSrv.h"
#include <CommonStdLib/TimesStd.h>

// -------------------------------------------------------------------------------------
//
// GrpcFileBase class implementation
//
// -------------------------------------------------------------------------------------

GrpcFileBase::GrpcFileBase(const QString& rootFolder)
{
	setRootFolder(rootFolder);

	Q_ASSERT(m_rootFolder.isEmpty() == false);
}

void GrpcFileBase::setRootFolder(const QString& rootFolder)
{
	std::lock_guard lg(m_mutex);
	m_rootFolder = QDir::cleanPath(rootFolder);
}

QString GrpcFileBase::rootFolder() const
{
	std::lock_guard lg(m_mutex);
	return m_rootFolder;
}

QString GrpcFileBase::getCleanRoot(const QString& rootFolder)
{
	return QDir::cleanPath(rootFolder);
}

QString GrpcFileBase::getCleanFileName(const QString& rootFolder, const QString& fileName)
{
	QString filePathName = rootFolder + fileName;

	return QDir::cleanPath(filePathName);
}

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
	GrpcFileBase(rootFolder),
	GrpcServer(serverSwInfo, allowAllClients, clients, checkHostName, listenIP, log)
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

	QString root = rootFolder();

	QString cleanFileName = getCleanFileName(root, fileName);

	if (!cleanFileName.startsWith(root))
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

// -------------------------------------------------------------------------------------
//
// GrpcClient class implementation
//
// -------------------------------------------------------------------------------------

GrpcClient::GrpcClient(const SoftwareInfo& localSoftwareInfo,
						const std::vector<HostAddressPort>& serverAddress,
						const QString& clientDescription,
						CircularLoggerShared log,
						bool startClient) :
	LogWrapper(log),
	m_localSwInfo(localSoftwareInfo),
	m_serverAddress(serverAddress),
	m_clientDescription(clientDescription)

{
	Q_ASSERT(m_serverAddress.size() > 0);

	m_state.localSoftwareInfo = localSoftwareInfo;

	if (startClient)
	{
		start();
	}
}

GrpcClient::~GrpcClient()
{
	stop();
}

const SoftwareInfo& GrpcClient::localSwInfo() const
{
	return m_localSwInfo;
}

QString GrpcClient::clientDescription() const
{
	return m_clientDescription;
}

std::string GrpcClient::authToken() const
{
	std::string token;

	{
		std::lock_guard lg(m_stateMutex);
		token = m_authToken;
	}

	return token;
}

void GrpcClient::setAuthToken(const std::string& token)
{
	std::lock_guard lg(m_stateMutex);
	m_authToken = token;
}

void GrpcClient::clearAuthToken()
{
	std::lock_guard lg(m_stateMutex);
	m_authToken.clear();
}

void GrpcClient::start()
{
	if (m_serverAddress.size() == 0)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_threadStarted.load(std::memory_order::acquire))
	{
		Q_ASSERT(false);          // уже запущен
		return;
	}

	m_quitRequested.store(false, std::memory_order::relaxed);
	m_thread = std::thread(&GrpcClient::run, this);
	m_threadStarted.store(true, std::memory_order::release);
}

void GrpcClient::stop()
{
	if (m_threadStarted.load(std::memory_order::acquire) == false)
	{
		return;
	}

	m_quitRequested.store(true, std::memory_order::relaxed);

	wakeupThread();

	if (m_thread.joinable())
	{
		m_thread.join();
	}

	m_threadStarted.store(false, std::memory_order::release);
}

bool GrpcClient::isThreadStarted() const
{
	return m_threadStarted.load(std::memory_order::acquire);
}

bool GrpcClient::isQuitRequested() const
{
	return m_quitRequested.load(std::memory_order::relaxed);
}

HostAddressPort GrpcClient::getConnectedServerAddr() const
{
	HostAddressPort serverAddr;

	{
		std::lock_guard lg(m_stateMutex);
		serverAddr = m_state.peerAddr;
	}

	return serverAddr;
}

Tcp::ConnectionState GrpcClient::getConnectionState() const
{
	Tcp::ConnectionState state;

	{
		std::lock_guard lg(m_stateMutex);
		state = m_state;
	}

	return state;
}

void GrpcClient::setConnectionState(const Tcp::ConnectionState& state)
{
	std::lock_guard lg(m_stateMutex);

	m_state = state;
}

std::string GrpcClient::getNextServerAddr() const
{
	m_srvAddrIndex++;

	if (m_srvAddrIndex >= m_serverAddress.size())
	{
		m_srvAddrIndex = 0;
	}

	return m_serverAddress[m_srvAddrIndex].addressPortStr().toStdString();
}

void GrpcClient::run()				// do nothing, should be override in derived classes
{
	while(isQuitRequested() == false)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void GrpcClient::wakeupThread()
{
}

// -------------------------------------------------------------------------------------
//
// ActiveCtxGuard class implementation
//
// -------------------------------------------------------------------------------------

/*GrpcFileClient::ActiveCtxGuard::ActiveCtxGuard(std::mutex& mutex, grpc::ClientContext*& activeCtx, grpc::ClientContext* ctx) :
	m_mutex(mutex),
	m_activeCtx(activeCtx)
{
}
	~ActiveCtxGuard();

	std::mutex& m_mutex;
	grpc::ClientContext*& m_activeCtx;
};*/


// -------------------------------------------------------------------------------------
//
// GrpcFileClient class implementation
//
// -------------------------------------------------------------------------------------

GrpcFileClient::GrpcFileClient(const SoftwareInfo& localSoftwareInfo,
	const std::vector<HostAddressPort>& serverAddress,
	const QString& rootFolder,
	const QString& clientDescription,
	CircularLoggerShared log,
	bool startClient) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, startClient),
	GrpcFileBase(rootFolder)
{
}

GrpcFileClient::~GrpcFileClient()
{
	stop();
}

void GrpcFileClient::downloadSessionParams()
{
	downloadFile(SESSION_PARAMS_REQUEST);
}

void GrpcFileClient::downloadFile(const QString& fileName)
{
	Q_ASSERT(isThreadStarted() == true);

	{
		std::lock_guard lg(m_procMutex);
		m_downloadFileQueue.append(fileName);
	}

	m_procCond.notify_one();
}

bool GrpcFileClient::waitFileReady(FileReady* fileReady)
{
	TEST_PTR_RETURN_FALSE(fileReady);

	std::unique_lock ul(m_fileReadyMutex);

	if (!m_fileReadyCond.wait_for(ul, std::chrono::seconds(5), [this]
							  {
								  return isQuitRequested() ||
										 !m_fileReadyQueue.empty();
							  }))
	{
		fileReady->errorCode = Tcp::FileTransferResult::ServerReplyTimeout;

		ul.unlock();

		emit signal_fileDowloadTimeout();
		return false;
	}

	if (!m_fileReadyQueue.empty())
	{
		FileReady& fr = m_fileReadyQueue.front();

		fileReady->errorCode = fr.errorCode;
		fr.fileName.swap(fileReady->fileName);
		fr.fileData.swap(fileReady->fileData);
		fr.md5.swap(fileReady->md5);

		m_fileReadyQueue.pop();

		ul.unlock();

		return true;
	}

	fileReady->errorCode = Tcp::FileTransferResult::NotConnectedToServer;

	ul.unlock();

	emit signal_noConnection();

	return false;
}

bool GrpcFileClient::downloadFileBlocked(const QString& fileName, FileReady* fileReady)
{
	TEST_PTR_RETURN_FALSE(fileReady);

	downloadFile(fileName);
	return waitFileReady(fileReady);
}

bool GrpcFileClient::isTransferInProgress()
{
	return m_transferInProgress.load(std::memory_order::relaxed);
}

void GrpcFileClient::setEmitFileReady(bool enable)
{
	m_emitFileReady.store(enable, std::memory_order::relaxed);
}

void GrpcFileClient::run()
{
	std::unique_lock ul(m_procMutex, std::defer_lock);

	while(true)
	{
		if (isQuitRequested() == true)
		{
			break;
		}

		if (authToken().empty())
		{
			createStubAndHandshake();

			if (authToken().empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			}
		}

		ul.lock();

		m_procCond.wait(ul, [this]() -> bool
						   {
								return isQuitRequested() ||	!m_downloadFileQueue.empty();
						   });

		if (isQuitRequested() == true)
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

		if (fileName == SESSION_PARAMS_REQUEST)
		{
			privateGetSessionParams();
		}
		else
		{
			privateDownloadFile(fileName);
		}
	}

	if (m_stub != nullptr)
	{
		m_stub.reset();
	}
}

void GrpcFileClient::wakeupThread()
{
	m_procCond.notify_all();
	m_fileReadyCond.notify_all();
}

void GrpcFileClient::createStubAndHandshake(grpc::Status* status)
{
	logMsg(QString("%1::createStubAndHandshake").arg(clientDescription()));

	const std::string endpoint = getNextServerAddr();

	auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
	m_stub = Grpc::FileSrv::NewStub(channel);

	grpc::ClientContext handshakeCtx;

	handshakeCtx.set_deadline(makeDeadlineMs(1000));

	Grpc::HandshakeRequest req;
	Grpc::HandshakeReply rep;

	localSwInfo().serializeTo(req.mutable_clientsoftwareinfo());

	grpc::Status st = m_stub->Handshake(&handshakeCtx, req, &rep);

	if (status != nullptr)
	{
		*status = st;
	}

	Tcp::ConnectionState state;

	if (st.ok())
	{
		setAuthToken(rep.authtoken());

		//

		state.isSocketConnected = true;
		state.isConnected = true;
		state.connectedSoftwareInfo.serializeFrom(rep.serversoftwareinfo());
		state.localSoftwareInfo = localSwInfo();
		state.securityLevel = E::SecurityLevel::Basic;	// !!!
		state.setConnectionResult = Tcp::SetConnectionResult::Ok;
		state.connectionNo = 1;
		state.serverEquipmentID = state.connectedSoftwareInfo.equipmentID();
		state.peerAddr = HostAddressPort(rep.serverip(), rep.serverport());
		state.startTime = currentMSecsUTC();
		state.sentBytes = 0;
		state.receivedBytes = 0;
		state.requestCount = 1;
		state.replyCount = 1;
		state.isActual = false;

		setConnectionState(state);

		logMsg(QString("%1::createStubAndHandshake - Handshake Ok").arg(clientDescription()));

		emit signal_setConnection();
		return;
	}

	if (st.error_code() == grpc::StatusCode::UNAUTHENTICATED)
	{
		if (st.error_message() == Grpc::WRONG_CLIENT_EQUIPMENT_ID)
		{
			state.setConnectionResult = Tcp::SetConnectionResult::UnknownClientID;
			emit signal_unknownClientID(QString::fromStdString(Grpc::WRONG_CLIENT_EQUIPMENT_ID));
		}
		else
		{
			if (st.error_message() == Grpc::WRONG_HOST_NAME)
			{
				state.setConnectionResult = Tcp::SetConnectionResult::WrongClientHostname;
				emit signal_wrongClientHostname(QString::fromStdString(Grpc::WRONG_HOST_NAME));
			}
			else
			{
				Q_ASSERT(false);
			}
		}
	}

	clearAuthToken();

	state.localSoftwareInfo = localSwInfo();

	setConnectionState(state);

	logErr(QString("%1::createStubAndHandshake - Handshake Failed").arg(clientDescription()));

	m_stub.reset();
}

Tcp::FileTransferResult GrpcFileClient::privateGetSessionParams()
{
	SessionParams params;

	Tcp::FileTransferResult result = Tcp::FileTransferResult::Ok;

	if (m_stub == nullptr)
	{
		m_transferInProgress.store(false, std::memory_order::relaxed);
		result = Tcp::FileTransferResult::NotConnectedToServer;
		emit signal_sessionParamsReady(result, params);
		return result;
	}

	m_transferInProgress.store(true, std::memory_order::relaxed);

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken());
	ctx.set_deadline(makeDeadlineMs(1000));

	Grpc::GetSessionParamsRequest req;
	Grpc::GetSessionParamsReply rep;

	::grpc::Status st = m_stub->GetSessionParams(&ctx, req, &rep);

	if (st.ok())
	{
		params.loadFrom(rep.sessionparams());
		emit signal_sessionParamsReady(result, params);
	}
	else
	{
		if (st.error_code() == grpc::StatusCode::UNAUTHENTICATED)
		{
			result = Tcp::FileTransferResult::NotConnectedToServer;
			emit signal_sessionParamsReady(result, params);
		}
		else
		{
			result = Tcp::FileTransferResult::InternalError;
			emit signal_sessionParamsReady(result, params);
		}
	}

	m_transferInProgress.store(false, std::memory_order::relaxed);

	return result;
}

Tcp::FileTransferResult GrpcFileClient::privateDownloadFile(const QString& fileName)
{
	Tcp::FileTransferResult result = Tcp::FileTransferResult::Ok;

	if (m_stub == nullptr)
	{
		m_transferInProgress.store(false, std::memory_order::relaxed);
		result = Tcp::FileTransferResult::NotConnectedToServer;
		pushFileReady(fileName, result);
		return result;
	}

	m_transferInProgress.store(true, std::memory_order::relaxed);

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken());
	ctx.set_deadline(makeDeadlineMs(1000));

	Grpc::GetFileRequest req;

	req.set_filename(fileName.toStdString());

	auto reader = m_stub->GetFile(&ctx, req);

	Grpc::GetFileReply reply;
	QByteArray fileData;

	bool anyReplyReceived = false;
	bool readyPushed = false;

	while(reader->Read(&reply))
	{
		anyReplyReceived = true;

		if (reply.errorcode() != TO_INT(Tcp::FileTransferResult::Ok))
		{
			result = static_cast<Tcp::FileTransferResult>(reply.errorcode());
			pushFileReady(fileName, result);
			readyPushed = true;
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
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		QString md5 = Md5Hash::hashStr(fileData);

		QString protoMd5 = QString::fromStdString(reply.md5());

		if (md5 != protoMd5)
		{
			result = Tcp::FileTransferResult::FileDataCorrupted;
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		QString root = rootFolder();

		QString filePathName = getCleanFileName(root, fileName);

		if (!filePathName.startsWith(root))
		{
			result = Tcp::FileTransferResult::CantCreateLocalFile;
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		QFile file;

		file.setFileName(filePathName);

		QFileInfo fi(filePathName);

		QDir dir(fi.path());

		if (dir.mkpath(fi.path()) == false)
		{
			result = Tcp::FileTransferResult::CantCreateLocalFolder;
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		bool res = file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate);

		if (res == false)
		{
			result = Tcp::FileTransferResult::CantCreateLocalFile;
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		file.write(fileData);
		file.close();

		QFileInfo fi2(filePathName);

		if (fi2.size() != fileData.size())
		{
			Q_ASSERT(false);
			result = Tcp::FileTransferResult::CantWriteLocalFile;
			pushFileReady(fileName, result);
			readyPushed = true;
			break;
		}

		pushFileReady(fileName, result, fileData, md5);
		readyPushed = true;
		break;
	}

	grpc::Status st = reader->Finish();

	if (!st.ok())
	{
		if (result == Tcp::FileTransferResult::Ok)
		{
			if (st.error_code() == grpc::StatusCode::UNAUTHENTICATED)
			{
				result = Tcp::FileTransferResult::NotConnectedToServer;
			}
			else
			{
				result = Tcp::FileTransferResult::InternalError;
			}
		}

		if (st.error_code() == grpc::StatusCode::UNAUTHENTICATED)
		{
			clearAuthToken();
			m_stub.reset();
		}

		if (!readyPushed)
		{
			pushFileReady(fileName, result);
			readyPushed = true;
		}
	}
	else
	{
		if (!anyReplyReceived && !readyPushed)
		{
			result = Tcp::FileTransferResult::InternalError;
			pushFileReady(fileName, result);
			readyPushed = true;
		}
	}

	m_transferInProgress.store(false, std::memory_order::relaxed);

	return result;
}

void GrpcFileClient::pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode)
{
	QByteArray fileData;
	QString md5;

	pushFileReady(fileName, errorCode, fileData, md5);
}

void GrpcFileClient::pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode,
									QByteArray& fileData, QString& md5)
{
	if (m_emitFileReady.load(std::memory_order::relaxed) == true)
	{
		FileReady fr;

		fr.fileName = fileName;
		fr.errorCode = errorCode;
		fr.fileData.swap(fileData);
		fr.md5.swap(md5);

		emit signal_fileReady(fr);
	}
	else
	{
		{
			std::lock_guard lg(m_fileReadyMutex);

			m_fileReadyQueue.push(FileReady{});

			FileReady& last = m_fileReadyQueue.back();

			last.fileName = fileName;
			last.errorCode = errorCode;
			last.fileData.swap(fileData);
			last.md5.swap(md5);
		}

		m_fileReadyCond.notify_all();
	}
}

std::chrono::system_clock::time_point GrpcFileClient::makeDeadlineMs(int ms)
{
	return std::chrono::system_clock::now()	+ std::chrono::milliseconds(ms);
}
