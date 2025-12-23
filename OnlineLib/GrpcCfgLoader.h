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

class GrpcCfgLoader : public SimpleThreadWorker
{
	Q_OBJECT

public:
	GrpcCfgLoader(const SoftwareInfo& softwareInfo,
				  int appInstance,
				  const std::vector<HostAddressPort>& serverAddrs,
				  bool enableDownloadCfg,
				  CircularLoggerShared logger);

	void changeAppAndInitPaths(const QString& appEquipmentID, int appInstance);

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	// When file downloaded signal_fileReady will emitted
	//
//	bool getFileAsync(const QString& pathFileName);
//	bool getFileAsyncByID(const QString& fileID);

	bool hasFileID(QString fileID) const;

	Tcp::FileTransferResult getLastError() const { return m_lastError; }
//	QString getLastErrorStr() const { return getErrorStr(getLastError()); }

	OnlineLib::BuildInfo buildInfo();
	SoftwareInfo softwareInfo() const { return m_swInfo; }
	int appInstance() const { return m_appInstance; }
	bool enableDownloadCfg() const { return m_enableDownloadCfg; }

	SessionParams sessionParams() const;
	QString curSoftwareSettingsProfileName() const;
	E::SoftwareRunMode softwareRunMode() const;

	QStringList getSettingsProfiles() const;

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

//	virtual void onStartDownload(const QString& fileName);
//	virtual void onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode);

	friend class CfgLoaderThread;

signals:
	void signal_enableDownloadConfiguration();
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);
	void signal_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);
	void signal_configurationChanged();

	void signal_unknownClientID();
	void signal_wrongClientHostname();

	// Emits only for ASYNC file requests getFileAsync and getFileAsyncByID
	// Only if errorCode == Tcp::FileTransferResult::Ok fileData will be a valid pointer, otherwise fileData will be Nullptr
	//
	void signal_fileReady(QString fileName, Tcp::FileTransferResult errorCode, std::shared_ptr<QByteArray> fileData);

private slots:
	void slot_setConnection();
	void slot_sessionParamsReady(Tcp::FileTransferResult result, SessionParams params);
	void slot_fileReady(FileReady fileReady);
	void slot_enableDownloadConfiguration();
	void slot_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);
	void slot_onTimer();

protected:
	void processGetSessionParamsReply(const char* replyData, quint32 replyDataSize);
	void sendGetSessionParamsRequest();

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
	bool readFile(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool findBuildFileInfo(const QString& pathFileName, OnlineLib::BuildFileInfo& bfi);

	void shutdown();

	void startDownload();
	void resetStatuses();

//	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) override final;

	bool startConfigurationXmlLoading();
	bool readCfgXmlFile(const QByteArray& fileData);

	void readSavedConfiguration();

	bool readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress);

	bool readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needDecompress);
	bool isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	void configurationChanged();

	void emitFileReady(const QString& fileName, Tcp::FileTransferResult errorCode,
					   std::shared_ptr<QByteArray> fileData, bool asyncCall);

	QString getFilePathNameByID(const QString& fileID) const;

	void logMsg(const QString& msg);
	void logWrn(const QString& wrn);
	void logErr(const QString& err);

private:
	inline static std::atomic_bool m_typesRegistered {false};

	SoftwareInfo m_swInfo;
	int m_appInstance = 0;
	std::vector<HostAddressPort> m_serverAddrs;
	bool m_enableDownloadCfg = false;
	CircularLoggerShared m_log;

	std::unique_ptr<GrpcFileClient> m_grpcFileClient;

	QString m_appEquipmentID;

	QString m_appDataPath;
	QString m_rootFolder;
	QString m_cfgXmlFileName;
	QString m_cfgXmlMd5;

	SessionParams m_sessionParams;
	SoftwareSettingsSet m_settingsSet;

