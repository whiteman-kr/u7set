#include "../include/TcpFileTransfer.h"
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>

namespace Tcp
{

	GetFileReply::GetFileReply()
	{
		clear();
	}


	void GetFileReply::clear()
	{
		errorCode = FileTransferError::Ok;
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
	// Tcp::FileClient class implementation
	//
	// -------------------------------------------------------------------------------------

	FileClient::FileClient(const QString& rootFolder, const HostAddressPort &serverAddressPort) :
		Client(serverAddressPort),
		m_rootFolder(rootFolder)
	{
		init();
	}


	FileClient::FileClient(const QString &rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
		Client(serverAddressPort1, serverAddressPort2),
		m_rootFolder(rootFolder)
	{
		init();
	}


	void FileClient::onClientThreadStarted()
	{
		connect(this, &FileClient::signal_downloadFile, this, &FileClient::slot_downloadFile);
	}

	void FileClient::init()
	{
		m_downloadInProgress = false;
		m_fileName = "";
		m_fileSize = 0;
		m_fileMD5.clear();
	}


	FileTransferError FileClient::checkLocalFolder()
	{
		QFileInfo pathInfo(m_rootFolder);

		if (pathInfo.permission(QFileDevice::WriteUser | QFileDevice::ReadUser) == false)
		{
			return FileTransferError::LocalFolderIsNotWriteable;
		}

		return FileTransferError::Ok;
	}


	FileTransferError FileClient::createFile()
	{
		QFileInfo fileInfo(m_rootFolder + m_fileName);

		QString path = fileInfo.path();

		QDir dir;

		if (dir.mkpath(path) == false)
		{
			return FileTransferError::CantCreateLocalFolder;
		}

		return FileTransferError::Ok;
	}



	void FileClient::endFileDownload(FileTransferError errorCode)
	{
		emit signal_fileDownloaded(m_fileName, errorCode);
		init();
	}


	void FileClient::slot_downloadFile(const QString fileName)
	{
		if (m_downloadInProgress == true)
		{
			assert(false);
			emit signal_fileDownloaded(fileName, FileTransferError::AlreadyDownloadFile);
			return;
		}

		m_downloadInProgress = true;
		m_fileName = fileName;

		FileTransferError err = checkLocalFolder();

		if (err != FileTransferError::Ok)
		{
			endFileDownload(err);
			return;
		}

		if (isConnected() == false)
		{
			endFileDownload(FileTransferError::NotConnectedToServer);
			return;
		}

		startFileDownload();
	}


	void FileClient::startFileDownload()
	{
		if (isClearToSendRequest() == false)
		{
			endFileDownload(FileTransferError::CantSendRequest);
			return;
		}

		sendRequest(RQID_GET_FILE_START, m_fileName.toUtf8());
	}


	void FileClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case RQID_GET_FILE_START:
			processGetFileStartReply(replyData, replyDataSize);
			break;

		case RQID_GET_FILE_NEXT:
			break;

		case RQID_GET_FILE_CANCEL:
			break;

		default:
			assert(false);
		}
	}


	void FileClient::processGetFileStartReply(const char* replyData, quint32 replyDataSize)
	{
		assert(replyDataSize >= sizeof(GetFileReply));

		const GetFileReply* reply = reinterpret_cast<const GetFileReply*>(replyData);

		if (reply->errorCode != FileTransferError::Ok)
		{
			endFileDownload(reply->errorCode);
			return;
		}

//		assert(replyDataSize == (sizeof(GetFileReply) - sizeof(char) + reply->md5Size));

		m_fileSize = reply->fileSize;

	//	m_fileMD5.fromRawData(&reply->md5, reply->md5Size);

		// try create file & set file size
	}


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class implementation
	//
	// -------------------------------------------------------------------------------------

	FileServer::FileServer(const QString& rootFolder) :
		m_rootFolder(rootFolder),
		m_md5Generator(QCryptographicHash::Md5)
	{
	}


	void FileServer::init()
	{
		m_fileName = "";
		m_file.close();
		m_uploadInProgress = false;
		m_reply.clear();
		m_md5Generator.reset();
	}


	void FileServer::onServerThreadStarted()
	{
	}


	void FileServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		switch(requestID)
		{
		case RQID_GET_FILE_START:
			processGetFileStartRequest(requestData, requestDataSize);
			break;
		}
	}


	void FileServer::processGetFileStartRequest(const char* requestData, quint32 requestDataSize)
	{
		/*if (m_uploadInProgress == true)
		{
			reply.errorCode = FileTransferError::AlreadyUploadFile;
			sendReply(reply, sizeof(reply));
			return;
		}

		m_uploadInProgress = true;

		// start upload process
		//
		init();

		m_fileName = QString::fromUtf8(requestData, requestDataSize);

		m_file.setFileName(m_fileName);

		if (m_file.exists() == false)
		{
			reply.errorCode = FileTransferError::FileNotFound;
			sendReply(reply, sizeof(reply));
			init();
			return;
		}

		if (m_file.open(QIODevice::ReadOnly) == false)
		{
			reply.errorCode = FileTransferError::CantOpenFile;
			sendReply(reply, sizeof(reply));
			init();
			return;
		}

		QFileInfo fi(m_file);

		m_reply.fileSize = fi.size();
		m_reply.totalParts = m_reply.fileSize / FILE_PART_SIZE + (m_reply.fileSize % FILE_PART_SIZE ? 1 : 0);*/
	}

	void FileServer::readFileData()
	{

	}

}
