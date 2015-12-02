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
	CfgServer(const QString& rootFolder);

	virtual CfgServer* getNewInstance() override { return new CfgServer(m_rootFolder); }

	virtual void onServerThreadStarted() override;
};


// -------------------------------------------------------------------------------------
//
// CfgLoader class declaration
//
// -------------------------------------------------------------------------------------


const int CONFIGURATION_XML = 0;

class CfgLoader: public Tcp::FileClient, public CfgServerLoaderBase
{
	Q_OBJECT

private:

	struct CfgFileInfo : public Builder::BuildFileInfo
	{
		QByteArray fileData;

		bool loaded = false;
	};

	struct FileDownloadRequest
	{
		QString pathFileName;
		QString etalonMD5;
		bool isAutoRequest = false;
		bool isTestCfgRequest = false;			// does matter only for configuration.xml file request
		QByteArray* fileData = nullptr;

		void clear()
		{
			pathFileName = "";
			etalonMD5 = "";
			isAutoRequest = false;
			isTestCfgRequest = false;
			fileData = nullptr;
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
	bool m_configurationReady = false;
	bool m_allFilesLoaded = false;
	int m_autoDownloadIndex = 0;

	Builder::BuildInfo m_buildInfo;
	HashedVector<QString, CfgFileInfo> m_cfgFileInfo;

	FileDownloadRequest m_currentDownload;

	void shutdown();

	void onTimer();

	virtual void onConnection() override;

	void startDownload();

	void resetStatuses();

	bool readFile(const QString& filePathName, QByteArray* fileData);

	bool cfgFileIsExists(const QString& filePathName, const QString& etalonMd5);

	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) final;

	bool readConfigurationXml();
	bool readConfigurationFile(const QString& pathFileName, QByteArray* fileData);

signals:
	void signal_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);
	void signal_endFileDownload(const QString fileName, const QString errorStr);
	void signal_downloadCfgFile(const QString& fileName, QByteArray* fileData);

private slots:

	void slot_downloadCfgdFile(const QString& fileName, QByteArray *fileData);

public:
	CfgLoader(const QString& appStrID, int appInstance, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	virtual void onClientThreadStarted() override;

	void changeApp(const QString& appStrID, int appInstance);

	bool downloadCfgFile(const QString& pathFileName, QByteArray* fileData);
};
