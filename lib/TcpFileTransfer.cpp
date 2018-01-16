#include "../lib/TcpFileTransfer.h"
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QMetaObject>

namespace Tcp
{

	// -------------------------------------------------------------------------------------
	//
	// Tcp::GetFileReply structure implementation
	//
	// -------------------------------------------------------------------------------------

	GetFileReply::GetFileReply()
	{
		clear();
	}


	void GetFileReply::clear()
	{
		errorCode = FileTransferResult::Ok;
		fileSize = 0;
		totalParts = 0;
		currentPart = 0;
		currentPartSize = 0;
		memset(md5, 0, MD5_LEN);
	}


	void GetFileReply::setMD5(const QByteArray& md5)
	{
		if (md5.size() < MD5_LEN)
		{
			assert(false);
			return;
		}

		memcpy(this->md5, md5.constData(), MD5_LEN);
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileTransfer class implementation
	//
	// -------------------------------------------------------------------------------------

	bool FileTransfer::m_FileTransferErrorIsRegistered = false;

	FileTransfer::FileTransfer() :
		m_md5Generator(QCryptographicHash::Md5)
	{
		if (m_FileTransferErrorIsRegistered == false)
		{
			qRegisterMetaType<FileTransferResult>("FileTransferResult");
			m_FileTransferErrorIsRegistered = true;
		}
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileClient class implementation
	//
	// -------------------------------------------------------------------------------------

	FileClient::FileClient(const QString& rootFolder,
						   const HostAddressPort &serverAddressPort,
						   E::SoftwareType softwareType,
						   const QString equipmentID,
						   int majorVersion,
						   int minorVersion,
						   int commitNo) :
		Client(serverAddressPort,
			   softwareType,
			   equipmentID,
			   majorVersion,
			   minorVersion,
			   commitNo)
	{
		m_rootFolder = rootFolder;
		m_file.setParent(this);

		init();
	}


	FileClient::FileClient(const QString &rootFolder,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2,
						   E::SoftwareType softwareType,
						   const QString equipmentID,
						   int majorVersion,
						   int minorVersion,
						   int commitNo) :
		Client(serverAddressPort1,
			   serverAddressPort2,
			   softwareType,
			   equipmentID,
			   majorVersion,
			   minorVersion,
			   commitNo)
	{
		m_rootFolder = rootFolder;
		m_file.setParent(this);

		init();
	}


	void FileClient::onClientThreadStarted()
	{
		connect(this, &FileClient::signal_downloadFile, this, &FileClient::slot_downloadFile);
	}


	void FileClient::onClientThreadFinished()
	{
		m_file.close();
	}

	void FileClient::onReplyTimeout()
	{
		endFileDownload(FileTransferResult::ServerReplyTimeout);
	}

	void FileClient::init()
	{
		m_fileName = "";
		m_file.close();
		m_transferInProgress = false;
		m_reply.clear();
		m_md5Generator.reset();
	}

	FileTransferResult FileClient::checkLocalFolder()
	{
		QDir dir(m_rootFolder);

		if (dir.exists() == false)
		{
			dir.mkpath(m_rootFolder);
		}

		QFileInfo pathInfo(m_rootFolder);

		if (pathInfo.permission(QFileDevice::WriteUser | QFileDevice::ReadUser) == false)
		{
			return FileTransferResult::LocalFolderIsNotWriteable;
		}

		return FileTransferResult::Ok;
	}


	FileTransferResult FileClient::createFile()
	{
		QFileInfo fileInfo(m_rootFolder + m_fileName);

		QString path = fileInfo.path();

		QDir dir;

		if (dir.mkpath(path) == false)
		{
			return FileTransferResult::CantCreateLocalFolder;
		}

		return FileTransferResult::Ok;
	}


	void FileClient::endFileDownload(FileTransferResult errorCode)
	{
		emit signal_endFileDownload(m_fileName, errorCode, QString(""));

		onEndFileDownload(m_fileName, errorCode, QString(""));

		init();
	}


	void FileClient::slot_downloadFile(const QString fileName)
	{
		if (isConnected() == false)
		{
			emit signal_endFileDownload(fileName, FileTransferResult::NotConnectedToServer, QString(""));
			onEndFileDownload(fileName, FileTransferResult::NotConnectedToServer, QString(""));
			return;
		}

		if (m_transferInProgress == true)
		{
			assert(false);
			emit signal_endFileDownload(fileName, FileTransferResult::AlreadyDownloadFile, QString(""));
			onEndFileDownload(fileName, FileTransferResult::AlreadyDownloadFile, QString(""));
			return;
		}

		m_transferInProgress = true;
		m_fileName = fileName;

		FileTransferResult err = checkLocalFolder();

		if (err != FileTransferResult::Ok)
		{
			endFileDownload(err);
			return;
		}

		if (isClearToSendRequest() == false)
		{
			endFileDownload(FileTransferResult::CantSendRequest);
			return;
		}

		sendRequest(RQID_GET_FILE_START, m_fileName.toUtf8());
	}


	void FileClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case RQID_GET_FILE_START:
			processGetFileStartNextReply(true, replyData, replyDataSize);
			break;

		case RQID_GET_FILE_NEXT:
			processGetFileStartNextReply(false, replyData, replyDataSize);
			break;

		default:
			processSuccessorReply(requestID, replyData, replyDataSize);
		}
	}