//	using CfgFilesInfo = HashedVector<QString, CfgFileInfo> ;

	struct FileDownloadRequest
	{
		QString pathFileName;
		QString etalonMD5;
		bool needUncompress = false;

		bool isAutoRequest = false;
		bool isTestCfgRequest = false;						// does matter only for Configuration.xml file request

		bool isAsyncCall = false;							// sets for manual requests only
		std::shared_ptr<QByteArray> fileData = nullptr;		// sets for manual requests only

		void clear();
	};

	//



	//

	static const int CONFIGURATION_XML_FILE_INDEX = 0;

	mutable QRecursiveMutex m_mutex;


	std::list<FileDownloadRequest> m_downloadQueue;
	FileDownloadRequest m_currentDownloadRequest;

//	QTimer m_timer;
	bool m_configurationXmlReady = false;
	bool m_allFilesLoaded = false;
	int m_autoDownloadIndex = 0;

	QByteArray m_cfgXmlFileData;
	OnlineLib::BuildInfo m_buildInfo;

	BuildFileInfoArray m_buildFilesInfo;
	std::unordered_map<QString, QString> m_filesToDownload;		// fileName => md5
//	std::condition_variable m_fileReadyCondition;

	bool m_hasValidSavedConfiguration = false;

	Tcp::FileTransferResult m_lastError = Tcp::FileTransferResult::Ok;

	std::unordered_map<QString, QString> m_fileIDPathMap;		// fileID => filePathName
};

template<typename T>
std::shared_ptr<const T> GrpcCfgLoader::getSettingsProfile(const QString& profile) const
{
	AUTO_LOCK(m_mutex);

	std::shared_ptr<const T> settings = m_settingsSet.getSettingsProfile<T>(profile);

	return settings;
}

template<typename T>
std::shared_ptr<const T> GrpcCfgLoader::getCurrentSettingsProfile() const
{
	AUTO_LOCK(m_mutex);

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
					bool enableDownloadCfg,
					std::shared_ptr<CircularLogger> logger);

	virtual ~GrpcCfgLoaderThread();

	void start();
	void quitAndWait();

//	void enableDownloadConfiguration();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	// When file downloaded signal_fileReady will emitted
	//
	// bool getFileAsync(const QString& pathFileName);
	// bool getFileAsyncByID(const QString& fileID);

	bool hasFileID(QString fileID) const;

	OnlineLib::BuildInfo buildInfo();

	QString getLastErrorStr();

	Tcp::ConnectionState getConnectionState();
	HostAddressPort getCurrentServerAddressPort();

	void setConnectionParams(const SoftwareInfo& softwareInfo,
							 const HostAddressPort& serverAddressPort1,
							 const HostAddressPort& serverAddressPort2,
							 bool enableDownloadConfiguration);

	SessionParams sessionParams() const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

signals:
	void signal_configurationChanged();
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);

	void signal_unknownClientID();
	void signal_wrongClientHostname();

	// Emits only for ASYNC file requests getFileAsync and getFileAsyncByID
	// Only if errorCode == Tcp::FileTransferResult::Ok fileData will be a valid pointer, otherwise fileData will be Nullptr
	//
	void signal_fileReady(QString fileName, Tcp::FileTransferResult errorCode, std::shared_ptr<QByteArray> fileData);

private:
	void addServerAddrs(const HostAddressPort& addr1, const HostAddressPort& addr2);
	void initThread();
	void shutdownThread();

private:
	SoftwareInfo m_softwareInfo;
	int m_appInstance;
	std::vector<HostAddressPort> m_serverAddrs;
	bool m_enableDownloadCfg;
	std::shared_ptr<CircularLogger> m_logger;

	//

	mutable QRecursiveMutex m_mutex;

	GrpcCfgLoader* m_grpcCfgLoader = nullptr;
	SimpleThread* m_thread = nullptr;
};

template<typename T>
std::shared_ptr<const T> GrpcCfgLoaderThread::getCurrentSettingsProfile() const
{
	return m_grpcCfgLoader->getCurrentSettingsProfile<T>();
}

