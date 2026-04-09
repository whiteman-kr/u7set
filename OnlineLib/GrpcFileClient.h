#pragma once

#include <memory>
#include <queue>
#include <chrono>
#include <mutex>
#include <grpcpp/grpcpp.h>

#include <QString>

#include <GrpcFileSrv.grpc.pb.h>

#include <CommonLib/HostAddressPort.h>
#include <CommonLib/Hash.h>

#include "GrpcClient.h"
#include "CircularLogger.h"
#include "SoftwareSettings.h"
#include "TcpFileTransfer.h"

struct FileReady
{
	QString fileName;
	Tcp::FileTransferResult errorCode;
	QByteArray fileData;
	QString md5;
};

// -------------------------------------------------------------------------------------
//
// GrpcFileClient class declaration
//
// -------------------------------------------------------------------------------------

class GrpcFileClient : public GrpcClient<Grpc::FileSrv>
{
	Q_OBJECT

public:
	GrpcFileClient(const SoftwareInfo& localSoftwareInfo,
				   const std::vector<HostAddressPort>& serverAddress,
				   const QString& rootFolder,
				   const QString& clientDescription,
				   CircularLoggerShared log,
				   int pingPeriodMs);

	GrpcFileClient(const GrpcFileClient&) = delete;
	GrpcFileClient& operator=(const GrpcFileClient&) = delete;
	GrpcFileClient(GrpcFileClient&&) = delete;
	GrpcFileClient& operator=(GrpcFileClient&&) = delete;

	virtual ~GrpcFileClient();

	void downloadSessionParams();

	// async file download
	//
	void downloadFile(const QString& fileName);

	// for blocked calls
	//
	bool waitFileReady(FileReady* fileReady);
	bool downloadFileBlocked(const QString& fileName, FileReady* fileReady);

	bool isTransferInProgress();

	void setEmitFileReady(bool enable);		// if TRUE - signal_fileReady emitted
											// if FALSE - use waitFileReady or downloadFileBlocked
signals:
	void signal_sessionParamsReady(Tcp::FileTransferResult result, SessionParams params);
	void signal_fileReady(FileReady fileReady);			// not ref - Ok
	void signal_fileDowloadTimeout();

protected:
	QString getErrorStr(Tcp::FileTransferResult errorCode) const;

private:
	virtual void run() override;
	virtual void wakeupThread() override;

	Tcp::FileTransferResult privateGetSessionParams();
	Tcp::FileTransferResult privateDownloadFile(const QString& fileName);

	void pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode);
	void pushFileReady(const QString& fileName, Tcp::FileTransferResult errorCode,
					   QByteArray& fileData, QString& md5);	// not const - OK!

	static QString getCleanFileName(const QString& rootFolder, const QString& fileName);

private:
	QString m_rootFolder;
	std::atomic_bool m_emitFileReady {false};

	QStringList m_downloadFileQueue;
	std::atomic_bool m_transferInProgress {false};

	std::mutex m_fileReadyMutex;
	std::condition_variable m_fileReadyCond;
	std::queue<FileReady> m_fileReadyQueue;

	inline static const QString SESSION_PARAMS_REQUEST = QStringLiteral("*SESSION_PARAMS_REQUEST*");
};
