#pragma once

#include <QFile>
#include <QDir>

#include "Tcp.h"
#include "CircularLogger.h"

namespace Network
{
	class GetFileReply;
}

namespace Tcp
{
	const quint32	RQID_GET_FILE_START = 400,
					RQID_GET_FILE_NEXT = 401;

	enum class FileTransferResult
	{
		Ok,

		NotConnectedToServer,

		FileIsNotAccessible,
		RemoteFileIsNotExists,
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
		ServerReplyTimeout,
		TransferIsNotStarted,

		ConfigurationIsNotReady,		// for CfgLoader class

		UnknownClientID,
		WrongClientHostname,

		InternalError,
		FileTooBig,
	};

	class FileTransfer
	{
	public:
		FileTransfer();
		virtual ~FileTransfer();

	protected:
		QString m_rootFolder;
		QString m_fileName;
		QFile m_file;
		QCryptographicHash m_md5Generator;

		bool m_transferInProgress = false;

		const size_t MD5_LEN = 128 / 8;		// MD5 code length in bytes

		const int FILE_PART_SIZE = 65536;

	private:
		static bool m_FileTransferErrorIsRegistered;
	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileClient class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileClient : public Client, public FileTransfer
	{
		Q_OBJECT

	public:
		FileClient(const SoftwareInfo& softwareInfo,
				   const QString& rootFolder,
				   const HostAddressPort& serverAddressPort,
				   const QString& clientDescription);

		FileClient(const SoftwareInfo& softwareInfo,
				   const QString& rootFolder,
				   const HostAddressPort& serverAddressPort1,
				   const HostAddressPort& serverAddressPort2,
				   const QString& clientDescription);

		virtual ~FileClient();

		void downloadFile(const QString& fileName) { emit signal_downloadFile(fileName); }

		void setRootFolder(const QString& rootFolder) { m_rootFolder = rootFolder; }

		virtual void processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize);

		virtual void onEndFileDownload(const QString fileName,
									   FileTransferResult errorCode,
									   const QString md5);

		bool isTransferInProgress() { return m_transferInProgress; }

	protected:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;
		virtual void onReplyTimeout() override;

		QString getErrorStr(FileTransferResult errorCode) const;

	protected slots:
		void slot_downloadFile(const QString fileName);

	signals:
		void signal_downloadFile(const QString fileName);

		void signal_endFileDownload(const QString fileName,
									FileTransferResult errorCode,
									const QString md5);
	private:
		FileTransferResult checkLocalFolder();		// check read/write ability for m_rootFolder
		FileTransferResult createFile();

		void init();
		void faultyFileDownload(FileTransferResult errorCode);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override final;

		void processGetFileStartNextReply(bool startReply, const char* replyData, quint32 replyDataSize);

	private:
		std::unique_ptr<Network::GetFileReply> m_reply;		// unique_ptr for build time optimization.
	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::FileServer class declaration
	//
	// -------------------------------------------------------------------------------------

	class FileServer : public Server, public FileTransfer
	{
	public:
		FileServer(const QString& rootFolder,
				   const SoftwareInfo& softwareInfo,
				   CircularLoggerShared logger,
				   const QString& serverDescription);

		virtual Server* getNewInstance(const ListenAddress& listenAddr) override;

		virtual void processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);

		QString rootFolder() const;

		virtual void onFileSent(const QString& fileName, const QString& ip);

	protected:
		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;
		virtual bool checkFile(QString& pathFileName, QByteArray& fileData);

	private:
		void init();

		void sendFirstFilePart(const QString& fileName);
		void sendNextFilePart();

		void restartTransmitionFilesTimer();

	private:
		QByteArray m_fileData;

		std::unique_ptr<Network::GetFileReply> m_reply;		// unique_ptr for build time optimization.

		QTimer m_transmitionFilesTimer;
	};

}


