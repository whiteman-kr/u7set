#pragma once

#include <QWaitCondition>

#include "TcpFileTransfer.h"
#include "TcpClientStatistics.h"
#include "SoftwareSettings.h"
#include "BuildInfo.h"
#include "GrpcFileSrv.h"

// -------------------------------------------------------------------------------------
//
// GrpcCfgLoader class declaration
//
// -------------------------------------------------------------------------------------

class GrpcCfgLoader : public SimpleThreadWorker, public LogWrapper
{
	Q_OBJECT

public:
	GrpcCfgLoader(const SoftwareInfo& softwareInfo,
				  int appInstance,
				  const std::vector<HostAddressPort>& serverAddrs,
				  CircularLoggerShared logger);

	void changeAppAndInitPaths(const QString& appEquipmentID, int appInstance);

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	HostAddressPort getServerAddr() const;
	Tcp::ConnectionState getConnectionState() const;

	bool hasFileID(const QString& fileID) const;

	OnlineLib::BuildInfo buildInfo() const;
	SoftwareInfo softwareInfo() const;
	int appInstance() const;

	SessionParams sessionParams() const;
	QString curSoftwareSettingsProfileName() const;
	E::SoftwareRunMode softwareRunMode() const;

	QStringList getSettingsProfiles() const;

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

	friend class CfgLoaderThread;

signals:
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);
	void signal_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);

	void signal_unknownClientID(QString errMsg);
	void signal_wrongClientHostname(QString errMsg);

private slots:
	void slot_setConnection();
	void slot_sessionParamsReady(Tcp::FileTransferResult result, SessionParams params);
	void slot_fileReady(FileReady fileReady);

//	void slot_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);

private:
	void onThreadStarted() override;
	void onThreadFinished() override;

	void startGrpcFileClient();
	void stopGrpcFileClient();
	void restartGrpcFileClient();

	void processCfgXmlFile(FileReady& fr);
	void processOtherFiles(const FileReady& fr);

	bool saveFile(const FileReady& fr);
	void checkExistsBuildFiles();
	void downloadNextFile();
	bool readFile(const QString& pathFileName, QByteArray* fileData, QString* errorStr) const;
	bool findBuildFileInfo(const QString& pathFileName, OnlineLib::BuildFileInfo& bfi) const;

	void resetStatuses();

	bool readCfgXmlFile(const QByteArray& fileData);

	bool readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress) const;

	bool readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needDecompress);
	bool isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	QString getFilePathNameByID(const QString& fileID) const;

private:
	inline static std::atomic_bool m_typesRegistered {false};

	SoftwareInfo m_swInfo;
	int m_appInstance = 0;
	std::vector<HostAddressPort> m_serverAddrs;

	std::unique_ptr<GrpcFileClient> m_grpcFileClient;

	QString m_appEquipmentID;
	QString m_appDataPath;
	QString m_rootFolder;
	QString m_cfgXmlFileName;
	QString m_cfgXmlMd5;

	SessionParams m_sessionParams;
	SoftwareSettingsSet m_settingsSet;

	//

	bool m_configurationXmlReady = false;
	bool m_allFilesLoaded = false;

	QByteArray m_cfgXmlFileData;
	OnlineLib::BuildInfo m_buildInfo;

	BuildFileInfoArray m_buildFilesInfo;
	std::unordered_map<QString, QString> m_filesToDownload;		// fileName => md5

	bool m_hasValidSavedConfiguration = false;

	Tcp::FileTransferResult m_lastError = Tcp::FileTransferResult::Ok;

	std::unordered_map<QString, QString> m_fileIDPathMap;		// fileID => filePathName
};

template<typename T>
std::shared_ptr<const T> GrpcCfgLoader::getSettingsProfile(const QString& profile) const
{
	std::shared_ptr<const T> settings = m_settingsSet.getSettingsProfile<T>(profile);

	return settings;
}

template<typename T>
std::shared_ptr<const T> GrpcCfgLoader::getCurrentSettingsProfile() const
{
	std::shared_ptr<const T> settings = m_settingsSet.getSettingsProfile<T>(curSoftwareSettingsProfileName());

	return settings;
}

// -------------------------------------------------------------------------------------
//
// GrpcCfgLoaderThread class declaration
//
// -------------------------------------------------------------------------------------

class GrpcCfgLoaderThread : public QObject
{
	Q_OBJECT

public:
	GrpcCfgLoaderThread(const SoftwareInfo& softwareInfo,
						int appInstance,
						const HostAddressPort& serverAddressPort1,
						const HostAddressPort& serverAddressPort2,
						std::shared_ptr<CircularLogger> logger);

	virtual ~GrpcCfgLoaderThread();

	void start();
	void quitAndWait();

	void setConnectionParams(const SoftwareInfo& softwareInfo,
							 const HostAddressPort& serverAddressPort1,

							 const HostAddressPort& serverAddressPort2);

	Tcp::ConnectionState getConnectionState() const;

//	void enableDownloadConfiguration();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	HostAddressPort getServerAddr() const;

	// When file downloaded signal_fileReady will emitted
	//
	// bool getFileAsync(const QString& pathFileName);
	// bool getFileAsyncByID(const QString& fileID);

	bool hasFileID(const QString& fileID) const;

	OnlineLib::BuildInfo buildInfo() const;

//	HostAddressPort getCurrentServerAddressPort();

	SessionParams sessionParams() const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

signals:
	void signal_configurationChanged();
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	void signal_unknownClientID(QString errMsg);
	void signal_wrongClientHostname(QString errMsg);

	// Emits only for ASYNC file requests getFileAsync and getFileAsyncByID
	// Only if errorCode == Tcp::FileTransferResult::Ok fileData will be a valid pointer, otherwise fileData will be Nullptr
	//
	void signal_fileReady(QString fileName, Tcp::FileTransferResult errorCode, std::shared_ptr<QByteArray> fileData);

private:
	void addServerAddrs(const HostAddressPort& addr1, const HostAddressPort& addr2);
	void initThread();
	void shutdownThread();

private:
	mutable std::mutex m_mutex;

	SoftwareInfo m_softwareInfo;
	int m_appInstance = 0;
	std::vector<HostAddressPort> m_serverAddrs;
	std::shared_ptr<CircularLogger> m_logger;

	GrpcCfgLoader* m_grpcCfgLoader = nullptr;
	SimpleThread* m_thread = nullptr;
};

template<typename T>
std::shared_ptr<const T> GrpcCfgLoaderThread::getCurrentSettingsProfile() const
{
	return m_grpcCfgLoader->getCurrentSettingsProfile<T>();
}

