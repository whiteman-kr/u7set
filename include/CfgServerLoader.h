#pragma once

#include "../include/TcpFileTransfer.h"
#include "../include/OrderedHash.h"
#include "../include/BuildInfo.h"


// -------------------------------------------------------------------------------------
//
// CfgServerLoaderBase class declaration
//
// -------------------------------------------------------------------------------------

class CfgServerLoaderBase
{
protected:

	enum ErrorCode
	{
		Ok,
		BuildNotFound,
		BuildCantRead
	};
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

class CfgLoader: public Tcp::FileClient, public CfgServerLoaderBase
{
	Q_OBJECT

private:

	struct CfgFileInfo : public Builder::BuildFileInfo
	{
		bool loaded = false;
	};

	struct FileDownloadRequest
	{
		QString pathFileName;
		QString etalonMD5;
		bool isAutoRequest = false;
	};

	QString m_appStrID;
	int m_appInstance;

	QString m_appDataPath;
	QString m_rootFolder;
	QString m_configurationXmlPathFileName;

	QList<FileDownloadRequest> m_downloadQueue;
	FileDownloadRequest m_currentDownloadRequest;

	QTimer m_timer;
	bool m_configurationlReady = false;
	bool m_allFilesLoaded = false;

	Builder::BuildInfo m_buildInfo;
	HashedVector<QString, CfgFileInfo> m_cfgFileInfo;

	int m_autoDownloadIndex = 0;

	FileDownloadRequest m_currentDownload;

	void shutdown();

	void onTimer();

	virtual void onConnection() override;

	void startDownload();

	virtual void onEndFileDownload(const QString fileName, Tcp::FileTransferResult errorCode, const QString md5) final;

	void readConfigurationXml();

signals:
	void configurationReady(/*QVector<BuildFileInfo>*/);
	void endFileDownload(const QString fileName, const QString errorStr, const QString md5);

	void signal_downloadCfgFile(const QString& fileName);

private slots:

	void slot_downloadCfgdFile(const QString& fileName);

public:
	CfgLoader(const QString& appStrID, int appInstance, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

	virtual void onClientThreadStarted() override;

	void changeApp(const QString& appStrID, int appInstance);

	void downloadCfgFile(const QString& fileName);

};
