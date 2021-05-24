#pragma once

#include <QWaitCondition>

#include "TcpFileTransfer.h"
#include "../CommonLib/OrderedHash.h"
#include "../lib/BuildInfo.h"
#include "../lib/SoftwareSettings.h"

typedef QVector<Builder::BuildFileInfo> BuildFileInfoArray;

// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class declaration
//
// -------------------------------------------------------------------------------------

class CfgServerLoaderBase
{
public:
	CfgServerLoaderBase();

protected:

	enum ErrorCode
	{
		Ok,
		BuildNotFound,
		BuildCantRead
	};

private:
	static bool m_typesRegistered;
};

// -------------------------------------------------------------------------------------
//
// CfgServer class declaration
//
// -------------------------------------------------------------------------------------

class CfgServer : public Tcp::FileServer, public CfgServerLoaderBase
{
	Q_OBJECT

public:
	CfgServer(const SoftwareInfo& softwareInfo,
			  const QString& buildFolder,
			  const SessionParams& sessionParams,
			  std::shared_ptr<CircularLogger> logger);

	virtual CfgServer* getNewInstance() override;

	virtual void processSuccessorRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	const Builder::BuildInfo& buildInfo() { return m_buildInfo; }

private:
	void readBuildXml();

	bool checkFile(QString& pathFileName, QByteArray& fileData) override;

	void processGetSessionParamsRequest();

private:
	std::shared_ptr<CircularLogger> m_logger;

	SessionParams m_sessionParams;

	QString m_buildXmlPathFileName;

	Builder::BuildInfo m_buildInfo;
	HashedVector<QString, Builder::BuildFileInfo> m_buildFileInfo;		// fileName => buildFileInfo

	ErrorCode m_errorCode = ErrorCode::Ok;
};


// -------------------------------------------------------------------------------------
//
// CfgLoader class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoader : public Tcp::FileClient, public CfgServerLoaderBase
{
	Q_OBJECT

public:
	CfgLoader(const SoftwareInfo& softwareInfo,
				int appInstance,
				const HostAddressPort& serverAddressPort1,
				const HostAddressPort& serverAddressPort2,
				bool enableDownloadCfg,
				std::shared_ptr<CircularLogger> logger);

	virtual void onClientThreadStarted() override;

	void changeApp(const QString& appEquipmentID, int appInstance);

	bool getFileBlocked(QString pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFile(QString pathFileName, QByteArray* fileData);

	bool getFileBlockedByID(QString fileID, QByteArray* fileData, QString* errorStr);
	bool getFileByID(QString fileID, QByteArray* fileData);

	bool hasFileID(QString fileID) const;

	Tcp::FileTransferResult getLastError() const { return m_lastError; }
	QString getLastErrorStr() const { return getErrorStr(getLastError()); }

	Builder::BuildInfo buildInfo();
	SoftwareInfo softwareInfo() const { return localSoftwareInfo(); }
	int appInstance() const { return m_appInstance; }
	bool enableDownloadCfg() const { return m_enableDownloadConfiguration; }
	std::shared_ptr<CircularLogger> logger() { return m_logger; }

	SessionParams sessionParams() const;
	QString curSoftwareSettingsProfileName() const;
	E::SoftwareRunMode softwareRunMode() const;

	QStringList getSettingsProfiles() const;

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

	virtual void onTryConnectToServer(const HostAddressPort& serverAddr) override;
	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onStartDownload(const QString& fileName);
	virtual void onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode);

	friend class CfgLoaderThread;

signals:
	void signal_enableDownloadConfiguration();
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);
	void signal_getFile(const QString& fileName, QByteArray* fileData);
	void signal_fileReady();					// emit only for manual requests
	void signal_configurationChanged();

	void signal_unknownClient();
	void signal_onEndFileDownload(const QString& fileName, Tcp::FileTransferResult errorCode);
	void signal_onEndFileDownloadError(const QString& fileName, Tcp::FileTransferResult errorCode);

private slots:
	void slot_enableDownloadConfiguration();
	void slot_getFile(QString fileName, QByteArray *fileData);
	void slot_onTimer();

protected:
	virtual void processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;
	void processGetSessionParamsReply(const char* replyData, quint32 replyDataSize);
	void sendGetSessionParamsRequest();

private:
	void shutdown();

	void startDownload();
	void resetStatuses();

	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) final;

	bool startConfigurationXmlLoading();
	bool readConfigurationXml();

	void readSavedConfiguration();

	bool readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress);

	bool readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needDecompress);
	bool isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	void configurationChanged();

	void emitFileReady();

	QString getFilePathNameByID(QString fileID) const;

	void emitSignalUnknownClient();

