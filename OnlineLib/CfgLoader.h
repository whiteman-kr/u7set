#pragma once

#include <QWaitCondition>

#include "TcpFileTransfer.h"
#include "TcpClientStatistics.h"
#include "SoftwareSettings.h"
#include <CommonLib/HashedVector.h>
#include "../OnlineLib/BuildInfo.h"

using BuildFileInfoArray = std::vector<OnlineLib::BuildFileInfo>;

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
// CfgLoader class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoader : public Tcp::FileClient, public CfgServerLoaderBase, protected TcpClientStatistics
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

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	// When file downloaded signal_fileReady will emitted
	//
	bool getFileAsync(const QString& pathFileName);
	bool getFileAsyncByID(const QString& fileID);

	bool hasFileID(QString fileID) const;

	Tcp::FileTransferResult getLastError() const { return m_lastError; }
	QString getLastErrorStr() const { return getErrorStr(getLastError()); }

	OnlineLib::BuildInfo buildInfo();
	SoftwareInfo softwareInfo() const { return localSoftwareInfo(); }
	int appInstance() const { return m_appInstance; }
	bool enableDownloadCfg() const { return m_enableDownloadConfiguration; }

	SessionParams sessionParams() const;
	QString curSoftwareSettingsProfileName() const;
	E::SoftwareRunMode softwareRunMode() const;

	QStringList getSettingsProfiles() const;

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getCurrentSettingsProfile() const;

	virtual void onConnection() override;
	virtual void onStartDownload(const QString& fileName);
	virtual void onEndDownload(const QString& fileName, Tcp::FileTransferResult errorCode);

	friend class CfgLoaderThread;

signals:
	void signal_enableDownloadConfiguration();
	void signal_configurationReady(const QByteArray configurationXmlData,
								   const BuildFileInfoArray buildFileInfoArray,
								   SessionParams sessionParams,
								   std::shared_ptr<const SoftwareSettings> currentSettingsProfile);
	void signal_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);
	void signal_configurationChanged();

	// Emits only for ASYNC file requests getFileAsync and getFileAsyncByID
	// Only if errorCode == Tcp::FileTransferResult::Ok fileData will be a valid pointer, otherwise fileData will be Nullptr
	//
	void signal_fileReady(QString fileName, Tcp::FileTransferResult errorCode, std::shared_ptr<QByteArray> fileData);

private slots:
	void slot_enableDownloadConfiguration();
	void slot_getFile(QString fileName, std::shared_ptr<QByteArray> fileData, bool asyncCall);
	void slot_onTimer();

protected:
	virtual void processSuccessorReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;
	void processGetSessionParamsReply(const char* replyData, quint32 replyDataSize);
	void sendGetSessionParamsRequest();

private:
	void shutdown();

	void startDownload();
	void resetStatuses();

	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) override final;

	bool startConfigurationXmlLoading();
	bool readConfigurationXml();

	void readSavedConfiguration();

	bool readCfgFile(const QString& pathFileName, QByteArray* fileData, bool needUncompress);

	bool readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5, bool needDecompress);
	bool isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	void configurationChanged();

	void emitFileReady(const QString& fileName, Tcp::FileTransferResult errorCode,
					   std::shared_ptr<QByteArray> fileData, bool asyncCall);

	QString getFilePathNameByID(const QString& fileID) const;

private:
	struct CfgFileInfo : public OnlineLib::BuildFileInfo
	{
		QByteArray fileData;
		bool md5IsValid = false;
	};

	using CfgFilesInfo = HashedVector<QString, CfgFileInfo> ;

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

	int m_appInstance = 0;
	volatile bool m_enableDownloadConfiguration = false;

	SessionParams m_sessionParams;
	SoftwareSettingsSet m_settingsSet;

	//

	static const int CONFIGURATION_XML_FILE_INDEX = 0;

	mutable QRecursiveMutex m_mutex;

	QString m_appEquipmentID;

	QString m_appDataPath;
	QString m_rootFolder;
	QString m_configurationXmlPathFileName;
	QString m_configurationXmlMd5;

	std::list<FileDownloadRequest> m_downloadQueue;
	FileDownloadRequest m_currentDownloadRequest;

	QTimer m_timer;
	bool m_configurationXmlReady = false;
	bool m_allFilesLoaded = false;
	int m_autoDownloadIndex = 0;

	OnlineLib::BuildInfo m_buildInfo;
	CfgFilesInfo m_cfgFilesInfo;					// can't remove HashedVector here because
													// configuration.xml should be in m_cfgFilesInfo[0]!!!

	bool m_hasValidSavedConfiguration = false;

	QWaitCondition m_fileReadyCondition;
	QMutex m_getFileBlockedMutex;

	Tcp::FileTransferResult m_lastError = Tcp::FileTransferResult::Ok;

	std::map<QString, QString> m_fileIDPathMap;		// fileID => filePathName
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
	void quitAndWait();

	void enableDownloadConfiguration();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedByID(const QString& fileID, QByteArray* fileData, QString* errorStr);

	// When file downloaded signal_fileReady will emitted
	//
	bool getFileAsync(const QString& pathFileName);
	bool getFileAsyncByID(const QString& fileID);

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

	void signal_unknownClientID(QString errMsg);
	void signal_wrongClientHostname(QString errMsg);

	// Emits only for ASYNC file requests getFileAsync and getFileAsyncByID
	// Only if errorCode == Tcp::FileTransferResult::Ok fileData will be a valid pointer, otherwise fileData will be Nullptr
	//
	void signal_fileReady(QString fileName, Tcp::FileTransferResult errorCode, std::shared_ptr<QByteArray> fileData);

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

	mutable QRecursiveMutex m_mutex;

	CfgLoader* m_cfgLoader = nullptr;
	SimpleThread* m_thread = nullptr;
};

template<typename T>
std::shared_ptr<const T> CfgLoaderThread::getCurrentSettingsProfile() const
{
	return m_cfgLoader->getCurrentSettingsProfile<T>();
}