	void FileClient::processGetFileStartNextReply(bool startReply, const char* replyData, quint32 replyDataSize)
	{
		assert(replyDataSize >= sizeof(GetFileReply));

		const GetFileReply* reply = reinterpret_cast<const GetFileReply*>(replyData);

		if (reply->errorCode != FileTransferResult::Ok)
		{
			endFileDownload(reply->errorCode);
			return;
		}

		assert(replyDataSize == (sizeof(GetFileReply) + reply->currentPartSize));

		if (startReply)
		{
			QString fileName = m_rootFolder + m_fileName;

			QFileInfo fi(fileName);

			QDir dir(fi.path());

			if (dir.mkpath(fi.path()) == false)
			{
				endFileDownload(FileTransferResult::CantCreateLocalFolder);
				return;
			}

			m_file.setFileName(fileName);

			if (m_file.open(QIODevice::ReadWrite | QIODevice::Truncate) == false)
			{
				endFileDownload(FileTransferResult::CantCreateLocalFile);
				return;
			}

			if (m_file.resize(reply->fileSize) == false)
			{
				endFileDownload(FileTransferResult::CantCreateLocalFile);
				return;
			}

			m_file.seek(0);
		}

		const char* filePartData = replyData + sizeof(GetFileReply);

		m_md5Generator.addData(filePartData, reply->currentPartSize);

		if (memcmp(m_md5Generator.result().constData(), reply->md5, MD5_LEN) != 0)
		{
			endFileDownload(FileTransferResult::FileDataCorrupted);
			return;
		}

		qint64 written = m_file.write(filePartData, reply->currentPartSize);

		if (written < reply->currentPartSize)
		{
			endFileDownload(FileTransferResult::CantWriteLocalFile);
			return;
		}

		if (reply->currentPart != reply->totalParts)
		{
			sendRequest(RQID_GET_FILE_NEXT);
			return;
		}

		QString downloadedFileName = m_fileName;

		QString md5 = QString(m_md5Generator.result().toHex());

		init();

		emit signal_endFileDownload(downloadedFileName, FileTransferResult::Ok, md5);

		onEndFileDownload(downloadedFileName, FileTransferResult::Ok, md5);
	}


	void FileClient::processSuccessorReply(quint32 /*requestID*/, const char* /*replyData*/, quint32 /*replyDataSize*/)
	{
	}


	void FileClient::onEndFileDownload(const QString /*fileName*/, FileTransferResult /*errorCode*/, const QString /*md5*/)
	{
	}


	QString FileClient::getErrorStr(FileTransferResult errorCode)
	{
		QString str;

		switch(errorCode)
		{
		case Ok:
			// return ""
			break;

		case NotConnectedToServer:
			str = QString(tr("Not connected to server"));
			break;

		case NotFoundRemoteFile:
			str = QString(tr("Not found remote file"));
			break;

		case CantOpenRemoteFile:
			str = QString(tr("Can't open remote file"));
			break;

		case CantReadRemoteFile:
			str = QString(tr("Can't read remote file"));
			break;

		case LocalFolderIsNotWriteable:
			str = QString(tr("Local folder is not writeable"));
			break;

		case CantCreateLocalFolder:
			str = QString(tr("Can't create local folder"));
			break;

		case AlreadyDownloadFile:
			str = QString(tr("Already download file now"));
			break;

		case CantSendRequest:
			str = QString(tr("Can't send request"));
			break;

		case CantCreateLocalFile:
			str = QString(tr("Can't create local file"));
			break;

		case FileDataCorrupted:
			str = QString(tr("File data corrupted"));
			break;

		case CantWriteLocalFile:
			str = QString(tr("Can't write local file"));
			break;

		case InternalError:
			str = QString(tr("Internal error"));
			break;

		case LocalFileReadingError:
			str = QString(tr("Local file reading error"));
			break;

		case ServerReplyTimeout:
			str = QString(tr("Server reply timeout"));
			break;

		case ConfigurationIsNotReady:
			str = QString(tr("Configuration is not ready"));
			break;


		default:
			assert(false);
		}

		return str;
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class implementation
	//
	// -------------------------------------------------------------------------------------

	FileServer::FileServer(const QString& rootFolder, std::shared_ptr<CircularLogger> logger) :
		m_logger(logger),
		m_reply(*reinterpret_cast<GetFileReply*>(m_replyData)),
		m_replyFileData(m_replyData + sizeof(GetFileReply)),
		m_transmitionFilesTimer(this)
	{
		m_rootFolder = QDir::fromNativeSeparators(rootFolder);
		m_file.setParent(this);

		m_reply.clear();

		connect(&m_transmitionFilesTimer, &QTimer::timeout, [this]{ m_state->isActual = true; });
	}


	Server* FileServer::getNewInstance()
	{
		return new FileServer(m_rootFolder, m_logger);
	}


	void FileServer::processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		Q_UNUSED(requestID);
		Q_UNUSED(requestData);
		Q_UNUSED(requestDataSize);
	}


