#pragma once

#include "../lib/Tcp.h"
#include <QFile>
#include <QDir>
#include "../lib/Md5Hash.h"


namespace Tcp
{

	const quint32	RQID_GET_FILE_START = 400,
					RQID_GET_FILE_NEXT = 401;


	enum FileTransferResult
	{
		Ok,

		NotConnectedToServer,

		NotFoundRemoteFile,
		CantOpenRemoteFile,
		CantReadRemoteFile,

		LocalFolderIsNotWriteable,
		CantCreateLocalFolder,
		AlreadyDownloadFile,
		CantSendRequest,
		CantCreateLocalFile,
		FileDataCorrupted,
		CantWriteLocalFile,
		LocalFileReadingError,

		ConfigurationIsNotReady,		// for CfgLoader class

		InternalError,
	};


	const int MD5_LEN = 128 / 8;		// MD5 code lenght in bytes

	const int FILE_PART_SIZE = 65536;


#pragma pack(push, 1)

	struct GetFileReply
	{
		FileTransferResult errorCode = FileTransferResult::Ok;
		qint64 fileSize = 0;
		qint32 totalParts = 0;				// == fileSize / FILE_PART_SIZE + (fileSize % FILE_PART_SIZE ? 1 : 0)
		qint32 currentPart = 0;			// from 1
		qint32 currentPartSize = 0;		// <= FILE_PART_SIZE
		char md5[MD5_LEN];				// current MD5 code

		GetFileReply();

		void setMD5(const QByteArray& md5);
		operator const char* () { return reinterpret_cast<const char*>(this); }
		void clear();
	};

#pragma pack(pop)


	class FileTransfer
	{
	private:
		static bool m_FileTransferErrorIsRegistered;

	protected:
		QString m_rootFolder;
		QString m_fileName;
		QFile m_file;
		QCryptographicHash m_md5Generator;

		bool m_transferInProgress = false;

	public:
		FileTransfer();
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileClient class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileClient : public Client, public FileTransfer
	{
		Q_OBJECT

	private:
		GetFileReply m_reply;

		FileTransferResult checkLocalFolder();		// check read/write ability for m_rootFolder
		FileTransferResult createFile();

		void init();
		void endFileDownload(FileTransferResult errorCode);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) final;

		void processGetFileStartNextReply(bool startReply, const char* replyData, quint32 replyDataSize);

	protected:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		QString getErrorStr(FileTransferResult errorCode);

	protected slots:
		void slot_downloadFile(const QString fileName);

	signals:
		void signal_downloadFile(const QString fileName);
		void signal_endFileDownload(const QString fileName, FileTransferResult errorCode, const QString md5);

	public:
		FileClient(const QString& rootFolder, const HostAddressPort& serverAddressPort);
		FileClient(const QString& rootFolder, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

		void downloadFile(const QString& fileName) { emit signal_downloadFile(fileName); }

		void setRootFolder(const QString& rootFolder) { m_rootFolder = rootFolder; }

		virtual void processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize);

		virtual void onEndFileDownload(const QString fileName, FileTransferResult errorCode, const QString md5);

		bool isTransferInProgress() { return m_transferInProgress; }
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileServer : public Server, public FileTransfer
	{
	private:
		GetFileReply& m_reply;
		char* m_fileData = nullptr;

		char m_replyData[sizeof(GetFileReply) + FILE_PART_SIZE];

		void init();

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) final;

		void sendFirstFilePart(const QString& fileName);
		void sendNextFilePart();

	public:
		FileServer(const QString& rootFolder);

		virtual Server* getNewInstance() override { return new FileServer(m_rootFolder); }

		virtual void processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);
	};

}

