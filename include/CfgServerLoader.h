#pragma once

#include "../include/TcpFileTransfer.h"
#include "../include/OrderedHash.h"
#include "../include/BuildInfo.h"



typedef QVector<Builder::BuildFileInfo> BuildFileInfoArray;


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class declaration
//
// -------------------------------------------------------------------------------------

class CfgServerLoaderBase
{
private:
	static bool m_BuildFileInfoArrayRegistered;

protected:

	enum ErrorCode
	{
		Ok,
		BuildNotFound,
		BuildCantRead
	};

public:
	CfgServerLoaderBase();
};


// -------------------------------------------------------------------------------------
//
// CfgServer class declaration
//
// -------------------------------------------------------------------------------------

class CfgServer : public Tcp::FileServer, public CfgServerLoaderBase
{
	Q_OBJECT

private:
	QString m_buildXmlPathFileName;

	Builder::BuildInfo m_buildInfo;
	HashedVector<QString, Builder::BuildFileInfo> m_buildFileInfo;

	ErrorCode m_errorCode = ErrorCode::Ok;

	void onRootFolderChange();

	void readBuildXml();

public:
	CfgServer(const QString& buildFolder);

	virtual CfgServer* getNewInstance() override { return new CfgServer(m_rootFolder); }

	virtual void onServerThreadStarted() override;
};


// -------------------------------------------------------------------------------------
//
// CfgLoader class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoader: public Tcp::FileClient, public CfgServerLoaderBase
{
	Q_OBJECT

private:
	static const int CONFIGURATION_XML = 0;

	QMutex mutex;

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
		bool isAutoRequest = false;
		bool isTestCfgRequest = false;						// does matter only for configuration.xml file request
		QByteArray* fileData = nullptr;						// sets for manual requests only
		Tcp::FileTransferResult* errorCode = nullptr;		// sets for manual requests only

		void clear()
		{
			pathFileName = "";
			etalonMD5 = "";
			isAutoRequest = false;
			isTestCfgRequest = false;
			fileData = nullptr;
			errorCode = nullptr;
		}

		void setErrorCode(Tcp::FileTransferResult result)
		{
			if (errorCode != nullptr)
			{
				*errorCode = result;
			}
		}
	};

	QString m_appStrID;
	int m_appInstance;

	QString m_appDataPath;
	QString m_rootFolder;
	QString m_configurationXmlPathFileName;
	QString m_configurationXmlMd5;

	QList<FileDownloadRequest> m_downloadQueue;
	FileDownloadRequest m_currentDownloadRequest;

	QTimer m_timer;
	bool m_configurationXmlReady = false;
	bool m_allFilesLoaded = false;
	int m_autoDownloadIndex = 0;

	Builder::BuildInfo m_buildInfo;
	CfgFilesInfo m_cfgFilesInfo;

	bool m_hasValidSavedConfiguration = false;
	HashedVector<QString, CfgFileInfo> m_savedCfgFileInfo;

	bool m_fileReady = false;
	Tcp::FileTransferResult m_lastError = Tcp::FileTransferResult::Ok;

	QMap<QString, QString> m_fileIDPathMap;

	volatile bool m_enableDownloadConfiguration = false;

	void shutdown();

	void onTimer();

	virtual void onConnection() override;

	void startDownload();
	void resetStatuses();

	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) final;

	bool startConfigurationXmlLoading();
	bool readConfigurationXml();

	void readSavedConfiguration();

	bool readCfgFile(const QString& pathFileName, QByteArray* fileData);

	bool readCfgFileIfExists(const QString& filePathName, QByteArray* fileData, const QString& etalonMd5);
	bool isCfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	void configurationChanged();

	void setFileReady(bool value);

	QString getFilePathNameByID(QString fileID);

signals:
	void signal_enableDownloadConfiguration();
	void signal_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);
	void signal_getFile(const QString& fileName, QByteArray* fileData);
	void signal_fileReady();					// emit only for manual requests
	void signal_configurationChanged();

private slots:
	void slot_enableDownloadConfiguration();
	void slot_getFile(QString fileName, QByteArray *fileData);

public:
	CfgLoader(const QString& appStrID, int appInstance, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	virtual void onClientThreadStarted() override;

	void changeApp(const QString& appStrID, int appInstance);

	bool getFileBlocked(QString pathFileName, QByteArray* fileData, QString *errorStr);
	bool getFile(QString pathFileName, QByteArray* fileData);

	bool getFileBlockedByID(QString fileID, QByteArray* fileData, QString *errorStr);
	bool getFileByID(QString fileID, QByteArray* fileData);

	Tcp::FileTransferResult getLastError() const { return m_lastError; }
	QString getLastErrorStr();

	bool isFileReady();

	friend class CfgLoaderThread;
};


// -------------------------------------------------------------------------------------
//
// CfgLoaderThread class declaration
//
// -------------------------------------------------------------------------------------

class CfgLoaderThread : public Tcp::Thread
{
	Q_OBJECT

private:
	CfgLoader* m_cfgLoader = nullptr;

signals:
	void signal_configurationChanged();
	void signal_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

public:
	CfgLoaderThread(const QString& appStrID, int appInstance, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	void enableDownloadConfiguration();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString *errorStr);
	bool getFile(const QString& pathFileName, QByteArray* fileData);

	bool isFileReady();

	QString getLastErrorStr();
};