	QString FileServer::rootFolder() const
	{
		return m_rootFolder;
	}


	void FileServer::onFileSent(const QString& fileName, const QString &ip)
	{
		DEBUG_LOG_MSG(m_logger,  QString(tr("File '%1' has been sent to %2")).arg(fileName).arg(ip));
	}


	void FileServer::init()
	{
		m_fileName = "";

		if (m_file.isOpen() == true)
		{
			m_file.close();
		}

		m_fileData.clear();

		m_transferInProgress = false;
		m_reply.clear();
		m_md5Generator.reset();
	}


	void FileServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		switch(requestID)
		{
		case RQID_GET_FILE_START:
			sendFirstFilePart(QString::fromUtf8(requestData, requestDataSize));

			restartTransmitionFilesTimer();
			break;

		case RQID_GET_FILE_NEXT:
			sendNextFilePart();

			restartTransmitionFilesTimer();
			break;

		default:
			processSuccessorRequest(requestID, requestData, requestDataSize);
		}
	}

	bool FileServer::checkFile(QString& pathFileName, QByteArray& fileData)
	{
		Q_UNUSED(pathFileName);
		Q_UNUSED(fileData);

		return true;
	}

	void FileServer::sendFirstFilePart(const QString& fileName)
	{
		init();

		// start upload process
		//
		m_transferInProgress = true;

		m_fileName = fileName;

		m_file.setFileName(m_rootFolder + m_fileName);

		if (m_file.exists() == false)
		{
			m_reply.errorCode = FileTransferResult::NotFoundRemoteFile;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		if (m_file.open(QIODevice::ReadOnly) == false)
		{
			m_reply.errorCode = FileTransferResult::CantOpenRemoteFile;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		QFileInfo fi(m_file);

		m_reply.fileSize = fi.size();

		if (m_reply.fileSize == 0)
		{
			m_reply.totalParts = 1;
		}
		else
		{
			m_reply.totalParts = m_reply.fileSize / FILE_PART_SIZE + (m_reply.fileSize % FILE_PART_SIZE ? 1 : 0);
		}

		// read file into memory and check file consistensy

		m_fileData = m_file.readAll();

		m_file.close();

		if (m_reply.fileSize != 0 && m_fileData.size() == 0)
		{
			// file reading error!
			m_reply.errorCode = FileTransferResult::CantReadRemoteFile;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		if (checkFile(m_fileName, m_fileData) == false)
		{
			// file reading error!
			m_reply.errorCode = FileTransferResult::FileDataCorrupted;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		sendNextFilePart();
	}


	void FileServer::sendNextFilePart()
	{
		int offset = m_reply.currentPart * FILE_PART_SIZE;

		int size = m_fileData.size() - offset;

		if (size > FILE_PART_SIZE)
		{
			size = FILE_PART_SIZE;
		}

		const char* partDataPtr = m_fileData.constData() + offset;

		memcpy(m_replyFileData, partDataPtr, size);
		m_md5Generator.addData(partDataPtr, size);

		m_reply.setMD5(m_md5Generator.result());

		m_reply.currentPart++;
		m_reply.currentPartSize = size;

		sendReply(m_replyData, sizeof(GetFileReply) + size);

		if (m_reply.currentPart == m_reply.totalParts)
		{
			// all parts of file are sent
			//
			onFileSent(m_file.fileName(), peerAddr().addressStr());

			init();
		}
	}

	void FileServer::restartTransmitionFilesTimer()
	{
		m_state->isActual = false;

		m_transmitionFilesTimer.stop();

		m_transmitionFilesTimer.start(3 * 1000);
	}

}
