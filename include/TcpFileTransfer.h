#pragma once

#include "../include/Tcp.h"
#include <QFile>


namespace Tcp
{

	const quint32	RQID_GET_FILE_START = 400,
					RQID_GET_FILE_NEXT = 401,
					RQID_GET_FILE_CANCEL = 402;

	enum FileTransferError
	{
		Ok,
		FileNotFound,
		LocalFolderIsNotWriteable,
		FileDownloadError,
		AlreadyDownloadFile,
		CantCreateLocalFolder,
		NotConnectedToServer,
		CantSendRequest,
		AlreadyUploadFile,
		CantOpenFile
	};


	struct GetFileStartReply
	{
		FileTransferError errorCode = FileTransferError::Ok;
		qint64 fileSize = 0;
		int md5Size = 0;
		char md5 = 0;			// array of bytes md5[md5Size]

		operator const char* () { return reinterpret_cast<const char*>(this); }
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileClient class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileClient : public Client
	{
		Q_OBJECT

	public:

	private:
		QString m_rootFolder;
		bool m_downloadInProgress = false;
		QString m_fileName;
		qint64 m_fileSize = 0;
		QByteArray m_fileMD5 = 0;

		FileTransferError checkLocalFolder();		// check read/write ability for m_rootFolder
		FileTransferError createFile();

		void init();

		void startFileDownload();

		void endFileDownload(FileTransferError errorCode);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) final;

		void processGetFileStartReply(const char* replyData, quint32 replyDataSize);

		virtual void onClientThreadStarted() override;

	private slots:
		void slot_downloadFile(const QString fileName);

	signals:
		void signal_downloadFile(const QString fileName);
		void signal_fileDownloaded(const QString filename, FileTransferError errorCode);

	public:
		FileClient(const QString& rootFolder, const HostAddressPort& serverAddressPort);
		FileClient(const QString& rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

		void downloadFile(const QString& fileName) { emit signal_downloadFile(fileName); }
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileServer : public Server
	{
	private:
		QString m_rootFolder;
		QString m_fileName;
		QFile m_file;

		bool m_uploadInProgress = false;

		void init();

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);

		void processGetFileStartRequest(const char* requestData, quint32 requestDataSize);

		void onServerThreadStarted() override;

	public:
		FileServer(const QString& rootFolder);

		virtual Server* getNewInstance() override { return new FileServer(m_rootFolder); }
	};

}