private:
	struct CfgFileInfo : public Builder::BuildFileInfo
	{
		QByteArray fileData;
		bool md5IsValid = false;
	};

	typedef HashedVector<QString, CfgFileInfo> CfgFilesInfo;

	struct FileDownloadRequest
	{
		QString pathFileName;
		QString etalonMD5;
		bool needUncompress = false;

		bool isAutoRequest = false;
		bool isTestCfgRequest = false;						// does matter only for Configuration.xml file request
		QByteArray* fileData = nullptr;						// sets for manual requests only
		Tcp::FileTransferResult* errorCode = nullptr;		// sets for manual requests only

		void clear();
		void setErrorCode(Tcp::FileTransferResult result);
	};

	//

	int m_appInstance = 0;
	volatile bool m_enableDownloadConfiguration = false;

	SessionParams m_sessionParams;
	SoftwareSettingsSet m_settingsSet;

	//

	static const int CONFIGURATION_XML_FILE_INDEX = 0;

	mutable QMutex m_mutex;

	QString m_appEquipmentID;

	QString m_appDataPath;
	QString m_rootFolder;
	QString m_configurationXmlPathFileName;
	QString m_configurationXmlMd5;

	QByteArray m_localFileData;

	std::shared_ptr<CircularLogger> m_logger;

	QList<FileDownloadRequest> m_downloadQueue;
	FileDownloadRequest m_currentDownloadRequest;

	QTimer m_timer;
	bool m_configurationXmlReady = false;
	bool m_allFilesLoaded = false;
	int m_autoDownloadIndex = 0;

	Builder::BuildInfo m_buildInfo;
	CfgFilesInfo m_cfgFilesInfo;

	bool m_hasValidSavedConfiguration = false;
	CfgFilesInfo m_savedCfgFileInfo;

	QWaitCondition m_fileReadyCondition;
	QMutex m_getFileBlockedMutex;

	Tcp::FileTransferResult m_lastError = Tcp::FileTransferResult::Ok;

	QMap<QString, QString> m_fileIDPathMap;

	bool m_enableSignalUnknownClient = true;
};

template<typename T>
std::shared_ptr<const T> CfgLoader::getSettingsProfile(const QString& profile) const
{
	AUTO_LOCK(m_mutex);

	std::shared_ptr<const T> settings = m_settingsSet.getSettingsProfile<T>(profile);

	return settings;
}

template<typename T>
std::shared_ptr<const T> CfgLoader::getCurrentSettingsProfile() const
{
	AUTO_LOCK(m_mutex);

	std::shared_ptr<const T> settings = m_settingsSet.getSettingsProfile<T>(curSoftwareSettingsProfileName());

	return settings;
}


// -------------------------------------------------------------------------------------
//
// CfgLoaderThread class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoaderThread : public QObject
{
	Q_OBJECT

public:
	CfgLoaderThread(const SoftwareInfo& softwareInfo,
					int appInstance,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					bool enableDownloadCfg,
					std::shared_ptr<CircularLogger> logger);

	virtual ~CfgLoaderThread();

	void start();
	void quit();
	void quitAndWait();

	void enableDownloadConfiguration();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFile(const QString& pathFileName, QByteArray* fileData);

	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);
	bool getFileByID(const QString& fileID, QByteArray* fileData);

	bool hasFileID(QString fileID) const;

	Builder::BuildInfo buildInfo();

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

	void signal_unknownClient();
	void signal_onEndFileDownload(const QString& fileName, Tcp::FileTransferResult errorCode);
	void signal_onEndFileDownloadError(const QString& fileName, Tcp::FileTransferResult errorCode);

private:
	void initThread();
	void shutdownThread(bool* restartThread);

private:
	SoftwareInfo m_softwareInfo;
	int m_appInstance;
	HostAddressPort m_server1;
	HostAddressPort m_server2;
	bool m_enableDownloadCfg;
	std::shared_ptr<CircularLogger> m_logger;

	//

	mutable QMutex m_mutex;

	CfgLoader* m_cfgLoader = nullptr;
	SimpleThread* m_thread = nullptr;
};

template<typename T>
std::shared_ptr<const T> CfgLoaderThread::getCurrentSettingsProfile() const
{
	return m_cfgLoader->getCurrentSettingsProfile<T>();
}

