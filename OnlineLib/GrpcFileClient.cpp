#include "GrpcFileClient.h"

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
	int pingPeriodMs,
	bool startClient) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, pingPeriodMs, startClient),
	m_rootFolder(rootFolder)
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
		std::lock_guard lg(m_processingMutex);
		m_downloadFileQueue.append(fileName);
	}

	m_processigCondition.notify_one();
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
	std::unique_lock ul(m_processingMutex, std::defer_lock);

	while(isQuitRequested() == false)
	{
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

		m_processigCondition.wait_for(ul, std::chrono::milliseconds(pingPeriod()), [this]() -> bool
								  {
									  return isQuitRequested() ||	!m_downloadFileQueue.empty();
								  });

		if (isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		if (!m_downloadFileQueue.empty())
		{
			QString fileName = m_downloadFileQueue.first();
			m_downloadFileQueue.removeFirst();

			ul.unlock();

			Tcp::FileTransferResult result = Tcp::FileTransferResult::Ok;

			if (fileName == SESSION_PARAMS_REQUEST)
			{
				result = privateGetSessionParams();
			}
			else
			{
				result = privateDownloadFile(fileName);
			}

			if (result != Tcp::FileTransferResult::Ok)
			{
				resetStub();
			}

			continue;
		}

		ul.unlock();

		if (sendPingRequest() == false)
		{
			resetStub();
		}
	}

	resetStub();
}

void GrpcFileClient::wakeupThread()
{
	m_processigCondition.notify_all();
	m_fileReadyCond.notify_all();
}

Tcp::FileTransferResult GrpcFileClient::privateGetSessionParams()
{
	SessionParams params;

	Tcp::FileTransferResult result = Tcp::FileTransferResult::Ok;

	if (stub() == nullptr)
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

	::grpc::Status st = stub()->GetSessionParams(&ctx, req, &rep);

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

	if (stub() == nullptr)
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

	auto reader = stub()->GetFile(&ctx, req);

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

		QString filePathName = getCleanFileName(m_rootFolder, fileName);

		if (!filePathName.startsWith(m_rootFolder))
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
			resetStub();
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

QString GrpcFileClient::getCleanFileName(const QString& rootFolder, const QString& fileName)
{
	QString filePathName = rootFolder + fileName;

	return QDir::cleanPath(filePathName);
}
