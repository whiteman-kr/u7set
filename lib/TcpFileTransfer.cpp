#include "../include/TcpFileTransfer.h"
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

	FileClient::FileClient(const QString& rootFolder, const HostAddressPort &serverAddressPort) :
		Client(serverAddressPort)
	{
		m_rootFolder = rootFolder;
		m_file.setParent(this);

		init();
	}


	FileClient::FileClient(const QString &rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
		Client(serverAddressPort1, serverAddressPort2)
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
		emit signal_endDownloadFile(m_fileName, errorCode);
		init();
	}


	void FileClient::slot_downloadFile(const QString fileName)
	{
		if (isConnected() == false)
		{
			emit signal_endDownloadFile(fileName, FileTransferResult::NotConnectedToServer);
			return;
		}

		if (m_transferInProgress == true)
		{
			assert(false);
			emit signal_endDownloadFile(fileName, FileTransferResult::AlreadyDownloadFile);
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
			assert(false);
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

		init();

		emit signal_endDownloadFile(downloadedFileName, FileTransferResult::Ok);
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class implementation
	//
	// -------------------------------------------------------------------------------------

	FileServer::FileServer(const QString& rootFolder)
	{
		m_rootFolder = rootFolder;
		m_file.setParent(this);
	}


	void FileServer::init()
	{
		m_fileName = "";
		m_file.close();
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
			break;

		case RQID_GET_FILE_NEXT:
			sendNextFilePart();
			break;

		default:
			assert(false);
		}
	}


	void FileServer::sendFirstFilePart(const QString& fileName)
	{
		if (m_transferInProgress == true)
		{
			// cancel current transfer
			//
			init();
		}

		m_transferInProgress = true;

		// start upload process
		//
		init();

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
		m_reply.totalParts = m_reply.fileSize / FILE_PART_SIZE + (m_reply.fileSize % FILE_PART_SIZE ? 1 : 0);

		sendNextFilePart();
	}


	void FileServer::sendNextFilePart()
	{
		if (m_file.isOpen() == false)
		{
			assert(false);
			m_reply.errorCode = FileTransferResult::CantReadRemoteFile;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		int readed = m_file.read(m_fileData, FILE_PART_SIZE);

		if (readed == 0 || readed == -1)
		{
			assert(false);
			m_reply.errorCode = FileTransferResult::CantReadRemoteFile;
			sendReply(m_reply, sizeof(m_reply));
			init();
			return;
		}

		m_reply.currentPart++;
		m_reply.currentPartSize = readed;

		m_md5Generator.addData(m_fileData, readed);
		m_reply.setMD5(m_md5Generator.result());

		sendReply(m_replyData, sizeof(GetFileReply) + m_reply.currentPartSize);

		if (m_reply.currentPart == m_reply.totalParts)
		{
			// all parts of file are sent
			//
			init();
		}
	}

}
