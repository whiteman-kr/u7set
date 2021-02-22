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

/*	GetFileReply::GetFileReply()
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
*/

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

	FileClient::FileClient(const SoftwareInfo& softwareInfo, const QString& rootFolder,
						   const HostAddressPort &serverAddressPort) :
		Client(softwareInfo, serverAddressPort, "FileClient")
	{
		m_rootFolder = rootFolder;

		m_file.setParent(this);

		init();
	}

	FileClient::FileClient(const SoftwareInfo& softwareInfo,
						   const QString &rootFolder,
						   const HostAddressPort& serverAddressPort1,
						   const HostAddressPort& serverAddressPort2) :
		Client(softwareInfo, serverAddressPort1, serverAddressPort2,  "FileClient")
	{
		m_rootFolder = rootFolder;

		m_file.setParent(this);

		init();
	}

	void FileClient::processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		Q_UNUSED(requestID);
		Q_UNUSED(replyData);
		Q_UNUSED(replyDataSize);
	}

	void FileClient::onEndFileDownload(const QString fileName,
									   FileTransferResult errorCode,
									   const QString md5)
	{
		Q_UNUSED(fileName);
		Q_UNUSED(errorCode);
		Q_UNUSED(md5);
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
		faultyFileDownload(FileTransferResult::ServerReplyTimeout);
	}

	QString FileClient::getErrorStr(FileTransferResult errorCode) const
	{
		QString str;

		switch(errorCode)
		{
		case FileTransferResult::Ok:
			// return ""
			break;

		case FileTransferResult::NotConnectedToServer:
			str = QString(tr("Not connected to server"));
			break;

		case FileTransferResult::FileIsNotAccessible:
			str = QString(tr("File isn't enumerated in <files> section of Configuration.xml"));
			break;

		case FileTransferResult::RemoteFileIsNotExists:
			str = QString(tr("Remote file is not exists"));
			break;

		case FileTransferResult::CantOpenRemoteFile:
			str = QString(tr("Can't open remote file"));
			break;

		case FileTransferResult::CantReadRemoteFile:
			str = QString(tr("Can't read remote file"));
			break;

		case FileTransferResult::LocalFolderIsNotWriteable:
			str = QString(tr("Local folder is not writeable"));
			break;

		case FileTransferResult::CantCreateLocalFolder:
			str = QString(tr("Can't create local folder"));
			break;

		case FileTransferResult::AlreadyDownloadFile:
			str = QString(tr("Already download file now"));
			break;

		case FileTransferResult::CantSendRequest:
			str = QString(tr("Can't send request"));
			break;

		case FileTransferResult::CantCreateLocalFile:
			str = QString(tr("Can't create local file"));
			break;

		case FileTransferResult::FileDataCorrupted:
			str = QString(tr("File data corrupted"));
			break;

		case FileTransferResult::CantWriteLocalFile:
			str = QString(tr("Can't write local file"));
			break;

		case FileTransferResult::LocalFileReadingError:
			str = QString(tr("Local file reading error"));
			break;

		case FileTransferResult::ServerReplyTimeout:
			str = QString(tr("Server reply timeout"));
			break;

		case FileTransferResult::TransferIsNotStarted:
			str = QString(tr("File transferring is not started"));
			break;

		case FileTransferResult::ConfigurationIsNotReady:
			str = QString(tr("Configuration is not ready"));
			break;

		case FileTransferResult::UnknownClient:
			str = QString(tr("Unknown client's EquipmentID"));
			break;

		case FileTransferResult::InternalError:
			str = QString(tr("Internal error"));
			break;

		default:
			assert(false);
		}

		return str;
	}

	void FileClient::slot_downloadFile(const QString fileName)
	{
		if (isConnected() == false)
		{
			emit signal_endFileDownload(fileName, FileTransferResult::NotConnectedToServer, QString());
			onEndFileDownload(fileName, FileTransferResult::NotConnectedToServer, QString());
			return;
		}

		if (m_transferInProgress == true)
		{
			assert(false);
			emit signal_endFileDownload(fileName, FileTransferResult::AlreadyDownloadFile, QString());
			onEndFileDownload(fileName, FileTransferResult::AlreadyDownloadFile, QString());
			return;
		}

		m_transferInProgress = true;
		m_fileName = fileName;

		FileTransferResult err = checkLocalFolder();

		if (err != FileTransferResult::Ok)
		{
			faultyFileDownload(err);
			return;
		}

		if (isClearToSendRequest() == false)
		{
			faultyFileDownload(FileTransferResult::CantSendRequest);
			return;
		}

		sendRequest(RQID_GET_FILE_START, m_fileName.toUtf8());
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

	void FileClient::init()
	{
		m_fileName = "";
		m_file.close();
		m_transferInProgress = false;
		m_reply.Clear();
		m_md5Generator.reset();
	}

	void FileClient::faultyFileDownload(FileTransferResult errorCode)
	{
		emit signal_endFileDownload(m_fileName, errorCode, QString());

		onEndFileDownload(m_fileName, errorCode, QString());

		init();
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
		bool res = m_reply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

		if (res == false)
		{
			Q_ASSERT(false);
			return;
		}

		FileTransferResult replyErrorCode = static_cast<FileTransferResult>(m_reply.errorcode());

		if (replyErrorCode != FileTransferResult::Ok)
		{
			faultyFileDownload(replyErrorCode);
			return;
		}

		if (startReply == true)
		{
			Q_ASSERT(m_reply.currentpart() == 1);
			Q_ASSERT(m_reply.totalparts() >= 1);

			QString fileName = m_rootFolder + m_fileName;

			QFileInfo fi(fileName);

			QDir dir(fi.path());

			if (dir.mkpath(fi.path()) == false)
			{
				faultyFileDownload(FileTransferResult::CantCreateLocalFolder);
				return;
			}

			m_file.setFileName(fileName);

			if (m_file.open(QIODevice::ReadWrite | QIODevice::Truncate) == false)
			{
				faultyFileDownload(FileTransferResult::CantCreateLocalFile);
				return;
			}

			if (m_file.resize(m_reply.filesize()) == false)
			{
				faultyFileDownload(FileTransferResult::CantCreateLocalFile);
				return;
			}

			m_file.seek(0);
		}

		if (m_reply.currentpart() > m_reply.totalparts() ||
			m_reply.filepartdata().size() != m_reply.currentpartsize() ||
			m_reply.md5().size() != MD5_LEN)
		{
			faultyFileDownload(FileTransferResult::FileDataCorrupted);
			return;
		}

		m_md5Generator.addData(m_reply.filepartdata().data(), static_cast<int>(m_reply.filepartdata().size()));

		if (memcmp(m_md5Generator.result().constData(), m_reply.md5().data(), MD5_LEN) != 0)
		{
			faultyFileDownload(FileTransferResult::FileDataCorrupted);
			return;
		}

		qint64 written = m_file.write(m_reply.filepartdata().data(), m_reply.filepartdata().size());

		if (written < static_cast<qint64>(m_reply.filepartdata().size()))
		{
			faultyFileDownload(FileTransferResult::CantWriteLocalFile);
			return;
		}

		if (m_reply.currentpart() < m_reply.totalparts())
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

	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class implementation
	//
	// -------------------------------------------------------------------------------------

	FileServer::FileServer(const QString& rootFolder, const SoftwareInfo& softwareInfo, std::shared_ptr<CircularLogger> logger) :
		Server(softwareInfo),
		m_logger(logger),
		m_transmitionFilesTimer(this)
	{
		m_rootFolder = QDir::fromNativeSeparators(rootFolder);
		m_file.setParent(this);

		connect(&m_transmitionFilesTimer, &QTimer::timeout, [this]{ m_state.isActual = true; });
	}

	Server* FileServer::getNewInstance()
	{
		return new FileServer(m_rootFolder, localSoftwareInfo(), m_logger);
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
		m_reply.Clear();
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

	bool FileServer::checkClientID()
	{
		return true;			// real checking will be implemented in derived classes (if required)
	}

	void FileServer::sendFirstFilePart(const QString& fileName)
	{
		bool res = checkClientID();

		if (res == false)
		{
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::UnknownClient));
			sendReply(m_reply);
			init();
			return;
		}

		init();

		// start upload process
		//
		m_transferInProgress = true;

		m_fileName = fileName;

		m_file.setFileName(m_rootFolder + m_fileName);

		if (m_file.exists() == false)
		{
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::RemoteFileIsNotExists));
			sendReply(m_reply);
			init();
			return;
		}

		if (m_file.open(QIODevice::ReadOnly) == false)
		{
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::CantOpenRemoteFile));
			sendReply(m_reply);
			init();
			return;
		}

		QFileInfo fi(m_file);

		m_reply.set_filesize(fi.size());

		if (m_reply.filesize() == 0)
		{
			m_reply.set_totalparts(1);
		}
		else
		{
			m_reply.set_totalparts(static_cast<qint32>(m_reply.filesize() / FILE_PART_SIZE + ((m_reply.filesize() % FILE_PART_SIZE) ? 1 : 0)));
		}

		// read file into memory and check file consistensy

		m_fileData = m_file.readAll();

		m_file.close();

		if (m_reply.filesize() != 0 && m_fileData.size() == 0)
		{
			// file reading error!
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::CantReadRemoteFile));
			sendReply(m_reply);
			init();
			return;
		}

		if (checkFile(m_fileName, m_fileData) == false)
		{
			// file reading error!
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::FileDataCorrupted));
			sendReply(m_reply);
			init();
			return;
		}

		sendNextFilePart();
	}

	void FileServer::sendNextFilePart()
	{
		if (m_transferInProgress == false)
		{
			m_reply.set_errorcode(static_cast<int>(FileTransferResult::TransferIsNotStarted));
			sendReply(m_reply);
			init();
			return;
		}

		int offset = m_reply.currentpart() * FILE_PART_SIZE;

		int size = m_fileData.size() - offset;

		if (size > FILE_PART_SIZE)
		{
			size = FILE_PART_SIZE;
		}

		const char* partDataPtr = m_fileData.constData() + offset;

		m_reply.set_filepartdata(partDataPtr, size);

		m_md5Generator.addData(partDataPtr, size);

		m_reply.set_md5(m_md5Generator.result().toStdString());

		m_reply.set_currentpart(m_reply.currentpart() + 1);
		m_reply.set_currentpartsize(size);

		m_reply.set_errorcode(static_cast<int>(FileTransferResult::Ok));

		sendReply(m_reply);

		if (m_reply.currentpart() == m_reply.totalparts())
		{
			// all parts of file are sent
			//
			onFileSent(m_file.fileName(), peerAddr().addressStr());

			init();
		}
	}

	void FileServer::restartTransmitionFilesTimer()
	{
		m_state.isActual = false;

		m_transmitionFilesTimer.stop();

		m_transmitionFilesTimer.start(3 * 1000);
	}

}
